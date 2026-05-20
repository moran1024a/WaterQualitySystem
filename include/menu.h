#ifndef WQ_MENU_H
#define WQ_MENU_H

#include "types.h"

/*
 * menu.h
 *
 * 菜单交互接口。
 * 菜单只负责显示选项、接收选择、鉴权和调用对应模块，不直接实现数据分析算法。
 */

void wq_show_main_menu(WQUserRole role);
void wq_show_data_menu(WQUserRole role);
void wq_show_preprocess_menu(WQUserRole role);
void wq_show_statistics_menu(WQUserRole role);
void wq_show_prediction_menu(WQUserRole role);
void wq_show_backup_menu(WQUserRole role);
void wq_show_user_menu(WQUserRole role);

/* 根据主菜单选项分发功能调用；每个分支内部必须先调用 auth 模块鉴权。 */
void wq_handle_main_menu(SystemContext *context, int option);

/* 子菜单选项到具体操作的映射。 */
WQOperation wq_data_menu_option_to_operation(int option);
WQOperation wq_preprocess_menu_option_to_operation(int option);
WQOperation wq_statistics_menu_option_to_operation(int option);
WQOperation wq_prediction_menu_option_to_operation(int option);
WQOperation wq_backup_menu_option_to_operation(int option);
WQOperation wq_user_menu_option_to_operation(int option);

/* 程序启动时自动加载账户文件与默认数据文件。 */
int wq_initialize_system_context(SystemContext *context);

/* 程序退出时保存账户文件、释放数据和用户表。 */
void wq_destroy_system_context(SystemContext *context);

/* 尝试自动加载数据：优先读取 clean_data.bin，其次读取 data.csv。 */
int wq_auto_load_dataset(SystemContext *context);

/* 系统入口流程：初始化、登录、菜单循环、退出释放资源。 */
void wq_run_system(void);

#endif
