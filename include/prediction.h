#ifndef WQ_PREDICTION_H
#define WQ_PREDICTION_H

#include <stddef.h>
#include "types.h"

/*
 * prediction.h
 *
 * 单因素线性回归预测接口。
 * 主要用于建立 Air_temp -> DO、Temp -> DO、pH -> DO、Salinity -> DO 等模型。
 */

/* 训练单因素线性回归模型 y = a*x + b。 */
int wq_train_linear_regression(const WaterQualityDataset *dataset,
                               WQParameter x_param,
                               WQParameter y_param,
                               LinearRegressionModel *model);

/* 使用训练好的线性模型预测 y。 */
double wq_predict_by_linear_model(const LinearRegressionModel *model, double x_value);

/* 使用决定系数 R² 评估模型拟合效果。 */
double wq_evaluate_r_squared(const WaterQualityDataset *dataset,
                             const LinearRegressionModel *model);

/* 使用留出法评估模型，默认可按前 80% 训练、后 20% 测试。 */
double wq_evaluate_holdout_rmse(const WaterQualityDataset *dataset,
                                WQParameter x_param,
                                WQParameter y_param,
                                double train_ratio,
                                LinearRegressionModel *model_out);

/* 比较多个单因素对 DO 的预测效果，输出模型数组和模型数量。 */
int wq_compare_do_single_factor_models(const WaterQualityDataset *dataset,
                                       LinearRegressionModel *models,
                                       size_t max_models,
                                       size_t *model_count);

#endif
