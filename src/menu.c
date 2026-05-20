#include "menu.h"
#include "auth.h"
#include "file_io.h"
#include "preprocess.h"
#include "statistics.h"
#include "prediction.h"
#include "report.h"
#include "backup.h"
#include "data_manage.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

static int req(const UserAccount *user, WQOperation operation)
{
    if (wq_require_permission(user, operation) != WQ_SUCCESS) {
        printf("无权限: %s\n", wq_operation_to_string(operation));
        return WQ_ERROR;
    }
    return WQ_SUCCESS;
}

static int ensure_data(SystemContext *context)
{
    if (context == NULL || !context->data_loaded || context->dataset == NULL) {
        printf("未加载数据。\n");
        return WQ_ERROR;
    }
    return WQ_SUCCESS;
}

static bool main_option_allowed(WQUserRole role, int option)
{
    if (option == 0) return true;
    if (role == WQ_ROLE_ADMIN) return option >= 1 && option <= 10;
    if (role == WQ_ROLE_USER) return option >= 1 && option <= 9;
    return option == 5 || option == 7;
}

static bool read_confirm(const char *prompt)
{
    int v = 0;
    if (wq_read_int(prompt, &v) != WQ_SUCCESS) return false;
    return v == 1;
}

static int read_parameter(WQParameter *parameter_out)
{
    int p;
    if (parameter_out == NULL) return WQ_ERROR;
    if (wq_read_int("参数(0 Temp,1 Salinity,2 pH,3 DO,4 precipitation,5 Air_temp): ", &p) != WQ_SUCCESS) return WQ_ERROR;
    if (p < 0 || p >= (int)WQ_PARAM_COUNT) {
        puts("参数编号无效。");
        return WQ_ERROR;
    }
    *parameter_out = (WQParameter)p;
    return WQ_SUCCESS;
}

static int create_auto_backup(SystemContext *context, const char *reason)
{
    char path[WQ_MAX_PATH_LENGTH];
    if (context == NULL || context->dataset == NULL) return WQ_ERROR;
    if (wq_create_backup(context->dataset, WQ_BACKUP_DIR, path, sizeof(path)) != WQ_SUCCESS) {
        puts("自动备份失败，操作已取消。");
        return WQ_ERROR;
    }
    printf("%s前已自动备份: %s\n", reason != NULL ? reason : "操作", path);
    return WQ_SUCCESS;
}

static int save_clean_dataset(SystemContext *context)
{
    if (context == NULL || context->dataset == NULL) return WQ_ERROR;
    if (wq_write_csv(WQ_CLEAN_CSV_FILE, context->dataset) != WQ_SUCCESS) return WQ_ERROR;
    if (wq_write_binary(WQ_CLEAN_BIN_FILE, context->dataset) != WQ_SUCCESS) return WQ_ERROR;
    if (wq_compare_storage_formats(context->dataset,
                                   WQ_CLEAN_CSV_FILE,
                                   WQ_CLEAN_BIN_FILE,
                                   &context->overview.csv_storage,
                                   &context->overview.binary_storage) == WQ_SUCCESS) {
        context->overview.storage_benchmark_valid = true;
    }
    context->overview.valid_records = context->dataset->size;
    context->overview.invalid_records = (context->overview.total_records >= context->overview.valid_records)
                                ? (context->overview.total_records - context->overview.valid_records)
                                : 0U;
    return WQ_SUCCESS;
}

static size_t count_records_by_range(const WaterQualityDataset *dataset,
                                     WQParameter parameter,
                                     double min_value,
                                     double max_value)
{
    size_t i;
    size_t count = 0U;
    if (dataset == NULL || min_value > max_value) return 0U;
    for (i = 0U; i < dataset->size; ++i) {
        const WaterQualityRecord *r = &dataset->records[i];
        if (!r->missing[parameter] && r->value[parameter] >= min_value && r->value[parameter] <= max_value) {
            count++;
        }
    }
    return count;
}

