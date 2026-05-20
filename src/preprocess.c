#include "preprocess.h"
#include "file_io.h"
#include "utils.h"
#include <ctype.h>
#include <string.h>
#include <math.h>


static double wq_sqrt_local(double x)
{
    double g;
    int i;
    if (x <= 0.0) return 0.0;
    g = x > 1.0 ? x : 1.0;
    for (i = 0; i < 20; ++i) g = 0.5 * (g + x / g);
    return g;
}

static double wq_compute_stddev_parameter(const WaterQualityDataset *dataset, WQParameter p)
{
    size_t i, n = 0U;
    double sum = 0.0, sq = 0.0;
    for (i = 0U; i < dataset->size; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        if (!r->valid || r->missing[(size_t)p] || isnan(r->value[(size_t)p])) continue;
        sum += r->value[(size_t)p];
        sq += r->value[(size_t)p] * r->value[(size_t)p];
        n++;
    }
    if (n < 2U) return 0.0;
    return wq_sqrt_local((sq / (double)n) - (sum / (double)n) * (sum / (double)n));
}

static bool wq_is_filter_window_valid(size_t window_size)
{
    return window_size == 3U || window_size == 5U || window_size == 7U ||
           window_size == 9U || window_size == 11U;
}

static double wq_compute_parameter_global_mean(const WaterQualityDataset *dataset, WQParameter parameter)
{
    size_t i;
    size_t count = 0U;
    double sum = 0.0;
    for (i = 0U; i < dataset->size; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        if (r->valid && !r->missing[(size_t)parameter]) {
            sum += r->value[(size_t)parameter];
            count++;
        }
    }
    return count > 0U ? (sum / (double)count) : 0.0;
}

static double wq_neighbor_mean(const WaterQualityDataset *dataset,
                               size_t index,
                               WQParameter parameter,
                               size_t limit,
                               bool backward,
                               size_t *count_out)
{
    size_t found = 0U;
    double sum = 0.0;
    size_t i;

    if (count_out == NULL) {
        return 0.0;
    }

    if (backward) {
        i = index;
        while (i > 0U && found < limit) {
            const WaterQualityRecord *r;
            i--;
            r = &dataset->records[i];
            if (r->valid && !r->missing[(size_t)parameter]) {
                sum += r->value[(size_t)parameter];
                found++;
            }
        }
    } else {
        i = index + 1U;
        while (i < dataset->size && found < limit) {
            const WaterQualityRecord *r = &dataset->records[i];
            if (r->valid && !r->missing[(size_t)parameter]) {
                sum += r->value[(size_t)parameter];
                found++;
            }
            i++;
        }
    }

    *count_out = found;
    return found > 0U ? (sum / (double)found) : 0.0;
}

/*
 * preprocess.c
 *
 * 数据预处理模块函数框架。
 * 实际算法包括异常值检测、缺失值均值逼近填充和移动平均滤波。
 */

bool wq_is_missing_text(const char *text)
{
    char buf[WQ_MAX_LINE_LENGTH];
    size_t i;
    if (text == NULL) {
        return true;
    }
    strncpy(buf, text, sizeof(buf) - 1U);
    buf[sizeof(buf) - 1U] = '\0';
    wq_trim(buf);
    if (buf[0] == '\0') {
        return true;
    }
    for (i = 0U; buf[i] != '\0'; ++i) {
        buf[i] = (char)tolower((unsigned char)buf[i]);
    }
    return (strcmp(buf, "nan") == 0);
}

bool wq_is_missing_value(double value)
{
    return value == WQ_MISSING_MARKER_1 || value == WQ_MISSING_MARKER_2 || isnan(value);
}

bool wq_is_value_in_range(WQParameter parameter, double value)
{
    /* 按参数枚举获取上下限，判断是否在合理范围内。 */
    return value >= wq_parameter_min(parameter) && value <= wq_parameter_max(parameter);
}

size_t wq_count_record_outliers(const WaterQualityRecord *record)
{
    size_t count;
    size_t i;

    if (record == NULL) {
        return 0U;
    }

    count = 0U;
    for (i = 0U; i < (size_t)WQ_PARAM_COUNT; ++i) {
        if (!record->missing[i] && !wq_is_value_in_range((WQParameter)i, record->value[i])) {
            count++;
        }
    }

    return count;
}

