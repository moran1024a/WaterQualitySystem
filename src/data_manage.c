#include "data_manage.h"
#include "preprocess.h"
#include "file_io.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"

static WQParameter g_sort_param = WQ_PARAM_TEMP;
static WQSortOrder g_sort_order = WQ_SORT_ASC;

static int wq_cmp_record(const void *a, const void *b)
{
    const WaterQualityRecord *ra = (const WaterQualityRecord *)a;
    const WaterQualityRecord *rb = (const WaterQualityRecord *)b;
    size_t p = (size_t)g_sort_param;
    bool ma = ra->missing[p];
    bool mb = rb->missing[p];
    if (ma && mb) return 0;
    if (ma) return 1;
    if (mb) return -1;
    if (ra->value[p] < rb->value[p]) return (g_sort_order == WQ_SORT_ASC) ? -1 : 1;
    if (ra->value[p] > rb->value[p]) return (g_sort_order == WQ_SORT_ASC) ? 1 : -1;
    return 0;
}

void wq_print_record(const WaterQualityRecord *record)
{
    size_t i;
    if (record == NULL) return;
    printf("[%lu] %04d-%02d-%02d %02d:%02d ", (unsigned long)record->index,
           record->time.year, record->time.month, record->time.day, record->time.hour, record->time.minute);
    for (i = 0U; i < (size_t)WQ_PARAM_COUNT; ++i) {
        if (record->missing[i]) printf("| %s=NaN ", wq_parameter_to_string((WQParameter)i));
        else printf("| %s=%.3f ", wq_parameter_to_string((WQParameter)i), record->value[i]);
    }
    printf("\n");
}

void wq_print_page(const WaterQualityDataset *dataset, size_t page_index, size_t page_size)
{
    size_t start, end, i;
    if (dataset == NULL || dataset->size == 0U) {
        printf("数据为空。\n");
        return;
    }
    if (page_size == 0U) page_size = WQ_PAGE_SIZE;
    if (page_index >= wq_get_total_pages(dataset, page_size)) {
        printf("页码越界。\n");
        return;
    }
    start = page_index * page_size;
    end = start + page_size;
    if (end > dataset->size) end = dataset->size;
    for (i = start; i < end; ++i) wq_print_record(&dataset->records[i]);
}

size_t wq_get_total_pages(const WaterQualityDataset *dataset, size_t page_size)
{
    if (dataset == NULL || page_size == 0U) return 0U;
    return (dataset->size + page_size - 1U) / page_size;
}

int wq_filter_by_range(const WaterQualityDataset *src,
                       WaterQualityDataset *dst,
                       WQParameter parameter,
                       double min_value,
                       double max_value)
{
    size_t i;
    if (src == NULL || dst == NULL || parameter < 0 || parameter >= WQ_PARAM_COUNT || min_value > max_value) return WQ_ERROR;
    wq_dataset_clear(dst);
    if (dst->capacity < src->size && wq_dataset_reserve(dst, src->size) != WQ_SUCCESS) return WQ_ERROR;
    for (i = 0U; i < src->size; ++i) {
        const WaterQualityRecord *r = &src->records[i];
        if (!r->missing[(size_t)parameter] && r->value[(size_t)parameter] >= min_value && r->value[(size_t)parameter] <= max_value) {
            if (wq_dataset_push(dst, r) != WQ_SUCCESS) return WQ_ERROR;
        }
    }
    return WQ_SUCCESS;
}

int wq_sort_dataset(WaterQualityDataset *dataset, WQParameter parameter, WQSortOrder order)
{
    if (dataset == NULL || parameter < 0 || parameter >= WQ_PARAM_COUNT) return WQ_ERROR;
    g_sort_param = parameter;
    g_sort_order = order;
    qsort(dataset->records, dataset->size, sizeof(WaterQualityRecord), wq_cmp_record);
    return WQ_SUCCESS;
}

int wq_modify_record_value(WaterQualityDataset *dataset,
                           size_t record_index,
                           WQParameter parameter,
                           double new_value)
{
    if (dataset == NULL || record_index >= dataset->size) return WQ_ERROR;
    if (!wq_is_value_in_range(parameter, new_value)) return WQ_ERROR;
    dataset->records[record_index].value[(size_t)parameter] = new_value;
    dataset->records[record_index].missing[(size_t)parameter] = false;
    return WQ_SUCCESS;
}

int wq_delete_record(WaterQualityDataset *dataset, size_t record_index)
{
    size_t i;
    if (dataset == NULL || record_index >= dataset->size) return WQ_ERROR;
    for (i = record_index + 1U; i < dataset->size; ++i) dataset->records[i - 1U] = dataset->records[i];
    dataset->size--;
    return WQ_SUCCESS;
}

size_t wq_delete_records_by_range(WaterQualityDataset *dataset,
                                  WQParameter parameter,
                                  double min_value,
                                  double max_value)
{
    size_t i, kept = 0U, deleted = 0U;
    if (dataset == NULL || parameter < 0 || parameter >= WQ_PARAM_COUNT || min_value > max_value) return 0U;
    for (i = 0U; i < dataset->size; ++i) {
        WaterQualityRecord r = dataset->records[i];
        bool hit = !r.missing[(size_t)parameter] && r.value[(size_t)parameter] >= min_value && r.value[(size_t)parameter] <= max_value;
        if (hit) deleted++;
        else dataset->records[kept++] = r;
    }
    dataset->size = kept;
    return deleted;
}
