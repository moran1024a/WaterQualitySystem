#ifndef WQ_REPORT_H
#define WQ_REPORT_H

#include <stddef.h>
#include "types.h"

/*
 * report.h
 *
 * 报告输出接口。
 * 统一负责 overview、statistics、warning、prediction 等文本报告写入与查看。
 */

/* 写入数据概览报告。 */
int wq_write_overview_report(const char *filename, const DataOverview *overview);

/* 写入统计分析报告，包括基本统计量和相关系数矩阵。 */
int wq_write_statistics_report(const char *filename,
                               const StatisticsResult *statistics,
                               const WaterQualityDataset *dataset);

/* 写入预警报告。 */
int wq_write_warning_report(const char *filename,
                            const WarningRecord *warnings,
                            size_t warning_count);

/* 写入预测模型报告，包括模型参数、R²、RMSE 等。 */
int wq_write_prediction_report(const char *filename,
                               const LinearRegressionModel *models,
                               size_t model_count);

/* 在控制台查看指定文本报告。 */
int wq_view_text_report(const char *filename);

#endif
