#ifndef WQ_PREPROCESS_H
#define WQ_PREPROCESS_H

#include <stddef.h>
#include <stdbool.h>
#include "types.h"

/*
 * preprocess.h
 *
 * 数据预处理接口。
 * 包含缺失值识别、异常值检测、均值逼近填充和移动平均滤波。
 */

/* 判断文本字段是否表示缺失值，如空字符串、NaN、nan。 */
bool wq_is_missing_text(const char *text);

/* 判断数值是否为约定的缺失值标记。 */
bool wq_is_missing_value(double value);

/* 判断指定参数值是否处于任务书规定的合理范围内。 */
bool wq_is_value_in_range(WQParameter parameter, double value);

/* 统计单条记录中异常参数的数量。 */
size_t wq_count_record_outliers(const WaterQualityRecord *record);

/* 只检测异常值并更新概览，不修改数据。 */
int wq_detect_outliers(const WaterQualityDataset *dataset, DataOverview *overview);

/* 处理异常值：异常参数数 >= 3 删除整条记录，否则标记后交给缺失值填充。 */
int wq_process_outliers(WaterQualityDataset *dataset, DataOverview *overview);

/* 使用均值逼近法填充缺失值，before_n 和 after_m 通常取 10。 */
int wq_fill_missing_values(WaterQualityDataset *dataset,
                           size_t before_n,
                           size_t after_m,
                           DataOverview *overview);

/* 对单个参数执行移动平均滤波。dst 用于保存滤波后的数据。 */
int wq_moving_average_filter(const WaterQualityDataset *src,
                             WaterQualityDataset *dst,
                             WQParameter parameter,
                             size_t window_size);

/* 对水温、盐度、pH、DO 等主要参数执行指定窗口滤波。 */
int wq_filter_main_parameters(const WaterQualityDataset *src,
                              WaterQualityDataset *dst,
                              size_t window_size);

/* 一键执行完整预处理流程：异常值处理、缺失值填充、必要的统计更新。 */
int wq_preprocess_dataset(WaterQualityDataset *dataset, DataOverview *overview);

#endif
