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

static int req(const UserAccount *u, WQOperation op){ if (wq_require_permission(u, op)!=WQ_SUCCESS){ printf("无权限: %s\n", wq_operation_to_string(op)); return WQ_ERROR;} return WQ_SUCCESS; }
static int ensure_data(SystemContext *c){ if(!c->data_loaded||!c->dataset){ printf("未加载数据。\n"); return WQ_ERROR;} return WQ_SUCCESS; }

static bool main_option_allowed(WQUserRole role, int option)
{
    if (option == 0) return true;
    if (role == WQ_ROLE_ADMIN) return option >= 1 && option <= 9;
    if (role == WQ_ROLE_USER) return option >= 1 && option <= 8;
    return option == 5 || option == 6 || option == 7;
}


void wq_show_main_menu(WQUserRole role){
    printf("\n==== 主菜单(%s) ====\n", wq_role_to_string(role));
    if(role==WQ_ROLE_ADMIN||role==WQ_ROLE_USER){ printf("1.数据基础操作\n2.数据预处理\n3.统计分析\n4.预测分析\n5.查看数据概览\n6.查看预警报告\n7.查看分析报告\n8.数据备份与恢复\n"); }
    else { printf("5.查看数据概览\n6.查看预警报告\n7.查看分析报告\n"); }
    if(role==WQ_ROLE_ADMIN) printf("9.用户与权限管理\n");
    printf("0.退出系统\n");
}
void wq_show_data_menu(WQUserRole role){ (void)role; printf("\n[数据基础] 1浏览 2筛选 3排序 4修改 5删一条 6按范围删 7保存 0返回\n"); }
void wq_show_preprocess_menu(WQUserRole role){ (void)role; printf("\n[预处理] 1执行完整预处理 2保存清洗CSV/BIN 3查看概览报告 0返回\n"); }
void wq_show_statistics_menu(WQUserRole role){ (void)role; printf("\n[统计] 1计算统计 2生成预警 3生成统计报告 4查看统计报告 0返回\n"); }
void wq_show_prediction_menu(WQUserRole role){ (void)role; printf("\n[预测] 1训练Air_temp->DO 2留出RMSE 3多因子比较 4生成预测报告 0返回\n"); }
void wq_show_backup_menu(WQUserRole role){ (void)role; printf("\n[备份] 1手动备份 2列表 3恢复 0返回\n"); }
void wq_show_user_menu(WQUserRole role){ (void)role; printf("\n[用户] 1创建用户 2导出用户CSV 3查看当前用户 0返回\n"); }

