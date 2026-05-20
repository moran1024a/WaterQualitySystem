#include "report.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

static const char *wq_warning_type_to_string(WQWarningType type)
{
    switch (type) {
    case WQ_WARNING_MILD_HYPOXIA: return "亚缺氧";
    case WQ_WARNING_SEVERE_HYPOXIA: return "严重缺氧";
    case WQ_WARNING_SALINITY_MUTATION: return "盐度突变";
    default: return "未知";
    }
}

static int wq_print_time(FILE *fp, const WQDateTime *t)
{
    return fprintf(fp, "%04d-%02d-%02d %02d:%02d", t->year, t->month, t->day, t->hour, t->minute);
}

int wq_write_overview_report(const char *filename, const DataOverview *overview)
{
    FILE *fp;
    if (filename == NULL || overview == NULL) return WQ_ERROR;
    if (wq_ensure_directory("output") != WQ_SUCCESS || wq_ensure_directory("output/reports") != WQ_SUCCESS || wq_ensure_directory("output/clean") != WQ_SUCCESS || wq_ensure_directory("backups") != WQ_SUCCESS) return WQ_ERROR;
    fp = fopen(filename, "w");
    if (fp == NULL) return WQ_ERROR;

    fprintf(fp, "=== 数据概览报告 ===\n");
    fprintf(fp, "总记录数: %lu\n", (unsigned long)overview->total_records);
    fprintf(fp, "有效记录数: %lu\n", (unsigned long)overview->valid_records);
    fprintf(fp, "无效/删除记录数: %lu\n", (unsigned long)overview->invalid_records);
    fprintf(fp, "缺失值数量: %lu\n", (unsigned long)overview->missing_values);
    fprintf(fp, "修复缺失值数量: %lu\n", (unsigned long)overview->fixed_missing_values);
    fprintf(fp, "异常记录数: %lu\n", (unsigned long)overview->outlier_records);
    fprintf(fp, "异常字段数量: %lu\n", (unsigned long)overview->outlier_parameter_count);
    fprintf(fp, "修复异常记录数: %lu\n", (unsigned long)overview->fixed_outlier_records);
    fprintf(fp, "删除异常记录数: %lu\n", (unsigned long)overview->deleted_outlier_records);
    fprintf(fp, "异常时间跨度: ");
    wq_print_time(fp, &overview->first_outlier_time);
    fprintf(fp, " ~ ");
    wq_print_time(fp, &overview->last_outlier_time);
    fprintf(fp, "\n");
    fprintf(fp, "清洗CSV路径: %s\n清洗二进制路径: %s\n", WQ_CLEAN_CSV_FILE, WQ_CLEAN_BIN_FILE);

    fprintf(fp, "\n[滤波标准差变化]\n");
    fprintf(fp, "Temp: %.6f -> %.6f (Δ=%.6f)\n", overview->filter_stddev_before[WQ_PARAM_TEMP], overview->filter_stddev_after[WQ_PARAM_TEMP], overview->filter_stddev_delta[WQ_PARAM_TEMP]);
    fprintf(fp, "DO: %.6f -> %.6f (Δ=%.6f)\n", overview->filter_stddev_before[WQ_PARAM_DO], overview->filter_stddev_after[WQ_PARAM_DO], overview->filter_stddev_delta[WQ_PARAM_DO]);
    fprintf(fp, "pH: %.6f -> %.6f (Δ=%.6f)\n", overview->filter_stddev_before[WQ_PARAM_PH], overview->filter_stddev_after[WQ_PARAM_PH], overview->filter_stddev_delta[WQ_PARAM_PH]);
    fprintf(fp, "Salinity: %.6f -> %.6f (Δ=%.6f)\n", overview->filter_stddev_before[WQ_PARAM_SALINITY], overview->filter_stddev_after[WQ_PARAM_SALINITY], overview->filter_stddev_delta[WQ_PARAM_SALINITY]);

    { FILE *f1=fopen(WQ_CLEAN_CSV_FILE,"rb"), *f2=fopen(WQ_CLEAN_BIN_FILE,"rb"); long s1=-1,s2=-1;
      if(f1){fseek(f1,0,SEEK_END);s1=ftell(f1);fclose(f1);} if(f2){fseek(f2,0,SEEK_END);s2=ftell(f2);fclose(f2);}
      fprintf(fp,"\n[存储对比数值]\nCSV文件大小: %ld bytes\nBIN文件大小: %ld bytes\nCSV写入时间: N/A\nBIN写入时间: N/A\nCSV读取时间: N/A\nBIN读取时间: N/A\nCSV人类可读: 是\nBIN人类可读: 否\n",s1,s2); }

    fprintf(fp, "\n[存储格式讨论]\nCSV可读性高，适合人工检查；二进制读写快、体积小，适合系统运行期缓存。\n");
    fprintf(fp, "实际大小与理论大小差异受字段文本长度、换行与格式化精度影响。\n");

    return (fclose(fp) == 0) ? WQ_SUCCESS : WQ_ERROR;
}