int wq_detect_outliers(const WaterQualityDataset *dataset, DataOverview *overview)
{
    size_t i;
    bool has_outlier = false;

    if (dataset == NULL || overview == NULL) {
        return WQ_ERROR;
    }

    overview->total_records = dataset->size;
    overview->valid_records = 0U;
    overview->invalid_records = 0U;
    overview->outlier_records = 0U;
    overview->outlier_parameter_count = 0U;

    for (i = 0U; i < dataset->size; ++i) {
        size_t outlier_count;
        const WaterQualityRecord *r = &dataset->records[i];
        if (r->valid) {
            overview->valid_records++;
        } else {
            overview->invalid_records++;
        }

        outlier_count = wq_count_record_outliers(r);
        if (outlier_count > 0U) {
            overview->outlier_records++;
            overview->outlier_parameter_count += outlier_count;
            if (!has_outlier) {
                overview->first_outlier_time = r->time;
                has_outlier = true;
            }
            overview->last_outlier_time = r->time;
        }
    }
    return WQ_SUCCESS;
}

int wq_process_outliers(WaterQualityDataset *dataset, DataOverview *overview)
{
    size_t i = 0U;

    if (dataset == NULL || overview == NULL) {
        return WQ_ERROR;
    }

    overview->fixed_outlier_records = 0U;
    overview->deleted_outlier_records = 0U;

    while (i < dataset->size) {
        size_t j;
        size_t outlier_count = wq_count_record_outliers(&dataset->records[i]);
        if (outlier_count >= 3U) {
            for (j = i + 1U; j < dataset->size; ++j) {
                dataset->records[j - 1U] = dataset->records[j];
            }
            dataset->size--;
            overview->deleted_outlier_records++;
            continue;
        }

        if (outlier_count > 0U) {
            for (j = 0U; j < (size_t)WQ_PARAM_COUNT; ++j) {
                if (!dataset->records[i].missing[j] &&
                    !wq_is_value_in_range((WQParameter)j, dataset->records[i].value[j])) {
                    dataset->records[i].missing[j] = true;
                    overview->missing_values++;
                }
            }
            overview->fixed_outlier_records++;
        }
        i++;
    }

    return WQ_SUCCESS;
}

int wq_fill_missing_values(WaterQualityDataset *dataset,
                           size_t before_n,
                           size_t after_m,
                           DataOverview *overview)
{
    size_t i;
    size_t p;
    double global_mean[WQ_PARAM_COUNT];

    if (dataset == NULL || overview == NULL || before_n == 0U || after_m == 0U) {
        return WQ_ERROR;
    }

    for (p = 0U; p < (size_t)WQ_PARAM_COUNT; ++p) {
        global_mean[p] = wq_compute_parameter_global_mean(dataset, (WQParameter)p);
    }

    for (i = 0U; i < dataset->size; ++i) {
        WaterQualityRecord *r = &dataset->records[i];
        if (!r->valid) {
            continue;
        }
        for (p = 0U; p < (size_t)WQ_PARAM_COUNT; ++p) {
            if (r->missing[p]) {
                size_t lc = 0U;
                size_t rc = 0U;
                double lv = wq_neighbor_mean(dataset, i, (WQParameter)p, before_n, true, &lc);
                double rv = wq_neighbor_mean(dataset, i, (WQParameter)p, after_m, false, &rc);
                if (lc > 0U && rc > 0U) {
                    r->value[p] = (lv + rv) / 2.0;
                } else if (lc > 0U) {
                    r->value[p] = lv;
                } else if (rc > 0U) {
                    r->value[p] = rv;
                } else {
                    r->value[p] = global_mean[p];
                }
                r->missing[p] = false;
                overview->fixed_missing_values++;
            }
        }
    }
    return WQ_SUCCESS;
}

int wq_moving_average_filter(const WaterQualityDataset *src,
                             WaterQualityDataset *dst,
                             WQParameter parameter,
                             size_t window_size)
{
    size_t i;
    size_t half;

    if (src == NULL || dst == NULL || !wq_is_filter_window_valid(window_size)) {
        return WQ_ERROR;
    }
    if (src->size != dst->size) {
        return WQ_ERROR;
    }

    half = window_size / 2U;
    for (i = 0U; i < src->size; ++i) {
        size_t begin = (i > half) ? (i - half) : 0U;
        size_t end = i + half;
        size_t k;
        size_t count = 0U;
        double sum = 0.0;

        if (end >= src->size) {
            end = src->size - 1U;
        }

        for (k = begin; k <= end; ++k) {
            if (src->records[k].valid && !src->records[k].missing[(size_t)parameter]) {
                sum += src->records[k].value[(size_t)parameter];
                count++;
            }
        }

        if (count > 0U) {
            dst->records[i].value[(size_t)parameter] = sum / (double)count;
            dst->records[i].missing[(size_t)parameter] = false;
        }
    }
    return WQ_SUCCESS;
}

