#include "auth.h"
#include <stdlib.h>
#include <string.h>

/*
 * auth.c
 *
 * 用户登录、用户创建、二进制账户文件和权限控制模块函数框架。
 * 当前只给出接口约定和必要的动态数组基础代码，不实现真正的 MD5、文件格式读写和控制台交互。
 */

int wq_user_store_init(UserStore *store, size_t initial_capacity)
{
    if (store == NULL) {
        return WQ_ERROR;
    }

    store->users = NULL;
    store->count = 0U;
    store->capacity = 0U;

    if (initial_capacity > 0U) {
        return wq_user_store_reserve(store, initial_capacity);
    }

    return WQ_SUCCESS;
}

void wq_user_store_destroy(UserStore *store)
{
    if (store == NULL) {
        return;
    }

    if (store->users != NULL) {
        wq_secure_clear(store->users, store->capacity * sizeof(UserAccount));
        free(store->users);
    }

    store->users = NULL;
    store->count = 0U;
    store->capacity = 0U;
}

void wq_user_store_clear(UserStore *store)
{
    if (store == NULL) {
        return;
    }

    if (store->users != NULL) {
        wq_secure_clear(store->users, store->capacity * sizeof(UserAccount));
    }

    store->count = 0U;
}

int wq_user_store_reserve(UserStore *store, size_t new_capacity)
{
    UserAccount *new_users;

    if (store == NULL) {
        return WQ_ERROR;
    }

    if (new_capacity <= store->capacity) {
        return WQ_SUCCESS;
    }

    if (new_capacity > WQ_MAX_USERS) {
        return WQ_ERROR;
    }

    new_users = (UserAccount *)realloc(store->users, new_capacity * sizeof(UserAccount));
    if (new_users == NULL) {
        return WQ_ERROR;
    }

    store->users = new_users;
    store->capacity = new_capacity;
    return WQ_SUCCESS;
}

int wq_user_store_push(UserStore *store, const UserAccount *account)
{
    size_t new_capacity;

    if (store == NULL || account == NULL) {
        return WQ_ERROR;
    }

    if (store->count >= store->capacity) {
        new_capacity = store->capacity == 0U ? WQ_DEFAULT_USER_COUNT : store->capacity * 2U;
        if (new_capacity > WQ_MAX_USERS) {
            new_capacity = WQ_MAX_USERS;
        }
        if (new_capacity <= store->capacity || wq_user_store_reserve(store, new_capacity) != WQ_SUCCESS) {
            return WQ_ERROR;
        }
    }

    store->users[store->count] = *account;
    store->count++;
    return WQ_SUCCESS;
}

UserAccount *wq_find_user(UserStore *store, const char *username)
{
    size_t i;

    if (store == NULL || username == NULL) {
        return NULL;
    }

    for (i = 0U; i < store->count; ++i) {
        if (strcmp(store->users[i].username, username) == 0) {
            return &store->users[i];
        }
    }

    return NULL;
}

const UserAccount *wq_find_user_const(const UserStore *store, const char *username)
{
    size_t i;

    if (store == NULL || username == NULL) {
        return NULL;
    }

    for (i = 0U; i < store->count; ++i) {
        if (strcmp(store->users[i].username, username) == 0) {
            return &store->users[i];
        }
    }

    return NULL;
}

int wq_add_builtin_users(UserStore *store)
{
    UserAccount admin;
    UserAccount guest;

    if (store == NULL) {
        return WQ_ERROR;
    }

    memset(&admin, 0, sizeof(admin));
    strcpy(admin.username, "admin");
    /* 待实现：默认账户也可在运行时转换为 MD5+salt 散列。 */
    strcpy(admin.password_hash, "BUILTIN_ADMIN_PASSWORD_PLACEHOLDER");
    admin.role = WQ_ROLE_ADMIN;
    admin.source = WQ_USER_SOURCE_BUILTIN;
    admin.status = WQ_USER_STATUS_ACTIVE;

    memset(&guest, 0, sizeof(guest));
    strcpy(guest.username, "guest");
    strcpy(guest.password_hash, "BUILTIN_GUEST_PASSWORD_PLACEHOLDER");
    guest.role = WQ_ROLE_GUEST;
    guest.source = WQ_USER_SOURCE_BUILTIN;
    guest.status = WQ_USER_STATUS_ACTIVE;

    if (wq_user_store_push(store, &admin) != WQ_SUCCESS) {
        return WQ_ERROR;
    }
    if (wq_user_store_push(store, &guest) != WQ_SUCCESS) {
        return WQ_ERROR;
    }

    return WQ_SUCCESS;
}

int wq_load_user_store(UserStore *store, const char *filename)
{
    /*
     * 待实现：
     * 1. 以 rb 方式打开 users.dat。
     * 2. 文件不存在时不视为致命错误，表示暂无创建用户。
     * 3. 读取 UserFileHeader，校验 magic、version、count。
     * 4. 循环读取 UserFileRecord，转换为 UserAccount 后加入 store。
     * 5. 任一步失败时释放临时资源，避免半加载状态。
     */
    (void)store;
    (void)filename;
    return WQ_SUCCESS;
}