int wq_write_statistics_report(const char *filename,
                               const StatisticsResult *statistics,
                               const WaterQualityDataset *dataset)
{
    FILE *fp;
    size_t i, j;
    double max_corr = -2.0, min_corr = 2.0;
    int max_i = -1, max_j = -1, min_i = -1, min_j = -1;
    (void)dataset;
    if (filename == NULL || statistics == NULL) return WQ_ERROR;
    fp = fopen(filename, "w");
    if (fp == NULL) return WQ_ERROR;

    fprintf(fp, "=== 统计分析报告 ===\n\n[六参数统计量]\n");
    for (i = 0U; i < (size_t)WQ_PARAM_COUNT; ++i) {
        const ParameterStatistics *ps = &statistics->parameter_stats[i];
        fprintf(fp, "%s: mean=%.6f min=%.6f max=%.6f stddev=%.6f n=%lu\n",
                wq_parameter_to_string((WQParameter)i), ps->mean, ps->min, ps->max, ps->stddev, (unsigned long)ps->count);
    }

    fprintf(fp, "\n[Pearson相关矩阵]\n");
    for (i = 0U; i < (size_t)WQ_PARAM_COUNT; ++i) {
        for (j = 0U; j < (size_t)WQ_PARAM_COUNT; ++j) {
            double c = statistics->correlation_matrix[i][j];
            fprintf(fp, "%8.4f ", c);
            if (i != j) {
                if (c > max_corr) { max_corr = c; max_i = (int)i; max_j = (int)j; }
                if (c < min_corr) { min_corr = c; min_i = (int)i; min_j = (int)j; }
            }
        }
        fputc('\n', fp);
    }

    fprintf(fp, "\n最强正相关: %s-%s = %.4f\n", wq_parameter_to_string((WQParameter)max_i), wq_parameter_to_string((WQParameter)max_j), max_corr);
    fprintf(fp, "最强负相关: %s-%s = %.4f\n", wq_parameter_to_string((WQParameter)min_i), wq_parameter_to_string((WQParameter)min_j), min_corr);
    fprintf(fp, "\n[重点相关]\nTemp-DO=%.4f\npH-DO=%.4f\nTemp-Air_temp=%.4f\nTemp-Salinity=%.4f\n",
            statistics->correlation_matrix[WQ_PARAM_TEMP][WQ_PARAM_DO],
            statistics->correlation_matrix[WQ_PARAM_PH][WQ_PARAM_DO],
            statistics->correlation_matrix[WQ_PARAM_TEMP][WQ_PARAM_AIR_TEMP],
            statistics->correlation_matrix[WQ_PARAM_TEMP][WQ_PARAM_SALINITY]);

    fprintf(fp, "\n[讨论]\n|r|越接近1线性相关越强，接近0则线性相关弱；相关不代表因果。\n");
    return (fclose(fp) == 0) ? WQ_SUCCESS : WQ_ERROR;
}