int wq_filter_main_parameters(const WaterQualityDataset *src,
                              WaterQualityDataset *dst,
                              size_t window_size)
{
    if (src == NULL || dst == NULL || !wq_is_filter_window_valid(window_size)) {
        return WQ_ERROR;
    }

    if (dst->capacity < src->size && wq_dataset_reserve(dst, src->size) != WQ_SUCCESS) {
        return WQ_ERROR;
    }
    memcpy(dst->records, src->records, src->size * sizeof(WaterQualityRecord));
    dst->size = src->size;

    if (wq_moving_average_filter(src, dst, WQ_PARAM_TEMP, window_size) != WQ_SUCCESS) return WQ_ERROR;
    if (wq_moving_average_filter(src, dst, WQ_PARAM_DO, window_size) != WQ_SUCCESS) return WQ_ERROR;
    if (wq_moving_average_filter(src, dst, WQ_PARAM_PH, window_size) != WQ_SUCCESS) return WQ_ERROR;
    if (wq_moving_average_filter(src, dst, WQ_PARAM_SALINITY, window_size) != WQ_SUCCESS) return WQ_ERROR;

    return WQ_SUCCESS;
}

int wq_preprocess_dataset(WaterQualityDataset *dataset, DataOverview *overview)
{
    WaterQualityDataset *filtered;
    size_t i;

    if (dataset == NULL || overview == NULL) {
        return WQ_ERROR;
    }

    if (wq_detect_outliers(dataset, overview) != WQ_SUCCESS) {
        return WQ_ERROR;
    }
    if (wq_process_outliers(dataset, overview) != WQ_SUCCESS) {
        return WQ_ERROR;
    }
    if (wq_fill_missing_values(dataset, 10U, 10U, overview) != WQ_SUCCESS) {
        return WQ_ERROR;
    }

    filtered = wq_dataset_create(dataset->size);
    if (filtered == NULL) {
        return WQ_ERROR;
    }
    overview->filter_stddev_before[WQ_PARAM_TEMP] = wq_compute_stddev_parameter(dataset, WQ_PARAM_TEMP);
    overview->filter_stddev_before[WQ_PARAM_DO] = wq_compute_stddev_parameter(dataset, WQ_PARAM_DO);
    overview->filter_stddev_before[WQ_PARAM_PH] = wq_compute_stddev_parameter(dataset, WQ_PARAM_PH);
    overview->filter_stddev_before[WQ_PARAM_SALINITY] = wq_compute_stddev_parameter(dataset, WQ_PARAM_SALINITY);

    if (wq_filter_main_parameters(dataset, filtered, 5U) != WQ_SUCCESS) {
        wq_dataset_destroy(filtered);
        return WQ_ERROR;
    }

    overview->filter_stddev_after[WQ_PARAM_TEMP] = wq_compute_stddev_parameter(filtered, WQ_PARAM_TEMP);
    overview->filter_stddev_after[WQ_PARAM_DO] = wq_compute_stddev_parameter(filtered, WQ_PARAM_DO);
    overview->filter_stddev_after[WQ_PARAM_PH] = wq_compute_stddev_parameter(filtered, WQ_PARAM_PH);
    overview->filter_stddev_after[WQ_PARAM_SALINITY] = wq_compute_stddev_parameter(filtered, WQ_PARAM_SALINITY);
    overview->filter_stddev_delta[WQ_PARAM_TEMP] = overview->filter_stddev_after[WQ_PARAM_TEMP] - overview->filter_stddev_before[WQ_PARAM_TEMP];
    overview->filter_stddev_delta[WQ_PARAM_DO] = overview->filter_stddev_after[WQ_PARAM_DO] - overview->filter_stddev_before[WQ_PARAM_DO];
    overview->filter_stddev_delta[WQ_PARAM_PH] = overview->filter_stddev_after[WQ_PARAM_PH] - overview->filter_stddev_before[WQ_PARAM_PH];
    overview->filter_stddev_delta[WQ_PARAM_SALINITY] = overview->filter_stddev_after[WQ_PARAM_SALINITY] - overview->filter_stddev_before[WQ_PARAM_SALINITY];

    for (i = 0U; i < dataset->size; ++i) {
        dataset->records[i] = filtered->records[i];
    }
    wq_dataset_destroy(filtered);

    overview->valid_records = dataset->size;
    overview->invalid_records = (overview->total_records >= overview->valid_records)
                              ? (overview->total_records - overview->valid_records)
                              : 0U;

    if (wq_write_csv(WQ_CLEAN_CSV_FILE, dataset) != WQ_SUCCESS) {
        return WQ_ERROR;
    }
    if (wq_write_binary(WQ_CLEAN_BIN_FILE, dataset) != WQ_SUCCESS) {
        return WQ_ERROR;
    }

    return WQ_SUCCESS;
}
