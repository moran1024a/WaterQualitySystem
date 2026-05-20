#include "prediction.h"

/*
 * prediction.c
 *
 * 单因素线性回归预测模块函数框架。
 */

int wq_train_linear_regression(const WaterQualityDataset *dataset,
                               WQParameter x_param,
                               WQParameter y_param,
                               LinearRegressionModel *model)
{
    /* 待实现：用最小二乘法计算 slope 和 intercept。 */
    (void)dataset;
    (void)x_param;
    (void)y_param;
    (void)model;
    return WQ_ERROR;
}

double wq_predict_by_linear_model(const LinearRegressionModel *model, double x_value)
{
    if (model == NULL) {
        return 0.0;
    }

    return model->slope * x_value + model->intercept;
}

double wq_evaluate_r_squared(const WaterQualityDataset *dataset,
                             const LinearRegressionModel *model)
{
    /* 待实现：计算决定系数 R²，用于评估拟合程度。 */
    (void)dataset;
    (void)model;
    return 0.0;
}

double wq_evaluate_holdout_rmse(const WaterQualityDataset *dataset,
                                WQParameter x_param,
                                WQParameter y_param,
                                double train_ratio,
                                LinearRegressionModel *model_out)
{
    /* 待实现：前 train_ratio 部分训练，剩余部分测试并计算 RMSE。 */
    (void)dataset;
    (void)x_param;
    (void)y_param;
    (void)train_ratio;
    (void)model_out;
    return 0.0;
}

int wq_compare_do_single_factor_models(const WaterQualityDataset *dataset,
                                       LinearRegressionModel *models,
                                       size_t max_models,
                                       size_t *model_count)
{
    /* 待实现：比较 Air_temp、Temp、pH、Salinity 等变量预测 DO 的效果。 */
    (void)dataset;
    (void)models;
    (void)max_models;
    (void)model_count;
    return WQ_ERROR;
}