static void handle_data(SystemContext *c){ int op; for(;;){ wq_show_data_menu(c->current_user.role); if(wq_read_int("选择: ",&op)!=WQ_SUCCESS) return; if(op==0) return; if(ensure_data(c)!=WQ_SUCCESS) continue; if(op==1){ size_t p=0; int pi; if(wq_read_int("页码(从1): ",&pi)==WQ_SUCCESS&&pi>0) p=(size_t)(pi-1); wq_print_page(c->dataset,p,WQ_PAGE_SIZE);} 
else if(op==2){ int par; double mn,mx; if(req(&c->current_user,WQ_OP_FILTER_DATA)!=WQ_SUCCESS) continue; if(wq_read_int("参数(0-5): ",&par)!=WQ_SUCCESS) continue; if(wq_read_double("最小值: ",&mn)!=WQ_SUCCESS||wq_read_double("最大值: ",&mx)!=WQ_SUCCESS) continue; { WaterQualityDataset *tmp=wq_dataset_create(32); if(!tmp) continue; if(wq_filter_by_range(c->dataset,tmp,(WQParameter)par,mn,mx)==WQ_SUCCESS) wq_print_page(tmp,0,WQ_PAGE_SIZE); wq_dataset_destroy(tmp);} }
else if(op==3){ int par,ord; if(req(&c->current_user,WQ_OP_SORT_DATA)!=WQ_SUCCESS) continue; if(wq_read_int("参数(0-5): ",&par)!=WQ_SUCCESS||wq_read_int("排序(0升1降): ",&ord)!=WQ_SUCCESS) continue; (void)wq_sort_dataset(c->dataset,(WQParameter)par,ord?WQ_SORT_DESC:WQ_SORT_ASC);} 
else if(op==4){ int idx,par; double v; if(req(&c->current_user,WQ_OP_MODIFY_DATA)!=WQ_SUCCESS) continue; if(wq_read_int("记录下标: ",&idx)!=WQ_SUCCESS||wq_read_int("参数(0-5): ",&par)!=WQ_SUCCESS||wq_read_double("新值: ",&v)!=WQ_SUCCESS) continue; (void)wq_modify_record_value(c->dataset,(size_t)idx,(WQParameter)par,v);} 
else if(op==5){ int idx; if(req(&c->current_user,WQ_OP_DELETE_DATA)!=WQ_SUCCESS) continue; if(wq_read_int("记录下标: ",&idx)!=WQ_SUCCESS) continue; (void)wq_delete_record(c->dataset,(size_t)idx);} 
else if(op==6){ int par; double mn,mx; if(req(&c->current_user,WQ_OP_DELETE_DATA)!=WQ_SUCCESS) continue; if(wq_read_int("参数(0-5): ",&par)!=WQ_SUCCESS||wq_read_double("最小值: ",&mn)!=WQ_SUCCESS||wq_read_double("最大值: ",&mx)!=WQ_SUCCESS) continue; printf("删除%lu条\n",(unsigned long)wq_delete_records_by_range(c->dataset,(WQParameter)par,mn,mx)); }
else if(op==7){ if(req(&c->current_user,WQ_OP_SAVE_DATA)!=WQ_SUCCESS) continue; if(wq_write_csv(WQ_CLEAN_CSV_FILE,c->dataset)==WQ_SUCCESS&&wq_write_binary(WQ_CLEAN_BIN_FILE,c->dataset)==WQ_SUCCESS) puts("保存成功"); else puts("保存失败"); }
 }
}

static void handle_pre(SystemContext *c){ int op; for(;;){ wq_show_preprocess_menu(c->current_user.role); if(wq_read_int("选择:",&op)!=WQ_SUCCESS) return; if(op==0) return; if(req(&c->current_user,WQ_OP_PREPROCESS_DATA)!=WQ_SUCCESS) continue; if(ensure_data(c)!=WQ_SUCCESS) continue; if(op==1){ if(wq_preprocess_dataset(c->dataset,&c->overview)==WQ_SUCCESS) puts("预处理完成"); else puts("预处理失败"); } else if(op==2){ (void)wq_write_csv(WQ_CLEAN_CSV_FILE,c->dataset); (void)wq_write_binary(WQ_CLEAN_BIN_FILE,c->dataset);} else if(op==3){ (void)wq_write_overview_report("output/reports/overview.txt",&c->overview); (void)wq_view_text_report("output/reports/overview.txt"); } }}

static void handle_stats(SystemContext *c){ int op; WarningRecord warns[512]; size_t wc=0; for(;;){ wq_show_statistics_menu(c->current_user.role); if(wq_read_int("选择:",&op)!=WQ_SUCCESS) return; if(op==0) return; if(ensure_data(c)!=WQ_SUCCESS) continue; if(op==1){ (void)wq_compute_all_statistics(c->dataset,&c->statistics);} else if(op==2){ wc=0; (void)wq_analyze_segment_warnings(c->dataset,warns,512,&wc); (void)wq_write_warning_report("output/reports/warning.txt",warns,wc);} else if(op==3){ (void)wq_compute_all_statistics(c->dataset,&c->statistics); (void)wq_write_statistics_report("output/reports/statistics.txt",&c->statistics,c->dataset);} else if(op==4){ (void)wq_view_text_report("output/reports/statistics.txt"); }} }