void wq_show_main_menu(WQUserRole role)
{
    printf("\n========================================\n");
    printf("     %s v%s (%s)\n", WQ_SYSTEM_NAME, WQ_VERSION, wq_role_to_string(role));
    printf("========================================\n");
    if (role == WQ_ROLE_ADMIN || role == WQ_ROLE_USER) {
        printf("   [1] 数据基础操作\n");
        printf("   [2] 数据预处理\n");
        printf("   [3] 统计分析\n");
        printf("   [4] 预测分析\n");
        printf("   [5] 查看数据概览\n");
        printf("   [6] 查看预警报告\n");
        printf("   [7] 查看分析报告\n");
        printf("   [8] 数据备份与恢复\n");
        printf("   [9] 清屏\n");
    } else {
        printf("   [5] 查看数据概览\n");
        printf("   [7] 查看统计分析报告\n");
    }
    if (role == WQ_ROLE_ADMIN) {
        printf("   [10] 用户与权限管理\n");
    }
    printf("   [0] 退出系统\n");
    printf("========================================\n");
}

void wq_show_data_menu(WQUserRole role)
{
    (void)role;
    printf("\n[数据基础]\n1分页浏览 2范围筛选 3参数排序 4单条修改 5单条删除 6按范围批量删除 7保存清洗数据 0返回\n");
}

void wq_show_preprocess_menu(WQUserRole role)
{
    (void)role;
    printf("\n[预处理]\n1执行完整预处理 2保存清洗CSV/BIN并对比存储 3查看概览报告 0返回\n");
}

void wq_show_statistics_menu(WQUserRole role)
{
    (void)role;
    printf("\n[统计]\n1计算基本统计和相关矩阵 2生成预警报告 3生成统计报告 4查看统计报告 0返回\n");
}

void wq_show_prediction_menu(WQUserRole role)
{
    (void)role;
    printf("\n[预测]\n1训练Air_temp->DO 2留出法RMSE 3多因子比较 4生成预测报告 0返回\n");
}

void wq_show_backup_menu(WQUserRole role)
{
    (void)role;
    printf("\n[备份]\n1手动备份 2列出备份 3恢复备份 0返回\n");
}

void wq_show_user_menu(WQUserRole role)
{
    (void)role;
    printf("\n[用户]\n1创建用户 2导出用户CSV 3查看当前用户 0返回\n");
}

static void browse_pages(const WaterQualityDataset *dataset)
{
    size_t page = 0U;
    size_t total;
    int op;
    if (dataset == NULL || dataset->size == 0U) {
        puts("数据为空。");
        return;
    }
    total = wq_get_total_pages(dataset, WQ_PAGE_SIZE);
    for (;;) {
        printf("\n第%lu/%lu页\n", (unsigned long)(page + 1U), (unsigned long)total);
        wq_print_page(dataset, page, WQ_PAGE_SIZE);
        printf("1下一页 2上一页 3跳转页 0返回\n");
        if (wq_read_int("选择: ", &op) != WQ_SUCCESS) return;
        if (op == 0) return;
        if (op == 1 && page + 1U < total) page++;
        else if (op == 2 && page > 0U) page--;
        else if (op == 3) {
            int target;
            if (wq_read_int("页码(从1开始): ", &target) == WQ_SUCCESS && target >= 1 && (size_t)target <= total) {
                page = (size_t)target - 1U;
            } else {
                puts("页码无效。");
            }
        } else {
            puts("无法翻页或输入无效。");
        }
    }
}

