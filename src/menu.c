#include "menu.h"
#include "auth.h"
#include "file_io.h"
#include <stdio.h>
#include <string.h>

/*
 * menu.c
 *
 * 系统菜单模块函数框架。
 * 约定：所有操作都通过 WQOperation 进行鉴权，不只依赖“菜单是否显示”。
 */

void wq_show_main_menu(WQUserRole role)
{
    /* 待实现：根据 role 显示允许访问的主菜单项。管理员额外显示用户管理菜单。 */
    (void)role;
}

void wq_show_data_menu(WQUserRole role)
{
    /* 待实现：数据读取、保存、浏览、筛选、排序、修改、删除等子菜单。 */
    (void)role;
}

void wq_show_preprocess_menu(WQUserRole role)
{
    /* 待实现：异常值处理、缺失值填充、移动平均滤波等子菜单。 */
    (void)role;
}

void wq_show_statistics_menu(WQUserRole role)
{
    /* 待实现：基本统计、分段预警、相关性分析等子菜单。 */
    (void)role;
}

void wq_show_prediction_menu(WQUserRole role)
{
    /* 待实现：线性回归建模、R²、RMSE、多模型比较等子菜单。 */
    (void)role;
}

void wq_show_backup_menu(WQUserRole role)
{
    /* 待实现：备份和恢复子菜单。普通用户只允许备份，管理员允许恢复。 */
    (void)role;
}

void wq_show_user_menu(WQUserRole role)
{
    /* 待实现：管理员用户管理菜单，包含创建用户和导出用户列表。 */
    (void)role;
}

void wq_handle_main_menu(SystemContext *context, int option)
{
    WQOperation operation;

    if (context == NULL) {
        return;
    }

    operation = wq_main_menu_option_to_operation(option);
    if (wq_require_permission(&context->current_user, operation) != WQ_SUCCESS) {
        /* 待实现：提示当前角色无权限执行该操作。 */
        return;
    }

    /*
     * 待实现：
     * 1. 若操作依赖数据集，先检查 context->data_loaded。
     * 2. 根据 option 进入对应子菜单。
     * 3. 子菜单内部仍需再次按具体操作调用 wq_require_permission()。
     */
    (void)operation;
}

WQOperation wq_data_menu_option_to_operation(int option)
{
    switch (option) {
    case 1:
        return WQ_OP_LOAD_DATA;
    case 2:
        return WQ_OP_SAVE_DATA;
    case 3:
        return WQ_OP_BROWSE_DATA;
    case 4:
        return WQ_OP_FILTER_DATA;
    case 5:
        return WQ_OP_SORT_DATA;
    case 6:
        return WQ_OP_MODIFY_DATA;
    case 7:
        return WQ_OP_DELETE_DATA;
    default:
        return WQ_OP_NONE;
    }
}

WQOperation wq_preprocess_menu_option_to_operation(int option)
{
    (void)option;
    return WQ_OP_PREPROCESS_DATA;
}

WQOperation wq_statistics_menu_option_to_operation(int option)
{
    switch (option) {
    case 1:
        return WQ_OP_STATISTICS_ANALYSIS;
    case 2:
        return WQ_OP_WARNING_ANALYSIS;
    default:
        return WQ_OP_NONE;
    }
}

WQOperation wq_prediction_menu_option_to_operation(int option)
{
    (void)option;
    return WQ_OP_PREDICTION_ANALYSIS;
}

WQOperation wq_backup_menu_option_to_operation(int option)
{
    switch (option) {
    case 1:
        return WQ_OP_BACKUP_DATA;
    case 2:
        return WQ_OP_RESTORE_DATA;
    default:
        return WQ_OP_NONE;
    }
}

WQOperation wq_user_menu_option_to_operation(int option)
{
    switch (option) {
    case 1:
        return WQ_OP_CREATE_USER;
    case 2:
        return WQ_OP_EXPORT_USERS;
    default:
        return WQ_OP_NONE;
    }
}

int wq_initialize_system_context(SystemContext *context)
{
    if (context == NULL) {
        return WQ_ERROR;
    }

    memset(context, 0, sizeof(*context));
    context->dataset = NULL;
    context->logged_in = false;
    context->data_loaded = false;

    if (wq_auth_startup(&context->user_store) != WQ_SUCCESS) {
        return WQ_ERROR;
    }

    /* 启动时自动尝试加载数据。加载失败不影响进入登录流程。 */
    (void)wq_auto_load_dataset(context);

    return WQ_SUCCESS;
}

void wq_destroy_system_context(SystemContext *context)
{
    if (context == NULL) {
        return;
    }

    wq_dataset_destroy(context->dataset);
    context->dataset = NULL;
    context->data_loaded = false;

    (void)wq_auth_shutdown(&context->user_store);

    wq_secure_clear(&context->current_user, sizeof(context->current_user));
    context->logged_in = false;
}

int wq_auto_load_dataset(SystemContext *context)
{
    WaterQualityDataset *dataset;

    if (context == NULL) {
        return WQ_ERROR;
    }

    dataset = wq_read_binary(WQ_CLEAN_BIN_FILE);
    if (dataset == NULL) {
        dataset = wq_read_csv(WQ_DEFAULT_DATA_FILE, &context->overview);
    }

    if (dataset == NULL) {
        context->data_loaded = false;
        return WQ_ERROR;
    }

    wq_dataset_destroy(context->dataset);
    context->dataset = dataset;
    context->data_loaded = true;
    return WQ_SUCCESS;
}

void wq_run_system(void)
{
    SystemContext context;

    if (wq_initialize_system_context(&context) != WQ_SUCCESS) {
        return;
    }

    if (wq_login(&context.user_store, &context.current_user) == WQ_SUCCESS) {
        context.logged_in = true;
        /* 待实现：进入菜单循环，循环中所有操作继续调用 wq_require_permission()。 */
    }

    wq_destroy_system_context(&context);
}
