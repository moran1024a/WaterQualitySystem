#ifndef WQ_AUTH_H
#define WQ_AUTH_H

#include <stddef.h>
#include <stdbool.h>
#include "types.h"

/*
 * auth.h
 *
 * 登录、用户创建、用户文件加载/保存和操作级权限控制接口。
 * 当前版本只定义接口与约定，不实现具体加密、文件读写和交互逻辑。
 */

/* ---------- 用户表生命周期 ---------- */

/* 初始化空用户表。成功返回 WQ_SUCCESS。 */
int wq_user_store_init(UserStore *store, size_t initial_capacity);

/* 释放用户表内部动态数组，并清空计数。允许重复调用。 */
void wq_user_store_destroy(UserStore *store);

/* 清空用户表，但保留已分配容量。 */
void wq_user_store_clear(UserStore *store);

/* 调整用户表容量。new_capacity 小于当前容量时不缩容。 */
int wq_user_store_reserve(UserStore *store, size_t new_capacity);

/* 向用户表追加账户。由调用方保证用户名不重复。 */
int wq_user_store_push(UserStore *store, const UserAccount *account);

/* 按用户名查找账户，找不到返回 NULL。 */
UserAccount *wq_find_user(UserStore *store, const char *username);
const UserAccount *wq_find_user_const(const UserStore *store, const char *username);

/* ---------- 默认用户与启动加载 ---------- */

/* 写入默认内置账户：admin / 123456，guest / guest。 */
int wq_add_builtin_users(UserStore *store);

/* 从 users.dat 加载创建账户，再与默认账户组成完整用户表。 */
int wq_load_user_store(UserStore *store, const char *filename);

/* 只将创建账户保存到 users.dat，默认账户不落盘。 */
int wq_save_created_users(const UserStore *store, const char *filename);

/* 程序启动时调用：初始化用户表、加入默认账户、加载已创建账户。 */
int wq_auth_startup(UserStore *store);

/* 程序退出前调用：保存创建账户，释放用户表。 */
int wq_auth_shutdown(UserStore *store);

/* ---------- 密码散列与安全辅助 ---------- */

/* 生成随机盐。实际实现可使用时间、rand 或平台随机源。 */
int wq_generate_salt(unsigned char salt[WQ_SALT_LENGTH]);

/* 使用 MD5 + salt 生成密码散列。hash_out 长度至少为 WQ_MAX_PASSWORD_HASH_LENGTH。 */
int wq_md5_with_salt(const char *password,
                     const unsigned char salt[WQ_SALT_LENGTH],
                     char hash_out[WQ_MAX_PASSWORD_HASH_LENGTH]);

/* 常量时间字符串比较接口，用于降低密码散列比较的时序泄露风险。 */
bool wq_secure_hash_equal(const char *hash_a, const char *hash_b);

/* 清除敏感缓冲区，如明文密码输入缓冲区。 */
void wq_secure_clear(void *buffer, size_t size);

/* ---------- 登录与创建用户 ---------- */

/* 验证用户名和明文密码。成功时将账户写入 login_user_out。 */
bool wq_verify_login(const UserStore *store,
                     const char *username,
                     const char *password,
                     UserAccount *login_user_out);

/* 控制台登录流程，最多允许尝试 WQ_MAX_LOGIN_ATTEMPTS 次。 */
int wq_login(const UserStore *store, UserAccount *login_user_out);

/* 创建普通用户账户。由管理员调用；不提供后续修改账户或密码接口。 */
int wq_create_user(UserStore *store,
                   const char *username,
                   const char *password,
                   WQUserRole role);

/* 管理员导出用户列表为 CSV。建议导出用户名、角色、来源、状态、盐、密码MD5，不导出明文密码。 */
int wq_export_users_to_csv(const UserStore *store,
                           const char *csv_filename,
                           bool include_builtin_users);

/* ---------- 权限控制 ---------- */

/* 判断某个角色是否允许执行某个操作。 */
bool wq_role_can_execute(WQUserRole role, WQOperation operation);

/* 判断当前用户是否允许执行某个操作。 */
bool wq_user_can_execute(const UserAccount *user, WQOperation operation);

/* 执行业务前统一鉴权。无权限返回 WQ_ERROR。 */
int wq_require_permission(const UserAccount *user, WQOperation operation);

/* 菜单编号转操作编号。子菜单可使用独立映射函数。 */
WQOperation wq_main_menu_option_to_operation(int menu_option);

/* 角色、操作枚举转文本，便于菜单和错误提示显示。 */
const char *wq_role_to_string(WQUserRole role);
const char *wq_operation_to_string(WQOperation operation);

#endif