static void handle_pred(SystemContext *c){ int op; LinearRegressionModel ms[8]; size_t mc=0; for(;;){ wq_show_prediction_menu(c->current_user.role); if(wq_read_int("选择:",&op)!=WQ_SUCCESS) return; if(op==0) return; if(ensure_data(c)!=WQ_SUCCESS) continue; if(op==1){ LinearRegressionModel m; if(wq_train_linear_regression(c->dataset,WQ_PARAM_AIR_TEMP,WQ_PARAM_DO,&m)==WQ_SUCCESS){ m.r_squared=wq_evaluate_r_squared(c->dataset,&m); ms[0]=m; mc=1; }} else if(op==2){ LinearRegressionModel m; double rm=wq_evaluate_holdout_rmse(c->dataset,WQ_PARAM_AIR_TEMP,WQ_PARAM_DO,0.8,&m); printf("RMSE=%.6f\n",rm); ms[0]=m; mc=1;} else if(op==3){ (void)wq_compare_do_single_factor_models(c->dataset,ms,8,&mc);} else if(op==4){ if(mc==0) (void)wq_compare_do_single_factor_models(c->dataset,ms,8,&mc); (void)wq_write_prediction_report("output/reports/prediction.txt",ms,mc);} }}

static void handle_backup(SystemContext *c){ int op; char path[WQ_MAX_PATH_LENGTH]; char files[WQ_MAX_BACKUP_FILES][WQ_MAX_PATH_LENGTH]; size_t n=0,i; for(;;){ wq_show_backup_menu(c->current_user.role); if(wq_read_int("选择:",&op)!=WQ_SUCCESS) return; if(op==0) return; if(ensure_data(c)!=WQ_SUCCESS) continue; if(op==1){ if(req(&c->current_user,WQ_OP_BACKUP_DATA)!=WQ_SUCCESS) continue; if(wq_create_backup(c->dataset,"backups",path,sizeof(path))==WQ_SUCCESS) printf("备份: %s\n",path);} else if(op==2){ if(wq_list_backup_files("backups",files,WQ_MAX_BACKUP_FILES,&n)==WQ_SUCCESS){ for(i=0;i<n;i++) printf("%lu. %s\n",(unsigned long)(i+1),files[i]); }} else if(op==3){ int idx; WaterQualityDataset *rest; if(req(&c->current_user,WQ_OP_RESTORE_DATA)!=WQ_SUCCESS) continue; if(wq_list_backup_files("backups",files,WQ_MAX_BACKUP_FILES,&n)!=WQ_SUCCESS||n==0){puts("无备份");continue;} for(i=0;i<n;i++) printf("%lu. %s\n",(unsigned long)(i+1),files[i]); if(wq_read_int("选择恢复编号:",&idx)!=WQ_SUCCESS||idx<1||(size_t)idx>n) continue; { int npath = snprintf(path, sizeof(path), "backups/%s", files[idx-1]);
            if (npath < 0 || (size_t)npath >= sizeof(path)) { puts("备份路径过长"); continue; }
            }
            rest=wq_restore_backup(path,&c->overview); if(rest){ wq_dataset_destroy(c->dataset); c->dataset=rest; c->data_loaded=true; puts("恢复成功"); } } }}

static void handle_user(SystemContext *c){ int op; for(;;){ wq_show_user_menu(c->current_user.role); if(wq_read_int("选择:",&op)!=WQ_SUCCESS) return; if(op==0) return; if(op==1){ char u[64],p[64]; int r; if(req(&c->current_user,WQ_OP_CREATE_USER)!=WQ_SUCCESS) continue; printf("用户名:"); if(!fgets(u,sizeof(u),stdin)) continue; printf("密码:"); if(!fgets(p,sizeof(p),stdin)) continue; wq_trim(u); wq_trim(p); if(wq_read_int("角色(0管理员 1普通 2访客):",&r)!=WQ_SUCCESS) continue; puts(wq_create_user(&c->user_store,u,p,(WQUserRole)r)==WQ_SUCCESS?"创建成功":"创建失败"); } else if(op==2){ if(req(&c->current_user,WQ_OP_EXPORT_USERS)!=WQ_SUCCESS) continue; puts(wq_export_users_to_csv(&c->user_store,WQ_USER_EXPORT_CSV_FILE,true)==WQ_SUCCESS?"导出成功":"导出失败"); } else if(op==3){ printf("当前用户: %s (%s)\n",c->current_user.username,wq_role_to_string(c->current_user.role)); }} }