int wq_save_created_users(const UserStore *store, const char *filename)
{
    /*
     * 待实现：
     * 1. 统计 source == WQ_USER_SOURCE_CREATED 的账户数量。
     * 2. 写入 UserFileHeader。
     * 3. 逐条写入 UserFileRecord。
     * 4. 不写入默认 admin / guest。
     * 5. 写入失败时关闭文件并返回 WQ_ERROR。
     */
    (void)store;
    (void)filename;
    return WQ_ERROR;
}

int wq_auth_startup(UserStore *store)
{
    if (store == NULL) {
        return WQ_ERROR;
    }

    if (wq_user_store_init(store, WQ_DEFAULT_USER_COUNT) != WQ_SUCCESS) {
        return WQ_ERROR;
    }

    if (wq_add_builtin_users(store) != WQ_SUCCESS) {
        wq_user_store_destroy(store);
        return WQ_ERROR;
    }

    if (wq_load_user_store(store, WQ_USER_BIN_FILE) != WQ_SUCCESS) {
        wq_user_store_destroy(store);
        return WQ_ERROR;
    }

    return WQ_SUCCESS;
}

int wq_auth_shutdown(UserStore *store)
{
    int result;

    if (store == NULL) {
        return WQ_ERROR;
    }

    result = wq_save_created_users(store, WQ_USER_BIN_FILE);
    wq_user_store_destroy(store);
    return result;
}

int wq_generate_salt(unsigned char salt[WQ_SALT_LENGTH])
{
    /* 待实现：生成 WQ_SALT_LENGTH 字节随机盐。 */
    (void)salt;
    return WQ_ERROR;
}

int wq_md5_with_salt(const char *password,
                     const unsigned char salt[WQ_SALT_LENGTH],
                     char hash_out[WQ_MAX_PASSWORD_HASH_LENGTH])
{
    /* 待实现：计算 MD5(password + salt)，输出 32 位十六进制字符串。 */
    (void)password;
    (void)salt;
    (void)hash_out;
    return WQ_ERROR;
}

bool wq_secure_hash_equal(const char *hash_a, const char *hash_b)
{
    /* 待实现：使用常量时间比较，避免直接 strcmp 暴露时序信息。 */
    (void)hash_a;
    (void)hash_b;
    return false;
}

void wq_secure_clear(void *buffer, size_t size)
{
    volatile unsigned char *p;
    size_t i;

    if (buffer == NULL) {
        return;
    }

    p = (volatile unsigned char *)buffer;
    for (i = 0U; i < size; ++i) {
        p[i] = 0U;
    }
}

bool wq_verify_login(const UserStore *store,
                     const char *username,
                     const char *password,
                     UserAccount *login_user_out)
{
    const UserAccount *account;
    char input_hash[WQ_MAX_PASSWORD_HASH_LENGTH];

    if (store == NULL || username == NULL || password == NULL || login_user_out == NULL) {
        return false;
    }

    account = wq_find_user_const(store, username);
    if (account == NULL || account->status != WQ_USER_STATUS_ACTIVE) {
        return false;
    }

    /*
     * 待实现：
     * 1. 对默认账户 admin/guest 可直接验证固定口令，或预先转换为散列。
     * 2. 对创建账户使用其 salt 计算输入密码 MD5。
     * 3. 使用 wq_secure_hash_equal 比较散列。
     */
    memset(input_hash, 0, sizeof(input_hash));
    (void)input_hash;

    return false;
}

int wq_login(const UserStore *store, UserAccount *login_user_out)
{
    /* 待实现：控制台读取用户名和密码，最多尝试三次。 */
    (void)store;
    (void)login_user_out;
    return WQ_ERROR;
}

int wq_create_user(UserStore *store,
                   const char *username,
                   const char *password,
                   WQUserRole role)
{
    UserAccount account;

    if (store == NULL || username == NULL || password == NULL) {
        return WQ_ERROR;
    }

    if (role == WQ_ROLE_GUEST) {
        /* 创建用户默认不创建访客；访客由系统内置提供。 */
        return WQ_ERROR;
    }

    if (wq_find_user(store, username) != NULL) {
        return WQ_ERROR;
    }

    memset(&account, 0, sizeof(account));
    /* 待实现：安全拷贝 username，生成 salt，计算 password_hash。 */
    account.role = role;
    account.source = WQ_USER_SOURCE_CREATED;
    account.status = WQ_USER_STATUS_ACTIVE;

    return wq_user_store_push(store, &account);
}

int wq_export_users_to_csv(const UserStore *store,
                           const char *csv_filename,
                           bool include_builtin_users)
{
    /*
     * 待实现：管理员调用。
     * 建议 CSV 字段：username,role,source,status,salt_hex,password_md5。
     * 不导出任何明文密码。
     */
    (void)store;
    (void)csv_filename;
    (void)include_builtin_users;
    return WQ_ERROR;
}

