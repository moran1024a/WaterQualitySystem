#ifndef WQ_TYPES_H
#define WQ_TYPES_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "config.h"

/*
 * types.h
 *
 * 系统公共数据结构定义。
 * 该文件不实现任何功能，只定义各模块共同使用的数据类型。
 */

/* 水质参数编号。枚举值同时作为 value[] 和 missing[] 的下标。 */
typedef enum WQParameter {
    WQ_PARAM_TEMP = 0,          /* 水温 Temp，单位：℃ */
    WQ_PARAM_SALINITY,          /* 盐度 Salinity，单位：PSU */
    WQ_PARAM_PH,                /* pH 值 */
    WQ_PARAM_DO,                /* 溶解氧 DO，单位：mg/L */
    WQ_PARAM_PRECIPITATION,     /* 降水量 precipitation，单位：m */
    WQ_PARAM_AIR_TEMP,          /* 气温 Air_temp，单位：℃ */
    WQ_PARAM_COUNT              /* 参数总数，固定为 6 */
} WQParameter;

/* 排序方向 */
typedef enum WQSortOrder {
    WQ_SORT_ASC = 0,
    WQ_SORT_DESC
} WQSortOrder;

/* 数据存储格式 */
typedef enum WQStorageFormat {
    WQ_STORAGE_CSV = 0,
    WQ_STORAGE_BINARY
} WQStorageFormat;

/*
 * 用户角色。
 * 管理员：系统管理与全部业务权限。
 * 普通用户：可进行数据读取、浏览、预处理、统计、预测和查看报告。
 * 访客：只允许查看数据概览和统计分析类报告。
 */
typedef enum WQUserRole {
    WQ_ROLE_ADMIN = 0,
    WQ_ROLE_USER,
    WQ_ROLE_GUEST
} WQUserRole;

/* 用户来源。默认账户不写入用户二进制文件，创建账户需要加密落盘。 */
typedef enum WQUserSource {
    WQ_USER_SOURCE_BUILTIN = 0,
    WQ_USER_SOURCE_CREATED
} WQUserSource;

/* 账户状态。当前不提供修改用户信息功能，预留禁用状态便于扩展。 */
typedef enum WQUserStatus {
    WQ_USER_STATUS_ACTIVE = 0,
    WQ_USER_STATUS_DISABLED
} WQUserStatus;

/*
 * 操作级权限枚举。
 * 所有菜单项和关键业务函数调用前都应映射到一个 WQOperation，再统一调用 auth 模块鉴权。
 */
typedef enum WQOperation {
    WQ_OP_NONE = 0,

    /* 系统与账户 */
    WQ_OP_SYSTEM_STARTUP,
    WQ_OP_LOGIN,
    WQ_OP_CREATE_USER,
    WQ_OP_EXPORT_USERS,
    WQ_OP_LOAD_USERS,
    WQ_OP_SAVE_USERS,

    /* 主菜单/子菜单入口。菜单入口只表示进入某类功能，不等同于执行具体业务。 */
    WQ_OP_DATA_MENU,
    WQ_OP_PREPROCESS_MENU,
    WQ_OP_STATISTICS_MENU,
    WQ_OP_PREDICTION_MENU,
    WQ_OP_BACKUP_MENU,
    WQ_OP_USER_MENU,

    /* 数据基础具体操作 */
    WQ_OP_LOAD_DATA,
    WQ_OP_SAVE_DATA,
    WQ_OP_BROWSE_DATA,
    WQ_OP_FILTER_DATA,
    WQ_OP_SORT_DATA,
    WQ_OP_MODIFY_DATA,
    WQ_OP_DELETE_DATA,

    /* 数据处理与分析 */
    WQ_OP_PREPROCESS_DATA,
    WQ_OP_STATISTICS_ANALYSIS,
    WQ_OP_WARNING_ANALYSIS,
    WQ_OP_PREDICTION_ANALYSIS,

    /* 报告与备份 */
    WQ_OP_VIEW_OVERVIEW,
    WQ_OP_VIEW_WARNING_REPORT,
    WQ_OP_VIEW_ANALYSIS_REPORT,
    WQ_OP_BACKUP_DATA,
    WQ_OP_RESTORE_DATA,

    WQ_OP_CLEAR_SCREEN,
    WQ_OP_EXIT_SYSTEM
} WQOperation;

/* 预警类型 */
typedef enum WQWarningType {
    WQ_WARNING_NONE = 0,
    WQ_WARNING_MILD_HYPOXIA,
    WQ_WARNING_SEVERE_HYPOXIA,
    WQ_WARNING_SALINITY_MUTATION
} WQWarningType;

/* 简化时间结构。 */
typedef struct WQDateTime {
    int year;
    int month;
    int day;
    int hour;
    int minute;
} WQDateTime;

/* 单条水质记录。 */
typedef struct WaterQualityRecord {
    size_t index;
    WQDateTime time;
    double value[WQ_PARAM_COUNT];
    bool missing[WQ_PARAM_COUNT];
    bool valid;
} WaterQualityRecord;

/* 动态数据集。 */
typedef struct WaterQualityDataset {
    WaterQualityRecord *records;
    size_t size;
    size_t capacity;
} WaterQualityDataset;