void wq_handle_main_menu(SystemContext *context, int option){ if(!context) return; switch(option){ case 1: handle_data(context); break; case 2: handle_pre(context); break; case 3: handle_stats(context); break; case 4: handle_pred(context); break; case 5: (void)wq_write_overview_report("output/reports/overview.txt",&context->overview); (void)wq_view_text_report("output/reports/overview.txt"); break; case 6: (void)wq_view_text_report("output/reports/warning.txt"); break; case 7: (void)wq_view_text_report("output/reports/statistics.txt"); (void)wq_view_text_report("output/reports/prediction.txt"); break; case 8: handle_backup(context); break; case 9: handle_user(context); break; default: break; }}

WQOperation wq_data_menu_option_to_operation(int option){(void)option;return WQ_OP_NONE;} WQOperation wq_preprocess_menu_option_to_operation(int option){(void)option;return WQ_OP_NONE;} WQOperation wq_statistics_menu_option_to_operation(int option){(void)option;return WQ_OP_NONE;} WQOperation wq_prediction_menu_option_to_operation(int option){(void)option;return WQ_OP_NONE;} WQOperation wq_backup_menu_option_to_operation(int option){(void)option;return WQ_OP_NONE;} WQOperation wq_user_menu_option_to_operation(int option){(void)option;return WQ_OP_NONE;}

int wq_initialize_system_context(SystemContext *context){ if(!context) return WQ_ERROR; memset(context,0,sizeof(*context)); if(wq_ensure_directory("output")!=WQ_SUCCESS||wq_ensure_directory("output/reports")!=WQ_SUCCESS||wq_ensure_directory("output/clean")!=WQ_SUCCESS||wq_ensure_directory("data")!=WQ_SUCCESS||wq_ensure_directory("data/auth")!=WQ_SUCCESS||wq_ensure_directory("backups")!=WQ_SUCCESS) return WQ_ERROR; if(wq_auth_startup(&context->user_store)!=WQ_SUCCESS) return WQ_ERROR; (void)wq_auto_load_dataset(context); return WQ_SUCCESS; }
void wq_destroy_system_context(SystemContext *context){ if(!context) return; wq_dataset_destroy(context->dataset); context->dataset=NULL; (void)wq_auth_shutdown(&context->user_store); wq_secure_clear(&context->current_user,sizeof(context->current_user)); }
int wq_auto_load_dataset(SystemContext *context){ WaterQualityDataset *d; if(!context) return WQ_ERROR; d=wq_read_binary(WQ_CLEAN_BIN_FILE); if(!d) d=wq_read_csv(WQ_DEFAULT_DATA_FILE,&context->overview); if(!d){ context->data_loaded=false; return WQ_ERROR;} wq_dataset_destroy(context->dataset); context->dataset=d; context->data_loaded=true; return WQ_SUCCESS; }
void wq_run_system(void){ SystemContext c; int op,confirm; if(wq_initialize_system_context(&c)!=WQ_SUCCESS){ puts("初始化失败"); return; } if(wq_login(&c.user_store,&c.current_user)!=WQ_SUCCESS){ puts("登录失败"); wq_destroy_system_context(&c); return; } c.logged_in=true; if(!c.data_loaded) puts("未自动加载数据，可在后续菜单处理。"); for(;;){ wq_show_main_menu(c.current_user.role); if(wq_read_int("请选择:",&op)!=WQ_SUCCESS) break; if(!main_option_allowed(c.current_user.role, op)){ puts("权限不足"); continue; } if(op==0){ if(wq_read_int("确认退出? 1是 0否:",&confirm)==WQ_SUCCESS&&confirm==1) break; else continue; } wq_handle_main_menu(&c,op);} wq_destroy_system_context(&c); }