static void handle_data(SystemContext *context)
{
    int op;
    for (;;) {
        wq_show_data_menu(context->current_user.role);
        if (wq_read_int("选择: ", &op) != WQ_SUCCESS) return;
        if (op == 0) return;
        if (ensure_data(context) != WQ_SUCCESS) continue;

        if (op == 1) {
            if (req(&context->current_user, WQ_OP_BROWSE_DATA) != WQ_SUCCESS) continue;
            browse_pages(context->dataset);
        } else if (op == 2) {
            WQParameter parameter;
            double min_value;
            double max_value;
            WaterQualityDataset *tmp;
            if (req(&context->current_user, WQ_OP_FILTER_DATA) != WQ_SUCCESS) continue;
            if (read_parameter(&parameter) != WQ_SUCCESS) continue;
            if (wq_read_double("最小值: ", &min_value) != WQ_SUCCESS ||
                wq_read_double("最大值: ", &max_value) != WQ_SUCCESS) continue;
            tmp = wq_dataset_create(32U);
            if (tmp == NULL) { puts("内存不足。"); continue; }
            if (wq_filter_by_range(context->dataset, tmp, parameter, min_value, max_value) == WQ_SUCCESS) {
                printf("筛选结果: %lu条\n", (unsigned long)tmp->size);
                browse_pages(tmp);
            } else {
                puts("筛选失败。");
            }
            wq_dataset_destroy(tmp);
        } else if (op == 3) {
            WQParameter parameter;
            int order;
            if (req(&context->current_user, WQ_OP_SORT_DATA) != WQ_SUCCESS) continue;
            if (read_parameter(&parameter) != WQ_SUCCESS) continue;
            if (wq_read_int("排序(0升序 1降序): ", &order) != WQ_SUCCESS) continue;
            if (wq_sort_dataset(context->dataset, parameter, order ? WQ_SORT_DESC : WQ_SORT_ASC) == WQ_SUCCESS) {
                puts("排序完成。");
            } else {
                puts("排序失败。");
            }
        } else if (op == 4) {
            int idx;
            WQParameter parameter;
            double value;
            if (req(&context->current_user, WQ_OP_MODIFY_DATA) != WQ_SUCCESS) continue;
            if (wq_read_int("记录下标: ", &idx) != WQ_SUCCESS || idx < 0) continue;
            if (read_parameter(&parameter) != WQ_SUCCESS) continue;
            if (wq_read_double("新值: ", &value) != WQ_SUCCESS) continue;
            if (create_auto_backup(context, "修改") != WQ_SUCCESS) continue;
            if (wq_modify_record_value(context->dataset, (size_t)idx, parameter, value) == WQ_SUCCESS) {
                puts("修改成功。");
                if (read_confirm("是否保存到文件? 1是 0否: ")) {
                    puts(save_clean_dataset(context) == WQ_SUCCESS ? "保存成功。" : "保存失败。");
                }
            } else {
                puts("修改失败：下标无效或新值超出合理范围。");
            }
        } else if (op == 5) {
            int idx;
            if (req(&context->current_user, WQ_OP_DELETE_DATA) != WQ_SUCCESS) continue;
            if (wq_read_int("记录下标: ", &idx) != WQ_SUCCESS || idx < 0 || (size_t)idx >= context->dataset->size) {
                puts("记录下标无效。");
                continue;
            }
            wq_print_record(&context->dataset->records[idx]);
            if (!read_confirm("确认删除该记录? 1是 0否: ")) continue;
            if (create_auto_backup(context, "删除") != WQ_SUCCESS) continue;
            puts(wq_delete_record(context->dataset, (size_t)idx) == WQ_SUCCESS ? "删除成功。" : "删除失败。");
        } else if (op == 6) {
            WQParameter parameter;
            double min_value;
            double max_value;
            size_t count;
            if (req(&context->current_user, WQ_OP_DELETE_DATA) != WQ_SUCCESS) continue;
            if (read_parameter(&parameter) != WQ_SUCCESS) continue;
            if (wq_read_double("最小值: ", &min_value) != WQ_SUCCESS ||
                wq_read_double("最大值: ", &max_value) != WQ_SUCCESS) continue;
            count = count_records_by_range(context->dataset, parameter, min_value, max_value);
            printf("将删除 %lu 条记录。\n", (unsigned long)count);
            if (count == 0U) continue;
            if (!read_confirm("确认批量删除? 1是 0否: ")) continue;
            if (create_auto_backup(context, "批量删除") != WQ_SUCCESS) continue;
            printf("实际删除 %lu 条记录。\n", (unsigned long)wq_delete_records_by_range(context->dataset, parameter, min_value, max_value));
        } else if (op == 7) {
            if (req(&context->current_user, WQ_OP_SAVE_DATA) != WQ_SUCCESS) continue;
            puts(save_clean_dataset(context) == WQ_SUCCESS ? "保存成功。" : "保存失败。");
        } else {
            puts("输入无效。");
        }
    }
}

