#ifndef WQ_REPORT_H
#define WQ_REPORT_H

#include <stddef.h>
#include <stdbool.h>
#include "types.h"

/*
 * report.h
 *
 * 报告输出接口。
 * 统一负责 overview、statistics、warning、prediction 以及分析讨论段落的文本报告写入与查看。
 * 当前只定义接口与约定，具体文件格式和文本内容由后续实现阶段补充。
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

/*
 * 写入或追加“分析讨论”段落。
 * 适用内容包括：
 * 1. CSV 与二进制存储适用场景、空间/时间差异；
 * 2. 异常值处理方法及合理性；
 * 3. 滤波窗口大小与噪声抑制关系；
 * 4. 相关性矩阵结论；
 * 5. 单因素线性回归准确度、原因与局限。
 */
int wq_write_discussion_report(const char *filename,
                               const AnalysisDiscussion *discussions,
                               size_t discussion_count,
                               bool append);

/* 根据讨论主题返回中文名称，便于报告标题统一。 */
const char *wq_discussion_topic_to_string(WQDiscussionTopic topic);

/* 在控制台查看指定文本报告。 */
int wq_view_text_report(const char *filename);

#endif
