#ifndef WQ_STATISTICS_H
#define WQ_STATISTICS_H

#include <stddef.h>
#include "types.h"

/*
 * statistics.h
 *
 * 统计分析接口。
 * 包含基本统计量、分段预警分析和皮尔逊相关性分析。
 */

/* 计算某一参数的均值、最大值、最小值和标准差。 */
int wq_compute_parameter_statistics(const WaterQualityDataset *dataset,
                                    WQParameter parameter,
                                    ParameterStatistics *result);

/* 计算某一参数的均值。 */
double wq_compute_mean(const WaterQualityDataset *dataset, WQParameter parameter);

/* 已知均值时，计算某一参数的标准差。 */
double wq_compute_stddev(const WaterQualityDataset *dataset,
                         WQParameter parameter,
                         double mean);

/* 计算两个参数之间的皮尔逊相关系数。 */
double wq_pearson_correlation(const WaterQualityDataset *dataset,
                              WQParameter x_param,
                              WQParameter y_param);

/* 计算完整 6×6 相关系数矩阵。 */
int wq_compute_correlation_matrix(const WaterQualityDataset *dataset,
                                  double matrix[WQ_PARAM_COUNT][WQ_PARAM_COUNT]);

/* 计算全部统计分析结果。 */
int wq_compute_all_statistics(const WaterQualityDataset *dataset,
                              StatisticsResult *result);

/* 分析每天 03:00 至 05:00 的 DO，生成凌晨缺氧预警。 */
int wq_analyze_dawn_hypoxia(const WaterQualityDataset *dataset,
                            WarningRecord *warnings,
                            size_t max_warnings,
                            size_t *warning_count);

/* 分析 1 小时和 24 小时盐度下降幅度，生成盐度突变预警。 */
int wq_analyze_salinity_mutation(const WaterQualityDataset *dataset,
                                 WarningRecord *warnings,
                                 size_t max_warnings,
                                 size_t *warning_count);

/* 统一执行所有分段预警分析。 */
int wq_analyze_segment_warnings(const WaterQualityDataset *dataset,
                                WarningRecord *warnings,
                                size_t max_warnings,
                                size_t *warning_count);

#endif