static void handle_preprocess(SystemContext *context)
{
    int op;
    for (;;) {
        wq_show_preprocess_menu(context->current_user.role);
        if (wq_read_int("选择: ", &op) != WQ_SUCCESS) return;
        if (op == 0) return;
        if (req(&context->current_user, WQ_OP_PREPROCESS_DATA) != WQ_SUCCESS) continue;
        if (ensure_data(context) != WQ_SUCCESS) continue;
        if (op == 1) {
            if (wq_preprocess_dataset(context->dataset, &context->overview) == WQ_SUCCESS) {
                (void)wq_write_overview_report(WQ_OVERVIEW_FILE, &context->overview);
                puts("预处理完成，清洗数据和概览报告已更新。");
            } else {
                puts("预处理失败。");
            }
        } else if (op == 2) {
            puts(save_clean_dataset(context) == WQ_SUCCESS ? "保存与存储对比完成。" : "保存失败。");
        } else if (op == 3) {
            (void)wq_write_overview_report(WQ_OVERVIEW_FILE, &context->overview);
            if (wq_view_text_report(WQ_OVERVIEW_FILE) != WQ_SUCCESS) puts("报告不存在或无法读取。");
        } else {
            puts("输入无效。");
        }
    }
}

static void handle_statistics(SystemContext *context)
{
    int op;
    WarningRecord warnings[512];
    size_t warning_count = 0U;
    for (;;) {
        wq_show_statistics_menu(context->current_user.role);
        if (wq_read_int("选择: ", &op) != WQ_SUCCESS) return;
        if (op == 0) return;
        if (req(&context->current_user, WQ_OP_STATISTICS_ANALYSIS) != WQ_SUCCESS) continue;
        if (ensure_data(context) != WQ_SUCCESS) continue;
        if (op == 1) {
            puts(wq_compute_all_statistics(context->dataset, &context->statistics) == WQ_SUCCESS ? "统计计算完成。" : "统计计算失败。");
        } else if (op == 2) {
            warning_count = 0U;
            if (wq_analyze_segment_warnings(context->dataset, warnings, 512U, &warning_count) == WQ_SUCCESS &&
                wq_write_warning_report(WQ_WARNING_REPORT_FILE, warnings, warning_count) == WQ_SUCCESS) {
                printf("预警报告已生成，共%lu条预警。\n", (unsigned long)warning_count);
            } else {
                puts("预警分析失败。");
            }
        } else if (op == 3) {
            if (wq_compute_all_statistics(context->dataset, &context->statistics) == WQ_SUCCESS &&
                wq_write_statistics_report(WQ_STATISTICS_REPORT_FILE, &context->statistics, context->dataset) == WQ_SUCCESS) {
                puts("统计报告已生成。");
            } else {
                puts("统计报告生成失败。");
            }
        } else if (op == 4) {
            if (wq_view_text_report(WQ_STATISTICS_REPORT_FILE) != WQ_SUCCESS) puts("报告不存在或无法读取。");
        } else {
            puts("输入无效。");
        }
    }
}