int wq_write_warning_report(const char *filename,
                            const WarningRecord *warnings,
                            size_t warning_count)
{
    FILE *fp;
    size_t i;
    if (filename == NULL) return WQ_ERROR;
    fp = fopen(filename, "w");
    if (fp == NULL) return WQ_ERROR;
    fprintf(fp, "=== 预警报告 ===\n");
    if (warnings == NULL || warning_count == 0U) {
        fprintf(fp, "未检测到预警。\n");
        return (fclose(fp) == 0) ? WQ_SUCCESS : WQ_ERROR;
    }
    for (i = 0U; i < warning_count; ++i) {
        fprintf(fp, "\n[%lu] 时间: ", (unsigned long)(i + 1U));
        wq_print_time(fp, &warnings[i].time);
        fprintf(fp, "\n类型: %s\n说明: %s\n处理建议: %s\n", wq_warning_type_to_string(warnings[i].type), warnings[i].message, warnings[i].suggestion);
    }
    return (fclose(fp) == 0) ? WQ_SUCCESS : WQ_ERROR;
}

int wq_write_prediction_report(const char *filename,
                               const LinearRegressionModel *models,
                               size_t model_count)
{
    FILE *fp;
    size_t i;
    size_t best_idx = 0U;
    if (filename == NULL || models == NULL || model_count == 0U) return WQ_ERROR;
    fp = fopen(filename, "w");
    if (fp == NULL) return WQ_ERROR;

    fprintf(fp, "=== 预测分析报告 ===\n\n");
    for (i = 0U; i < model_count; ++i) {
        fprintf(fp, "%s -> %s: y = %.6f*x + %.6f, R2=%.6f, RMSE=%.6f\n",
                wq_parameter_to_string(models[i].x_param), wq_parameter_to_string(models[i].y_param),
                models[i].slope, models[i].intercept, models[i].r_squared, models[i].rmse);
        if (models[i].r_squared > models[best_idx].r_squared) best_idx = i;
    }

    fprintf(fp, "\nR2最高单因子: %s -> %s (R2=%.6f)\n",
            wq_parameter_to_string(models[best_idx].x_param),
            wq_parameter_to_string(models[best_idx].y_param),
            models[best_idx].r_squared);

    fprintf(fp, "\n[局限性分析]\n单因素线性回归忽略多变量耦合与非线性关系，外推能力有限，受噪声与时变工况影响较大。\n");
    fprintf(fp, "窗口增大通常增强去噪但会平滑突变细节。\n");
    return (fclose(fp) == 0) ? WQ_SUCCESS : WQ_ERROR;
}

int wq_write_discussion_report(const char *filename,
                               const AnalysisDiscussion *discussions,
                               size_t discussion_count,
                               bool append)
{
    FILE *fp;
    size_t i;
    if (filename == NULL || discussions == NULL) return WQ_ERROR;
    fp = fopen(filename, append ? "a" : "w");
    if (fp == NULL) return WQ_ERROR;
    fprintf(fp, "\n=== 分析讨论 ===\n");
    for (i = 0U; i < discussion_count; ++i) {
        fprintf(fp, "\n[%s] %s\n%s\n", wq_discussion_topic_to_string(discussions[i].topic), discussions[i].title, discussions[i].content);
    }
    return (fclose(fp) == 0) ? WQ_SUCCESS : WQ_ERROR;
}

const char *wq_discussion_topic_to_string(WQDiscussionTopic topic)
{
    switch (topic) {
    case WQ_DISCUSSION_STORAGE_FORMAT:
        return "存储格式分析讨论";
    case WQ_DISCUSSION_OUTLIER_PROCESSING:
        return "异常值处理分析讨论";
    case WQ_DISCUSSION_FILTER_WINDOW:
        return "滤波窗口分析讨论";
    case WQ_DISCUSSION_CORRELATION:
        return "相关性分析讨论";
    case WQ_DISCUSSION_REGRESSION_ACCURACY:
        return "线性回归预测分析讨论";
    case WQ_DISCUSSION_CUSTOM:
        return "补充分析讨论";
    default:
        return "未知分析讨论";
    }
}

int wq_view_text_report(const char *filename)
{
    FILE *fp;
    char line[512];
    if (filename == NULL) return WQ_ERROR;
    fp = fopen(filename, "r");
    if (fp == NULL) return WQ_ERROR;
    while (fgets(line, sizeof(line), fp) != NULL) {
        fputs(line, stdout);
    }
    return (fclose(fp) == 0) ? WQ_SUCCESS : WQ_ERROR;
}
