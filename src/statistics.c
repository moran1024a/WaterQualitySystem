#include "statistics.h"
#include "utils.h"
#include <stdio.h>
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

static bool wq_valid_value(const WaterQualityRecord *r, WQParameter p)
{
    return r != NULL && r->valid && !r->missing[(size_t)p] && !isnan(r->value[(size_t)p]);
}

static void wq_fill_warning(WarningRecord *w,
                            const WQDateTime *time,
                            WQWarningType type,
                            const char *msg,
                            const char *sug)
{
    if (w == NULL || time == NULL) return;
    w->time = *time;
    w->type = type;
    if (msg != NULL) {
        strncpy(w->message, msg, sizeof(w->message) - 1U);
        w->message[sizeof(w->message) - 1U] = '\0';
    }
    if (sug != NULL) {
        strncpy(w->suggestion, sug, sizeof(w->suggestion) - 1U);
        w->suggestion[sizeof(w->suggestion) - 1U] = '\0';
    }
}

int wq_compute_parameter_statistics(const WaterQualityDataset *dataset,
                                    WQParameter parameter,
                                    ParameterStatistics *result)
{
    size_t i;
    double sum = 0.0;
    double min_v = 0.0;
    double max_v = 0.0;
    size_t count = 0U;

    if (dataset == NULL || result == NULL || parameter < 0 || parameter >= WQ_PARAM_COUNT) return WQ_ERROR;

    for (i = 0U; i < dataset->size; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        double v;
        if (!wq_valid_value(r, parameter)) continue;
        v = r->value[(size_t)parameter];
        if (count == 0U) { min_v = v; max_v = v; } else { if (v < min_v) min_v = v; if (v > max_v) max_v = v; }
        sum += v;
        count++;
    }

    if (count == 0U) return WQ_ERROR;

    result->count = count;
    result->mean = sum / (double)count;
    result->min = min_v;
    result->max = max_v;
    result->stddev = wq_compute_stddev(dataset, parameter, result->mean);
    return WQ_SUCCESS;
}

double wq_compute_mean(const WaterQualityDataset *dataset, WQParameter parameter)
{
    size_t i;
    size_t count = 0U;
    double sum = 0.0;
    if (dataset == NULL || parameter < 0 || parameter >= WQ_PARAM_COUNT) return 0.0;
    for (i = 0U; i < dataset->size; ++i) {
        if (!wq_valid_value(&dataset->records[i], parameter)) continue;
        sum += dataset->records[i].value[(size_t)parameter];
        count++;
    }
    return count > 0U ? (sum / (double)count) : 0.0;
}

double wq_compute_stddev(const WaterQualityDataset *dataset,
                         WQParameter parameter,
                         double mean)
{
    size_t i;
    size_t count = 0U;
    double sum_sq = 0.0;
    if (dataset == NULL || parameter < 0 || parameter >= WQ_PARAM_COUNT) return 0.0;
    for (i = 0U; i < dataset->size; ++i) {
        double d;
        if (!wq_valid_value(&dataset->records[i], parameter)) continue;
        d = dataset->records[i].value[(size_t)parameter] - mean;
        sum_sq += d * d;
        count++;
    }
    return count > 1U ? wq_sqrt_local(sum_sq / (double)count) : 0.0;
}

double wq_pearson_correlation(const WaterQualityDataset *dataset,
                              WQParameter x_param,
                              WQParameter y_param)
{
    size_t i;
    size_t n = 0U;
    double sx = 0.0, sy = 0.0, sxx = 0.0, syy = 0.0, sxy = 0.0;
    double num, denx, deny;
    if (dataset == NULL || x_param < 0 || y_param < 0 || x_param >= WQ_PARAM_COUNT || y_param >= WQ_PARAM_COUNT) return 0.0;

    for (i = 0U; i < dataset->size; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        double x, y;
        if (!wq_valid_value(r, x_param) || !wq_valid_value(r, y_param)) continue;
        x = r->value[(size_t)x_param];
        y = r->value[(size_t)y_param];
        sx += x; sy += y; sxx += x * x; syy += y * y; sxy += x * y;
        n++;
    }
    if (n < 2U) return 0.0;
    num = (double)n * sxy - sx * sy;
    denx = (double)n * sxx - sx * sx;
    deny = (double)n * syy - sy * sy;
    if (denx <= 0.0 || deny <= 0.0) return 0.0;
    return num / wq_sqrt_local(denx * deny);
}

int wq_compute_correlation_matrix(const WaterQualityDataset *dataset,
                                  double matrix[WQ_PARAM_COUNT][WQ_PARAM_COUNT])
{
    size_t i, j;
    if (dataset == NULL || matrix == NULL) return WQ_ERROR;
    for (i = 0U; i < (size_t)WQ_PARAM_COUNT; ++i) {
        for (j = 0U; j < (size_t)WQ_PARAM_COUNT; ++j) {
            matrix[i][j] = (i == j) ? 1.0 : wq_pearson_correlation(dataset, (WQParameter)i, (WQParameter)j);
        }
    }
    return WQ_SUCCESS;
}

int wq_compute_all_statistics(const WaterQualityDataset *dataset,
                              StatisticsResult *result)
{
    size_t p;
    if (dataset == NULL || result == NULL) return WQ_ERROR;
    for (p = 0U; p < (size_t)WQ_PARAM_COUNT; ++p) {
        if (wq_compute_parameter_statistics(dataset, (WQParameter)p, &result->parameter_stats[p]) != WQ_SUCCESS) {
            memset(&result->parameter_stats[p], 0, sizeof(result->parameter_stats[p]));
        }
    }
    return wq_compute_correlation_matrix(dataset, result->correlation_matrix);
}