static void handle_prediction(SystemContext *context)
{
    int op;
    LinearRegressionModel models[8];
    size_t model_count = 0U;
    for (;;) {
        wq_show_prediction_menu(context->current_user.role);
        if (wq_read_int("选择: ", &op) != WQ_SUCCESS) return;
        if (op == 0) return;
        if (req(&context->current_user, WQ_OP_PREDICTION_ANALYSIS) != WQ_SUCCESS) continue;
        if (ensure_data(context) != WQ_SUCCESS) continue;
        if (op == 1) {
            LinearRegressionModel model;
            if (wq_train_linear_regression(context->dataset, WQ_PARAM_AIR_TEMP, WQ_PARAM_DO, &model) == WQ_SUCCESS) {
                model.r_squared = wq_evaluate_r_squared(context->dataset, &model);
                model.rmse = wq_evaluate_holdout_rmse(context->dataset, WQ_PARAM_AIR_TEMP, WQ_PARAM_DO, 0.8, &model);
                models[0] = model;
                model_count = 1U;
                printf("模型: DO = %.6f * Air_temp + %.6f, R2=%.6f, RMSE=%.6f\n",
                       model.slope, model.intercept, model.r_squared, model.rmse);
            } else {
                puts("训练失败。");
            }
        } else if (op == 2) {
            LinearRegressionModel model;
            double rmse = wq_evaluate_holdout_rmse(context->dataset, WQ_PARAM_AIR_TEMP, WQ_PARAM_DO, 0.8, &model);
            if (rmse >= 0.0) {
                printf("Air_temp->DO 留出法RMSE=%.6f\n", rmse);
                models[0] = model;
                model_count = 1U;
            } else {
                puts("评估失败。");
            }
        } else if (op == 3) {
            if (wq_compare_do_single_factor_models(context->dataset, models, 8U, &model_count) == WQ_SUCCESS) {
                size_t i;
                for (i = 0U; i < model_count; ++i) {
                    printf("%s->DO R2=%.6f RMSE=%.6f\n",
                           wq_parameter_to_string(models[i].x_param), models[i].r_squared, models[i].rmse);
                }
            } else {
                puts("多因子比较失败。");
            }
        } else if (op == 4) {
            if (model_count == 0U) (void)wq_compare_do_single_factor_models(context->dataset, models, 8U, &model_count);
            puts(wq_write_prediction_report(WQ_PREDICTION_REPORT_FILE, models, model_count) == WQ_SUCCESS ? "预测报告已生成。" : "预测报告生成失败。");
        } else {
            puts("输入无效。");
        }
    }
}

static void handle_backup(SystemContext *context)
{
    int op;
    char path[WQ_MAX_PATH_LENGTH];
    char files[WQ_MAX_BACKUP_FILES][WQ_MAX_PATH_LENGTH];
    size_t count = 0U;
    size_t i;
    for (;;) {
        wq_show_backup_menu(context->current_user.role);
        if (wq_read_int("选择: ", &op) != WQ_SUCCESS) return;
        if (op == 0) return;
        if (ensure_data(context) != WQ_SUCCESS) continue;
        if (op == 1) {
            if (req(&context->current_user, WQ_OP_BACKUP_DATA) != WQ_SUCCESS) continue;
            if (wq_create_backup(context->dataset, WQ_BACKUP_DIR, path, sizeof(path)) == WQ_SUCCESS) {
                printf("备份成功，文件已保存至: %s\n", path);
            } else {
                puts("备份失败。");
            }
        } else if (op == 2) {
            if (wq_list_backup_files(WQ_BACKUP_DIR, files, WQ_MAX_BACKUP_FILES, &count) == WQ_SUCCESS) {
                if (count == 0U) puts("无可用备份。");
                for (i = 0U; i < count; ++i) printf("%lu. %s\n", (unsigned long)(i + 1U), files[i]);
            } else {
                puts("读取备份列表失败。");
            }
        } else if (op == 3) {
            int idx;
            WaterQualityDataset *restored;
            if (req(&context->current_user, WQ_OP_RESTORE_DATA) != WQ_SUCCESS) continue;
            if (wq_list_backup_files(WQ_BACKUP_DIR, files, WQ_MAX_BACKUP_FILES, &count) != WQ_SUCCESS || count == 0U) {
                puts("无可用备份。");
                continue;
            }
            for (i = 0U; i < count; ++i) printf("%lu. %s\n", (unsigned long)(i + 1U), files[i]);
            if (wq_read_int("选择恢复编号: ", &idx) != WQ_SUCCESS || idx < 1 || (size_t)idx > count) {
                puts("编号无效。");
                continue;
            }
            if (!read_confirm("恢复会替换当前内存数据，确认恢复? 1是 0否: ")) continue;
            if (create_auto_backup(context, "恢复") != WQ_SUCCESS) continue;
            if (snprintf(path, sizeof(path), "%s/%s", WQ_BACKUP_DIR, files[idx - 1]) >= (int)sizeof(path)) {
                puts("备份路径过长。");
                continue;
            }
            restored = wq_restore_backup(path, &context->overview);
            if (restored != NULL) {
                wq_dataset_destroy(context->dataset);
                context->dataset = restored;
                context->data_loaded = true;
                puts("恢复成功。");
            } else {
                puts("恢复失败，当前数据保持不变。");
            }
        } else {
            puts("输入无效。");
        }
    }
}

