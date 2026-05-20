#include "data_manage.h"
#include "preprocess.h"
#include <stdio.h>

/*
 * data_manage.c
 *
 * 数据查询、排序、修改、删除模块函数框架。
 */

void wq_print_record(const WaterQualityRecord *record)
{
    /* 待实现：按表格形式输出记录编号、时间和六个参数。 */
    (void)record;
}

void wq_print_page(const WaterQualityDataset *dataset, size_t page_index, size_t page_size)
{
    /* 待实现：分页显示，每页默认 15 条记录。 */
    (void)dataset;
    (void)page_index;
    (void)page_size;
}

size_t wq_get_total_pages(const WaterQualityDataset *dataset, size_t page_size)
{
    if (dataset == NULL || page_size == 0U) {
        return 0U;
    }

    return (dataset->size + page_size - 1U) / page_size;
}

int wq_filter_by_range(const WaterQualityDataset *src,
                       WaterQualityDataset *dst,
                       WQParameter parameter,
                       double min_value,
                       double max_value)
{
    /* 待实现：将参数值位于 [min_value, max_value] 的记录复制到 dst。 */
    (void)src;
    (void)dst;
    (void)parameter;
    (void)min_value;
    (void)max_value;
    return WQ_ERROR;
}

int wq_sort_dataset(WaterQualityDataset *dataset, WQParameter parameter, WQSortOrder order)
{
    /* 待实现：按指定参数升序或降序排序，可使用 qsort。 */
    (void)dataset;
    (void)parameter;
    (void)order;
    return WQ_ERROR;
}

int wq_modify_record_value(WaterQualityDataset *dataset,
                           size_t record_index,
                           WQParameter parameter,
                           double new_value)
{
    /* 修改前先做参数范围校验，真正保存由调用方决定。 */
    if (dataset == NULL || record_index >= dataset->size) {
        return WQ_ERROR;
    }

    if (!wq_is_value_in_range(parameter, new_value)) {
        return WQ_ERROR;
    }

    dataset->records[record_index].value[parameter] = new_value;
    dataset->records[record_index].missing[parameter] = false;
    return WQ_SUCCESS;
}

int wq_delete_record(WaterQualityDataset *dataset, size_t record_index)
{
    /* 待实现：删除单条记录，可将后续元素整体前移。 */
    (void)dataset;
    (void)record_index;
    return WQ_ERROR;
}

size_t wq_delete_records_by_range(WaterQualityDataset *dataset,
                                  WQParameter parameter,
                                  double min_value,
                                  double max_value)
{
    /* 待实现：按条件批量删除，返回实际删除数量。 */
    (void)dataset;
    (void)parameter;
    (void)min_value;
    (void)max_value;
    return 0U;
}
