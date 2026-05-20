#ifndef WQ_DATA_MANAGE_H
#define WQ_DATA_MANAGE_H

#include <stddef.h>
#include "types.h"

/*
 * data_manage.h
 *
 * 数据查询、显示、排序、修改与删除接口。
 * 修改和删除前的备份确认应由菜单层或调用方配合 backup 模块完成。
 */

/* 按统一格式输出一条记录。 */
void wq_print_record(const WaterQualityRecord *record);

/* 分页输出数据。page_index 建议从 0 开始。 */
void wq_print_page(const WaterQualityDataset *dataset, size_t page_index, size_t page_size);

/* 根据记录数和 page_size 计算总页数。 */
size_t wq_get_total_pages(const WaterQualityDataset *dataset, size_t page_size);

/* 按参数范围筛选记录，结果写入 dst。 */
int wq_filter_by_range(const WaterQualityDataset *src,
                       WaterQualityDataset *dst,
                       WQParameter parameter,
                       double min_value,
                       double max_value);

/* 按指定参数升序或降序排序。 */
int wq_sort_dataset(WaterQualityDataset *dataset, WQParameter parameter, WQSortOrder order);

/* 修改指定记录的指定参数，新值应先通过范围检查。 */
int wq_modify_record_value(WaterQualityDataset *dataset,
                           size_t record_index,
                           WQParameter parameter,
                           double new_value);

/* 删除单条记录。record_index 为数组下标。 */
int wq_delete_record(WaterQualityDataset *dataset, size_t record_index);

/* 按参数范围批量删除记录，返回删除数量。 */
size_t wq_delete_records_by_range(WaterQualityDataset *dataset,
                                  WQParameter parameter,
                                  double min_value,
                                  double max_value);

#endif