static void handle_user(SystemContext *context)
{
    int op;
    for (;;) {
        wq_show_user_menu(context->current_user.role);
        if (wq_read_int("选择: ", &op) != WQ_SUCCESS) return;
        if (op == 0) return;
        if (op == 1) {
            char username[64];
            char password[64];
            int role;
            if (req(&context->current_user, WQ_OP_CREATE_USER) != WQ_SUCCESS) continue;
            printf("用户名: ");
            if (fgets(username, sizeof(username), stdin) == NULL) continue;
            printf("密码: ");
            if (fgets(password, sizeof(password), stdin) == NULL) continue;
            wq_trim(username);
            wq_trim(password);
            if (wq_read_int("角色(0管理员 1普通用户 2访客): ", &role) != WQ_SUCCESS) continue;
            puts(wq_create_user(&context->user_store, username, password, (WQUserRole)role) == WQ_SUCCESS ? "创建成功。" : "创建失败。");
            wq_secure_clear(password, sizeof(password));
        } else if (op == 2) {
            if (req(&context->current_user, WQ_OP_EXPORT_USERS) != WQ_SUCCESS) continue;
            puts(wq_export_users_to_csv(&context->user_store, WQ_USER_EXPORT_CSV_FILE, true) == WQ_SUCCESS ? "导出成功。" : "导出失败。");
        } else if (op == 3) {
            printf("当前用户: %s (%s)\n", context->current_user.username, wq_role_to_string(context->current_user.role));
        } else {
            puts("输入无效。");
        }
    }
}

void wq_handle_main_menu(SystemContext *context, int option)
{
    WQOperation operation;
    if (context == NULL) return;
    operation = wq_main_menu_option_to_operation(option);
    if (req(&context->current_user, operation) != WQ_SUCCESS) return;

    switch (option) {
    case 1:
        handle_data(context);
        break;
    case 2:
        handle_preprocess(context);
        break;
    case 3:
        handle_statistics(context);
        break;
    case 4:
        handle_prediction(context);
        break;
    case 5:
        (void)wq_write_overview_report(WQ_OVERVIEW_FILE, &context->overview);
        if (wq_view_text_report(WQ_OVERVIEW_FILE) != WQ_SUCCESS) puts("报告不存在或无法读取。");
        break;
    case 6:
        if (wq_view_text_report(WQ_WARNING_REPORT_FILE) != WQ_SUCCESS) puts("报告不存在或无法读取。");
        break;
    case 7:
        if (wq_view_text_report(WQ_STATISTICS_REPORT_FILE) != WQ_SUCCESS) puts("统计报告不存在或无法读取。");
        if (context->current_user.role != WQ_ROLE_GUEST) {
            if (wq_view_text_report(WQ_PREDICTION_REPORT_FILE) != WQ_SUCCESS) puts("预测报告不存在或无法读取。");
        }
        break;
    case 8:
        handle_backup(context);
        break;
    case 9:
        wq_clear_screen();
        break;
    case 10:
        handle_user(context);
        break;
    default:
        puts("输入无效。");
        break;
    }
}

WQOperation wq_data_menu_option_to_operation(int option)
{
    switch (option) {
    case 1: return WQ_OP_BROWSE_DATA;
    case 2: return WQ_OP_FILTER_DATA;
    case 3: return WQ_OP_SORT_DATA;
    case 4: return WQ_OP_MODIFY_DATA;
    case 5:
    case 6: return WQ_OP_DELETE_DATA;
    case 7: return WQ_OP_SAVE_DATA;
    case 0: return WQ_OP_NONE;
    default: return WQ_OP_NONE;
    }
}

WQOperation wq_preprocess_menu_option_to_operation(int option)
{
    return option == 0 ? WQ_OP_NONE : WQ_OP_PREPROCESS_DATA;
}

