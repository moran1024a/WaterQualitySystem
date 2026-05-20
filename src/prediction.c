#include "prediction.h"
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

static bool wq_valid_xy(const WaterQualityRecord *r, WQParameter x, WQParameter y)
{
    return r != NULL && r->valid && !r->missing[(size_t)x] && !r->missing[(size_t)y] &&
           !isnan(r->value[(size_t)x]) && !isnan(r->value[(size_t)y]);
}

int wq_train_linear_regression(const WaterQualityDataset *dataset,
                               WQParameter x_param,
                               WQParameter y_param,
                               LinearRegressionModel *model)
{
    size_t i, n = 0U;
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    double den;
    if (dataset == NULL || model == NULL) return WQ_ERROR;

    for (i = 0U; i < dataset->size; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        double x, y;
        if (!wq_valid_xy(r, x_param, y_param)) continue;
        x = r->value[(size_t)x_param];
        y = r->value[(size_t)y_param];
        sx += x; sy += y; sxx += x * x; sxy += x * y; n++;
    }
    if (n < 2U) return WQ_ERROR;
    den = (double)n * sxx - sx * sx;
    if (den == 0.0) return WQ_ERROR;

    memset(model, 0, sizeof(*model));
    model->x_param = x_param;
    model->y_param = y_param;
    model->slope = (((double)n * sxy) - sx * sy) / den;
    model->intercept = (sy - model->slope * sx) / (double)n;
    return WQ_SUCCESS;
}

double wq_predict_by_linear_model(const LinearRegressionModel *model, double x_value)
{
    if (model == NULL) return 0.0;
    return model->slope * x_value + model->intercept;
}

double wq_evaluate_r_squared(const WaterQualityDataset *dataset,
                             const LinearRegressionModel *model)
{
    size_t i;
    size_t n = 0U;
    double y_mean = 0.0, ss_res = 0.0, ss_tot = 0.0;
    if (dataset == NULL || model == NULL) return 0.0;

    for (i = 0U; i < dataset->size; ++i) {
        if (!wq_valid_xy(&dataset->records[i], model->x_param, model->y_param)) continue;
        y_mean += dataset->records[i].value[(size_t)model->y_param];
        n++;
    }
    if (n < 2U) return 0.0;
    y_mean /= (double)n;

    for (i = 0U; i < dataset->size; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        double y, yhat, d;
        if (!wq_valid_xy(r, model->x_param, model->y_param)) continue;
        y = r->value[(size_t)model->y_param];
        yhat = wq_predict_by_linear_model(model, r->value[(size_t)model->x_param]);
        d = y - yhat;
        ss_res += d * d;
        d = y - y_mean;
        ss_tot += d * d;
    }
    if (ss_tot == 0.0) return 0.0;
    return 1.0 - (ss_res / ss_tot);
}

double wq_evaluate_holdout_rmse(const WaterQualityDataset *dataset,
                                WQParameter x_param,
                                WQParameter y_param,
                                double train_ratio,
                                LinearRegressionModel *model_out)
{
    size_t i;
    size_t split;
    size_t train_n = 0U;
    double sx = 0.0, sy = 0.0, sxx = 0.0, sxy = 0.0;
    size_t test_n = 0U;
    double mse = 0.0;
    LinearRegressionModel m;

    if (dataset == NULL || model_out == NULL || train_ratio <= 0.0 || train_ratio >= 1.0 || dataset->size < 10U) return -1.0;

    split = (size_t)((double)dataset->size * train_ratio);
    if (split < 2U || split >= dataset->size) return -1.0;

    for (i = 0U; i < split; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        if (!wq_valid_xy(r, x_param, y_param)) continue;
        sx += r->value[(size_t)x_param];
        sy += r->value[(size_t)y_param];
        sxx += r->value[(size_t)x_param] * r->value[(size_t)x_param];
        sxy += r->value[(size_t)x_param] * r->value[(size_t)y_param];
        train_n++;
    }
    if (train_n < 2U) return -1.0;
    {
        double den = (double)train_n * sxx - sx * sx;
        if (den == 0.0) return -1.0;
        m.x_param = x_param;
        m.y_param = y_param;
        m.slope = (((double)train_n * sxy) - sx * sy) / den;
        m.intercept = (sy - m.slope * sx) / (double)train_n;
    }

    for (i = split; i < dataset->size; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        double err;
        if (!wq_valid_xy(r, x_param, y_param)) continue;
        err = r->value[(size_t)y_param] - wq_predict_by_linear_model(&m, r->value[(size_t)x_param]);
        mse += err * err;
        test_n++;
    }
    if (test_n == 0U) return -1.0;

    m.r_squared = wq_evaluate_r_squared(dataset, &m);
    m.rmse = wq_sqrt_local(mse / (double)test_n);
    *model_out = m;
    return m.rmse;
}

int wq_compare_do_single_factor_models(const WaterQualityDataset *dataset,
                                       LinearRegressionModel *models,
                                       size_t max_models,
                                       size_t *model_count)
{
    const WQParameter xs[4] = {WQ_PARAM_AIR_TEMP, WQ_PARAM_TEMP, WQ_PARAM_PH, WQ_PARAM_SALINITY};
    size_t i;
    size_t n = 0U;
    if (dataset == NULL || models == NULL || model_count == NULL || max_models < 4U) return WQ_ERROR;

    for (i = 0U; i < 4U; ++i) {
        LinearRegressionModel m;
        if (wq_train_linear_regression(dataset, xs[i], WQ_PARAM_DO, &m) != WQ_SUCCESS) continue;
        m.r_squared = wq_evaluate_r_squared(dataset, &m);
        {
            LinearRegressionModel h;
            double rmse = wq_evaluate_holdout_rmse(dataset, xs[i], WQ_PARAM_DO, 0.8, &h);
            m.rmse = rmse >= 0.0 ? rmse : 0.0;
        }
        models[n++] = m;
        if (n >= max_models) break;
    }
    *model_count = n;
    return n > 0U ? WQ_SUCCESS : WQ_ERROR;
}