/* CSV 与二进制存储性能对比结果 */
typedef struct StorageBenchmark {
    WQStorageFormat format;
    unsigned long file_size_bytes;
    double write_seconds;
    double read_seconds;
    bool human_readable;
} StorageBenchmark;

/* 数据概览统计。 */
typedef struct DataOverview {
    size_t total_records;
    size_t valid_records;
    size_t invalid_records;
    size_t missing_values;
    size_t fixed_missing_values;
    size_t outlier_records;
    size_t fixed_outlier_records;
    size_t deleted_outlier_records;
    size_t outlier_parameter_count;
    WQDateTime first_outlier_time;
    WQDateTime last_outlier_time;
    double filter_stddev_before[WQ_PARAM_COUNT];
    double filter_stddev_after[WQ_PARAM_COUNT];
    double filter_stddev_delta[WQ_PARAM_COUNT];

    size_t filter_windows[WQ_FILTER_WINDOW_COUNT];
    double filter_window_stddev_before[WQ_FILTER_WINDOW_COUNT][WQ_PARAM_COUNT];
    double filter_window_stddev_after[WQ_FILTER_WINDOW_COUNT][WQ_PARAM_COUNT];
    double filter_window_stddev_delta[WQ_FILTER_WINDOW_COUNT][WQ_PARAM_COUNT];
    size_t best_filter_window[WQ_PARAM_COUNT];
    bool filter_window_comparison_valid;

    StorageBenchmark csv_storage;
    StorageBenchmark binary_storage;
    bool storage_benchmark_valid;
} DataOverview;

/* 单个参数的基本统计量 */
typedef struct ParameterStatistics {
    double mean;
    double max;
    double min;
    double stddev;
    size_t count;
} ParameterStatistics;

/* 统计分析总结果 */
typedef struct StatisticsResult {
    ParameterStatistics parameter_stats[WQ_PARAM_COUNT];
    double correlation_matrix[WQ_PARAM_COUNT][WQ_PARAM_COUNT];
} StatisticsResult;

/* 预警记录 */
typedef struct WarningRecord {
    WQDateTime time;
    WQWarningType type;
    char message[128];
    char suggestion[256];
} WarningRecord;

/* 单因素线性回归模型：y = slope * x + intercept */
typedef struct LinearRegressionModel {
    WQParameter x_param;
    WQParameter y_param;
    double slope;
    double intercept;
    double r_squared;
    double rmse;
} LinearRegressionModel;

/* 分析讨论主题。用于将任务书要求的讨论性结论统一写入报告。 */
typedef enum WQDiscussionTopic {
    WQ_DISCUSSION_STORAGE_FORMAT = 0,       /* CSV 与二进制存储场景、空间/时间差异 */
    WQ_DISCUSSION_OUTLIER_PROCESSING,       /* 异常值处理方法及合理性 */
    WQ_DISCUSSION_FILTER_WINDOW,            /* 滤波窗口与噪声抑制关系 */
    WQ_DISCUSSION_CORRELATION,              /* 相关性矩阵结论 */
    WQ_DISCUSSION_REGRESSION_ACCURACY,      /* 单因素线性回归准确度与局限 */
    WQ_DISCUSSION_CUSTOM                    /* 其他补充讨论 */
} WQDiscussionTopic;

/* 报告中的讨论段落。content 由统计、预处理、预测等模块准备，report 模块只负责输出。 */
typedef struct AnalysisDiscussion {
    WQDiscussionTopic topic;
    char title[WQ_MAX_DISCUSSION_TITLE_LENGTH];
    char content[WQ_MAX_DISCUSSION_TEXT_LENGTH];
} AnalysisDiscussion;

/*
 * 运行期账户。
 * password_hash 保存“明文密码 + salt”计算后的 MD5 十六进制结果。
 * 默认账户可在初始化时计算散列，也可由验证函数单独处理；创建账户必须保存散列值。
 */
typedef struct UserAccount {
    char username[WQ_MAX_USERNAME_LENGTH];
    char password_hash[WQ_MAX_PASSWORD_HASH_LENGTH];
    unsigned char salt[WQ_SALT_LENGTH];
    WQUserRole role;
    WQUserSource source;
    WQUserStatus status;
} UserAccount;

/* 用户动态表。程序启动时由默认账户 + users.dat 共同组成。 */
typedef struct UserStore {
    UserAccount *users;
    size_t count;
    size_t capacity;
} UserStore;

/* 用户二进制文件头。用于校验文件格式、版本和记录数量。 */
typedef struct UserFileHeader {
    char magic[8];
    uint32_t version;
    uint32_t count;
} UserFileHeader;

/* 用户二进制文件中的单条记录。只保存创建账户，不保存明文密码。 */
typedef struct UserFileRecord {
    char username[WQ_MAX_USERNAME_LENGTH];
    char password_hash[WQ_MAX_PASSWORD_HASH_LENGTH];
    unsigned char salt[WQ_SALT_LENGTH];
    uint32_t role;
    uint32_t status;
} UserFileRecord;

/* 系统运行上下文。菜单层通过该结构统一传递数据、登录用户和用户表。 */
typedef struct SystemContext {
    WaterQualityDataset *dataset;
    DataOverview overview;
    StatisticsResult statistics;
    UserStore user_store;
    UserAccount current_user;
    bool logged_in;
    bool data_loaded;
} SystemContext;

#endif