int wq_analyze_dawn_hypoxia(const WaterQualityDataset *dataset,
                            WarningRecord *warnings,
                            size_t max_warnings,
                            size_t *warning_count)
{
    size_t i;
    size_t count = (warning_count != NULL) ? *warning_count : 0U;
    double day_sum = 0.0;
    size_t day_n = 0U;
    int cur_y = -1, cur_m = -1, cur_d = -1;
    WQDateTime cur_time = {0};

    if (dataset == NULL || warnings == NULL || warning_count == NULL) return WQ_ERROR;

    for (i = 0U; i < dataset->size; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        int h = r->time.hour;
        if (cur_y != r->time.year || cur_m != r->time.month || cur_d != r->time.day) {
            if (day_n > 0U && count < max_warnings) {
                double m = day_sum / (double)day_n;
                if (m < WQ_DO_SEVERE_HYPOXIA_LIMIT) {
                    wq_fill_warning(&warnings[count++], &cur_time, WQ_WARNING_SEVERE_HYPOXIA, "凌晨DO均值低于3.0mg/L", "需立即投放颗粒氧并减少投喂");
                } else if (m < WQ_DO_MILD_HYPOXIA_LIMIT) {
                    wq_fill_warning(&warnings[count++], &cur_time, WQ_WARNING_MILD_HYPOXIA, "凌晨DO均值低于4.0mg/L", "建议开启底部增氧机");
                }
            }
            cur_y = r->time.year; cur_m = r->time.month; cur_d = r->time.day; cur_time = r->time;
            day_sum = 0.0; day_n = 0U;
        }
        if (h >= 3 && h < 5 && wq_valid_value(r, WQ_PARAM_DO)) {
            day_sum += r->value[WQ_PARAM_DO];
            day_n++;
        }
    }

    if (day_n > 0U && count < max_warnings) {
        double m = day_sum / (double)day_n;
        if (m < WQ_DO_SEVERE_HYPOXIA_LIMIT) {
            wq_fill_warning(&warnings[count++], &cur_time, WQ_WARNING_SEVERE_HYPOXIA, "凌晨DO均值低于3.0mg/L", "需立即投放颗粒氧并减少投喂");
        } else if (m < WQ_DO_MILD_HYPOXIA_LIMIT) {
            wq_fill_warning(&warnings[count++], &cur_time, WQ_WARNING_MILD_HYPOXIA, "凌晨DO均值低于4.0mg/L", "建议开启底部增氧机");
        }
    }

    *warning_count = count;
    return WQ_SUCCESS;
}

int wq_analyze_salinity_mutation(const WaterQualityDataset *dataset,
                                 WarningRecord *warnings,
                                 size_t max_warnings,
                                 size_t *warning_count)
{
    size_t i;
    size_t count;
    if (dataset == NULL || warnings == NULL || warning_count == NULL) return WQ_ERROR;
    count = *warning_count;
    for (i = 0U; i < dataset->size; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        if (!wq_valid_value(r, WQ_PARAM_SALINITY)) continue;

        if (i >= WQ_HOUR_RECORDS && count < max_warnings && wq_valid_value(&dataset->records[i - WQ_HOUR_RECORDS], WQ_PARAM_SALINITY)) {
            double drop1h = dataset->records[i - WQ_HOUR_RECORDS].value[WQ_PARAM_SALINITY] - r->value[WQ_PARAM_SALINITY];
            if (drop1h > WQ_SALINITY_HOUR_DROP_LIMIT) {
                wq_fill_warning(&warnings[count++], &r->time, WQ_WARNING_SALINITY_MUTATION, "1小时盐度下降超过2 PSU", "立即关闭进水口，并泼洒高稳VC或葡萄糖以增强抗应激能力");
            }
        }

        if (i >= WQ_DAY_RECORDS && count < max_warnings && wq_valid_value(&dataset->records[i - WQ_DAY_RECORDS], WQ_PARAM_SALINITY)) {
            double drop24h = dataset->records[i - WQ_DAY_RECORDS].value[WQ_PARAM_SALINITY] - r->value[WQ_PARAM_SALINITY];
            if (drop24h > WQ_SALINITY_DAY_DROP_LIMIT) {
                wq_fill_warning(&warnings[count++], &r->time, WQ_WARNING_SALINITY_MUTATION, "24小时盐度累计下降超过5 PSU", "立即关闭进水口，并泼洒高稳VC或葡萄糖以增强抗应激能力");
            }
        }
    }
    *warning_count = count;
    return WQ_SUCCESS;
}

int wq_analyze_segment_warnings(const WaterQualityDataset *dataset,
                                WarningRecord *warnings,
                                size_t max_warnings,
                                size_t *warning_count)
{
    size_t count = 0U;
    if (dataset == NULL || warnings == NULL || warning_count == NULL) return WQ_ERROR;
    if (wq_analyze_dawn_hypoxia(dataset, warnings, max_warnings, &count) != WQ_SUCCESS) return WQ_ERROR;
    if (wq_analyze_salinity_mutation(dataset, warnings, max_warnings, &count) != WQ_SUCCESS) return WQ_ERROR;
    *warning_count = count;
    return WQ_SUCCESS;
}