bool wq_role_can_execute(WQUserRole role, WQOperation operation)
{
    if (operation == WQ_OP_NONE) {
        return false;
    }

    if (operation == WQ_OP_LOGIN || operation == WQ_OP_SYSTEM_STARTUP || operation == WQ_OP_EXIT_SYSTEM) {
        return true;
    }

    if (role == WQ_ROLE_ADMIN) {
        return true;
    }

    if (role == WQ_ROLE_USER) {
        switch (operation) {
        case WQ_OP_LOAD_DATA:
        case WQ_OP_SAVE_DATA:
        case WQ_OP_BROWSE_DATA:
        case WQ_OP_FILTER_DATA:
        case WQ_OP_SORT_DATA:
        case WQ_OP_PREPROCESS_DATA:
        case WQ_OP_STATISTICS_ANALYSIS:
        case WQ_OP_WARNING_ANALYSIS:
        case WQ_OP_PREDICTION_ANALYSIS:
        case WQ_OP_VIEW_OVERVIEW:
        case WQ_OP_VIEW_WARNING_REPORT:
        case WQ_OP_VIEW_ANALYSIS_REPORT:
        case WQ_OP_BACKUP_DATA:
        case WQ_OP_CLEAR_SCREEN:
            return true;
        default:
            return false;
        }
    }

    if (role == WQ_ROLE_GUEST) {
        switch (operation) {
        case WQ_OP_STATISTICS_ANALYSIS:
        case WQ_OP_VIEW_OVERVIEW:
        case WQ_OP_VIEW_ANALYSIS_REPORT:
        case WQ_OP_CLEAR_SCREEN:
            return true;
        default:
            return false;
        }
    }

    return false;
}

bool wq_user_can_execute(const UserAccount *user, WQOperation operation)
{
    if (user == NULL || user->status != WQ_USER_STATUS_ACTIVE) {
        return false;
    }

    return wq_role_can_execute(user->role, operation);
}

int wq_require_permission(const UserAccount *user, WQOperation operation)
{
    return wq_user_can_execute(user, operation) ? WQ_SUCCESS : WQ_ERROR;
}

WQOperation wq_main_menu_option_to_operation(int menu_option)
{
    switch (menu_option) {
    case 1:
        return WQ_OP_BROWSE_DATA;
    case 2:
        return WQ_OP_PREPROCESS_DATA;
    case 3:
        return WQ_OP_STATISTICS_ANALYSIS;
    case 4:
        return WQ_OP_PREDICTION_ANALYSIS;
    case 5:
        return WQ_OP_VIEW_OVERVIEW;
    case 6:
        return WQ_OP_VIEW_WARNING_REPORT;
    case 7:
        return WQ_OP_VIEW_ANALYSIS_REPORT;
    case 8:
        return WQ_OP_BACKUP_DATA;
    case 9:
        return WQ_OP_CLEAR_SCREEN;
    case 10:
        return WQ_OP_CREATE_USER;
    case 11:
        return WQ_OP_EXPORT_USERS;
    case 0:
        return WQ_OP_EXIT_SYSTEM;
    default:
        return WQ_OP_NONE;
    }
}

const char *wq_role_to_string(WQUserRole role)
{
    switch (role) {
    case WQ_ROLE_ADMIN:
        return "管理员";
    case WQ_ROLE_USER:
        return "普通用户";
    case WQ_ROLE_GUEST:
        return "访客";
    default:
        return "未知角色";
    }
}

const char *wq_operation_to_string(WQOperation operation)
{
    switch (operation) {
    case WQ_OP_CREATE_USER:
        return "创建用户";
    case WQ_OP_EXPORT_USERS:
        return "导出用户列表";
    case WQ_OP_LOAD_DATA:
        return "加载数据";
    case WQ_OP_SAVE_DATA:
        return "保存数据";
    case WQ_OP_BROWSE_DATA:
        return "浏览数据";
    case WQ_OP_FILTER_DATA:
        return "筛选数据";
    case WQ_OP_SORT_DATA:
        return "排序数据";
    case WQ_OP_MODIFY_DATA:
        return "修改数据";
    case WQ_OP_DELETE_DATA:
        return "删除数据";
    case WQ_OP_PREPROCESS_DATA:
        return "数据预处理";
    case WQ_OP_STATISTICS_ANALYSIS:
        return "统计分析";
    case WQ_OP_WARNING_ANALYSIS:
        return "预警分析";
    case WQ_OP_PREDICTION_ANALYSIS:
        return "预测分析";
    case WQ_OP_VIEW_OVERVIEW:
        return "查看数据概览";
    case WQ_OP_VIEW_WARNING_REPORT:
        return "查看预警报告";
    case WQ_OP_VIEW_ANALYSIS_REPORT:
        return "查看分析报告";
    case WQ_OP_BACKUP_DATA:
        return "数据备份";
    case WQ_OP_RESTORE_DATA:
        return "数据恢复";
    case WQ_OP_CLEAR_SCREEN:
        return "清屏";
    case WQ_OP_EXIT_SYSTEM:
        return "退出系统";
    default:
        return "未知操作";
    }
}