WQOperation wq_statistics_menu_option_to_operation(int option)
{
    return option == 0 ? WQ_OP_NONE : WQ_OP_STATISTICS_ANALYSIS;
}

WQOperation wq_prediction_menu_option_to_operation(int option)
{
    return option == 0 ? WQ_OP_NONE : WQ_OP_PREDICTION_ANALYSIS;
}

WQOperation wq_backup_menu_option_to_operation(int option)
{
    switch (option) {
    case 1: return WQ_OP_BACKUP_DATA;
    case 2: return WQ_OP_BACKUP_MENU;
    case 3: return WQ_OP_RESTORE_DATA;
    default: return WQ_OP_NONE;
    }
}

WQOperation wq_user_menu_option_to_operation(int option)
{
    switch (option) {
    case 1: return WQ_OP_CREATE_USER;
    case 2: return WQ_OP_EXPORT_USERS;
    case 3: return WQ_OP_USER_MENU;
    default: return WQ_OP_NONE;
    }
}

int wq_initialize_system_context(SystemContext *context)
{
    if (context == NULL) return WQ_ERROR;
    memset(context, 0, sizeof(*context));
    if (wq_ensure_directory("output") != WQ_SUCCESS ||
        wq_ensure_directory("output/reports") != WQ_SUCCESS ||
        wq_ensure_directory("output/clean") != WQ_SUCCESS ||
        wq_ensure_directory("data") != WQ_SUCCESS ||
        wq_ensure_directory(WQ_USER_DATA_DIR) != WQ_SUCCESS ||
        wq_ensure_directory(WQ_BACKUP_DIR) != WQ_SUCCESS) {
        return WQ_ERROR;
    }
    if (wq_auth_startup(&context->user_store) != WQ_SUCCESS) return WQ_ERROR;
    (void)wq_auto_load_dataset(context);
    return WQ_SUCCESS;
}

void wq_destroy_system_context(SystemContext *context)
{
    if (context == NULL) return;
    wq_dataset_destroy(context->dataset);
    context->dataset = NULL;
    (void)wq_auth_shutdown(&context->user_store);
    wq_secure_clear(&context->current_user, sizeof(context->current_user));
}

int wq_auto_load_dataset(SystemContext *context)
{
    WaterQualityDataset *dataset;
    if (context == NULL) return WQ_ERROR;
    dataset = wq_read_binary(WQ_CLEAN_BIN_FILE);
    if (dataset == NULL) dataset = wq_read_csv(WQ_DEFAULT_DATA_FILE, &context->overview);
    if (dataset == NULL) {
        context->data_loaded = false;
        return WQ_ERROR;
    }
    if (context->overview.total_records == 0U) {
        memset(&context->overview, 0, sizeof(context->overview));
        context->overview.total_records = dataset->size;
        context->overview.valid_records = dataset->size;
    }
    wq_dataset_destroy(context->dataset);
    context->dataset = dataset;
    context->data_loaded = true;
    return WQ_SUCCESS;
}

void wq_run_system(void)
{
    SystemContext context;
    int option;
    int confirm;

    if (wq_initialize_system_context(&context) != WQ_SUCCESS) {
        puts("初始化失败。");
        return;
    }
    if (wq_login(&context.user_store, &context.current_user) != WQ_SUCCESS) {
        puts("登录失败，系统退出。");
        wq_destroy_system_context(&context);
        return;
    }
    context.logged_in = true;
    if (!context.data_loaded) puts("未自动加载数据，请检查数据文件。 ");

    for (;;) {
        wq_show_main_menu(context.current_user.role);
        if (wq_read_int("请选择操作: ", &option) != WQ_SUCCESS) break;
        if (!main_option_allowed(context.current_user.role, option)) {
            puts("权限不足或菜单项不可用。");
            continue;
        }
        if (option == 0) {
            if (wq_read_int("确认退出? 1是 0否: ", &confirm) == WQ_SUCCESS && confirm == 1) break;
            continue;
        }
        wq_handle_main_menu(&context, option);
    }
    wq_destroy_system_context(&context);
}
