#ifndef WQ_CONFIG_H
#define WQ_CONFIG_H

/*
 * config.h
 *
 * 全局配置文件。
 * 只存放不会频繁变化的常量、默认路径、参数范围和统一返回码。
 */

/* 系统基本信息 */
#define WQ_SYSTEM_NAME "海水养殖水质分析系统"
#define WQ_VERSION "1.1"

/* 默认输入输出路径 */
#define WQ_DEFAULT_DATA_FILE "data/data_modify.csv"
#define WQ_CLEAN_CSV_FILE "output/clean_data.csv"
#define WQ_CLEAN_BIN_FILE "output/clean_data.bin"
#define WQ_OVERVIEW_FILE "output/data_overview.txt"
#define WQ_STATISTICS_REPORT_FILE "output/statistics_report.txt"
#define WQ_WARNING_REPORT_FILE "output/warning_report.txt"
#define WQ_PREDICTION_REPORT_FILE "output/prediction_report.txt"
#define WQ_BACKUP_DIR "backup"

/* 用户账户文件路径。
 * 仅保存运行中创建的用户，不保存默认 admin / guest 明文账户。
 */
#define WQ_USER_DATA_DIR "data/auth"
#define WQ_USER_BIN_FILE "data/auth/users.dat"
#define WQ_USER_EXPORT_CSV_FILE "output/users_export.csv"

/* 用户二进制文件格式标识 */
#define WQ_USER_FILE_MAGIC "WQUSR001"
#define WQ_USER_FILE_VERSION 1U

/* 数据规模与缓冲区设置 */
#define WQ_INITIAL_CAPACITY 1000U
#define WQ_PAGE_SIZE 15U
#define WQ_MAX_LINE_LENGTH 512U
#define WQ_MAX_PATH_LENGTH 260U
#define WQ_MAX_BACKUP_FILES 128U
#define WQ_MAX_DISCUSSION_TITLE_LENGTH 64U
#define WQ_MAX_DISCUSSION_TEXT_LENGTH 1024U

/* 用户账户长度限制 */
#define WQ_MAX_USERS 128U
#define WQ_DEFAULT_USER_COUNT 2U
#define WQ_MAX_USERNAME_LENGTH 32U
#define WQ_MAX_PASSWORD_LENGTH 64U
#define WQ_MAX_PASSWORD_HASH_LENGTH 33U      /* MD5 十六进制字符串：32位 + '\0' */
#define WQ_SALT_LENGTH 16U                   /* 盐值原始字节数 */
#define WQ_SALT_HEX_LENGTH 33U               /* 盐值十六进制字符串：32位 + '\0' */
#define WQ_MAX_LOGIN_ATTEMPTS 3U

/* 缺失值标记。CSV 空字段和 NaN 文本由解析函数单独判断。 */
#define WQ_MISSING_MARKER_1 (-999.0)
#define WQ_MISSING_MARKER_2 (-9999.0)

/* 各水质参数合理范围，用于异常值检测与修改前校验 */
#define WQ_TEMP_MIN (-5.0)
#define WQ_TEMP_MAX 40.0
#define WQ_SALINITY_MIN 0.0
#define WQ_SALINITY_MAX 45.0
#define WQ_PH_MIN 6.5
#define WQ_PH_MAX 9.0
#define WQ_DO_MIN 0.0
#define WQ_DO_MAX 15.0
#define WQ_PRECIPITATION_MIN 0.0
#define WQ_PRECIPITATION_MAX 500.0
#define WQ_AIR_TEMP_MIN (-10.0)
#define WQ_AIR_TEMP_MAX 50.0

/* 数据采样规则：每 5 分钟一条，每天 288 条 */
#define WQ_DAY_RECORDS 288U
#define WQ_HOUR_RECORDS 12U
#define WQ_SAMPLE_INTERVAL_MINUTES 5U

/* 预警阈值 */
#define WQ_DO_MILD_HYPOXIA_LIMIT 4.0
#define WQ_DO_SEVERE_HYPOXIA_LIMIT 3.0
#define WQ_SALINITY_HOUR_DROP_LIMIT 2.0
#define WQ_SALINITY_DAY_DROP_LIMIT 5.0

/* 统一返回码：成功返回 0，失败返回 -1 */
#define WQ_SUCCESS 0
#define WQ_ERROR (-1)

#endif
