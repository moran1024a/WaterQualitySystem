# WaterQualitySystem

`WaterQualitySystem` 是“海水养殖水质分析系统”的纯 C 语言工程骨架，用于完成课程设计中的模块划分、结构体定义、函数接口定义、权限边界约定和报告输出约定。

当前版本重点是**框架定义**，多数业务函数仅保留占位实现，不包含完整算法逻辑。

## 1. 编译方式

Linux / MinGW gcc：

```bash
make gcc
```

Windows MSVC Developer Command Prompt：

```bat
nmake /f Makefile msvc
```

清理编译产物：

```bash
make clean
```

## 2. 当前工程状态

当前工程已完成：

1. 多文件 C 工程结构。
2. 核心数据结构定义。
3. 数据文件、报告文件、备份文件、用户文件路径约定。
4. 数据基础操作、预处理、统计分析、预测分析、备份恢复、登录权限、菜单、报告等模块接口。
5. 默认用户、创建用户、用户二进制落盘、MD5+salt 密码散列的接口约定。
6. 操作级权限枚举 `WQOperation`。
7. 主菜单入口级权限枚举，例如 `WQ_OP_DATA_MENU`、`WQ_OP_PREPROCESS_MENU`、`WQ_OP_STATISTICS_MENU`。
8. 报告“分析讨论”接口，用于输出存储格式、异常值处理、滤波窗口、相关性分析、预测准确度等讨论内容。
9. 可通过 gcc 编译检查。

当前工程未完成：

1. CSV 真实读取解析逻辑。
2. 二进制数据真实读写逻辑。
3. MD5 算法和真实密码散列逻辑。
4. 登录交互逻辑。
5. 菜单循环真实交互逻辑。
6. 数据清洗、统计分析、预测模型等具体算法实现。
7. 报告文件真实写入逻辑。

## 3. 目录结构

```text
WaterQualitySystem/
│
├── include/
│   ├── config.h
│   ├── types.h
│   ├── auth.h
│   ├── file_io.h
│   ├── preprocess.h
│   ├── data_manage.h
│   ├── backup.h
│   ├── statistics.h
│   ├── prediction.h
│   ├── menu.h
│   ├── report.h
│   └── utils.h
│
├── src/
│   ├── main.c
│   ├── auth.c
│   ├── file_io.c
│   ├── preprocess.c
│   ├── data_manage.c
│   ├── backup.c
│   ├── statistics.c
│   ├── prediction.c
│   ├── menu.c
│   ├── report.c
│   └── utils.c
│
├── data/
│   └── data_modify.csv
│
├── output/
│   ├── clean_data.csv
│   ├── clean_data.bin
│   ├── data_overview.txt
│   ├── statistics_report.txt
│   ├── warning_report.txt
│   ├── prediction_report.txt
│   ├── users_export.csv
│   └── storage_compare.txt
│
├── backup/
│   └── backup_YYYYMMDD_HHMMSS.csv
│
├── Makefile
└── README.md
```

说明：

- `include/`：头文件，定义宏、结构体、枚举和函数声明。
- `src/`：源文件，保留函数框架、必要参数检查和占位返回值。
- `data/`：原始数据和用户账户二进制文件默认目录。
- `output/`：清洗数据、概览报告、统计报告、预警报告、预测报告、用户导出文件默认目录。
- `backup/`：自动或手动备份文件默认目录。

## 4. 默认路径

主要路径在 `include/config.h` 中定义：

```c
#define WQ_DEFAULT_DATA_FILE "data/data_modify.csv"
#define WQ_CLEAN_CSV_FILE "output/clean_data.csv"
#define WQ_CLEAN_BIN_FILE "output/clean_data.bin"
#define WQ_OVERVIEW_FILE "output/data_overview.txt"
#define WQ_STATISTICS_REPORT_FILE "output/statistics_report.txt"
#define WQ_WARNING_REPORT_FILE "output/warning_report.txt"
#define WQ_PREDICTION_REPORT_FILE "output/prediction_report.txt"
#define WQ_BACKUP_DIR "backup"
#define WQ_USER_BIN_FILE "data/auth/users.dat"
#define WQ_USER_EXPORT_CSV_FILE "output/users_export.csv"
```

启动时数据自动加载顺序约定：

```text
优先读取 output/clean_data.bin
若失败，再读取 data/data_modify.csv
若仍失败，允许进入系统，但访问依赖数据集的功能时应提示未加载数据
```

## 5. 核心数据结构

核心结构体集中定义在 `include/types.h`。

### 5.1 水质参数枚举

```c
typedef enum WQParameter {
    WQ_PARAM_TEMP = 0,
    WQ_PARAM_SALINITY,
    WQ_PARAM_PH,
    WQ_PARAM_DO,
    WQ_PARAM_PRECIPITATION,
    WQ_PARAM_AIR_TEMP,
    WQ_PARAM_COUNT
} WQParameter;
```

共 6 个水质参数：

| 参数 | 英文标识 | 单位 |
|---|---|---|
| 水温 | Temp | ℃ |
| 盐度 | Salinity | PSU |
| pH | pH | — |
| 溶解氧 | DO | mg/L |
| 降水量 | precipitation | m |
| 气温 | Air_temp | ℃ |

### 5.2 单条记录

```c
typedef struct WaterQualityRecord {
    size_t index;
    WQDateTime time;
    double value[WQ_PARAM_COUNT];
    bool missing[WQ_PARAM_COUNT];
    bool valid;
} WaterQualityRecord;
```

说明：

- `index`：记录编号。
- `time`：按起始时间和 5 分钟采样间隔推算得到。
- `value[]`：6 个参数值。
- `missing[]`：对应参数是否缺失。
- `valid`：整条记录是否有效。

### 5.3 动态数据集

```c
typedef struct WaterQualityDataset {
    WaterQualityRecord *records;
    size_t size;
    size_t capacity;
} WaterQualityDataset;
```

约定：

1. 使用动态数组保存数据。
2. 初始容量由 `WQ_INITIAL_CAPACITY` 指定。
3. 容量不足时扩容。
4. 释放由 `wq_dataset_destroy()` 负责。

### 5.4 系统上下文

```c
typedef struct SystemContext {
    WaterQualityDataset *dataset;
    DataOverview overview;
    StatisticsResult statistics;
    UserStore user_store;
    UserAccount current_user;
    bool logged_in;
    bool data_loaded;
} SystemContext;
```

菜单层通过 `SystemContext` 统一传递数据集、统计结果、用户表和当前登录用户。

## 6. 用户与权限设计

### 6.1 用户角色

```c
typedef enum WQUserRole {
    WQ_ROLE_ADMIN = 0,
    WQ_ROLE_USER,
    WQ_ROLE_GUEST
} WQUserRole;
```

| 角色 | 权限说明 |
|---|---|
| 管理员 | 拥有全部功能权限 |
| 普通用户 | 可进行数据读取、浏览、预处理、统计、预测、备份和报告查看，不允许修改/删除数据和管理用户 |
| 访客 | 仅允许查看数据概览和统计分析相关内容 |

### 6.2 默认账户

| 用户名 | 密码 | 角色 | 来源 | 是否落盘 |
|---|---|---|---|---|
| `admin` | `123456` | 管理员 | 系统内置 | 否 |
| `guest` | `guest` | 访客 | 系统内置 | 否 |

约定：

1. 默认账户启动时写入内存。
2. 默认账户不保存到 `data/auth/users.dat`。
3. 创建用户保存到 `data/auth/users.dat`。
4. 创建用户密码不保存明文，只保存 `MD5(密码 + salt)` 后的散列值和盐值。
5. 当前接口不提供修改用户信息或修改密码功能。

### 6.3 用户文件格式

文件路径：

```text
data/auth/users.dat
```

文件内容约定：

```text
UserFileHeader
│
├── magic[8]      固定为 WQUSR001
├── version       当前为 1
└── count         创建用户数量

UserFileRecord[count]
│
├── username
├── password_hash     MD5 + salt 后的十六进制散列
├── salt              16 字节盐值
├── role              用户角色
└── status            用户状态
```

### 6.4 用户导出 CSV

导出路径：

```text
output/users_export.csv
```

建议字段：

```csv
username,role,source,status,salt_hex,password_md5
```

说明：

1. 仅管理员允许导出。
2. 不导出明文密码。
3. 可通过参数选择是否包含内置用户。

## 7. 操作级权限设计

权限枚举定义在 `types.h` 的 `WQOperation` 中。

### 7.1 菜单入口级操作

```c
WQ_OP_DATA_MENU
WQ_OP_PREPROCESS_MENU
WQ_OP_STATISTICS_MENU
WQ_OP_PREDICTION_MENU
WQ_OP_BACKUP_MENU
WQ_OP_USER_MENU
```

这些枚举只表示“进入某类菜单”，不代表已经执行具体业务。

例如：

```text
主菜单 [1] 数据基础操作  -> WQ_OP_DATA_MENU
数据子菜单 [3] 浏览数据  -> WQ_OP_BROWSE_DATA
```

这样可以避免把“进入数据基础操作菜单”错误映射为“浏览数据”。

### 7.2 具体业务操作

```c
WQ_OP_LOAD_DATA
WQ_OP_SAVE_DATA
WQ_OP_BROWSE_DATA
WQ_OP_FILTER_DATA
WQ_OP_SORT_DATA
WQ_OP_MODIFY_DATA
WQ_OP_DELETE_DATA
WQ_OP_PREPROCESS_DATA
WQ_OP_STATISTICS_ANALYSIS
WQ_OP_WARNING_ANALYSIS
WQ_OP_PREDICTION_ANALYSIS
WQ_OP_VIEW_OVERVIEW
WQ_OP_VIEW_WARNING_REPORT
WQ_OP_VIEW_ANALYSIS_REPORT
WQ_OP_BACKUP_DATA
WQ_OP_RESTORE_DATA
WQ_OP_CREATE_USER
WQ_OP_EXPORT_USERS
```

### 7.3 鉴权原则

1. 不只依赖菜单是否显示。
2. 主菜单入口先做入口级鉴权。
3. 进入子菜单后，具体业务操作再次鉴权。
4. 所有关键业务函数调用前都应使用 `wq_require_permission()`。
5. 无权限时返回 `WQ_ERROR`，菜单层负责提示用户。

### 7.4 权限表

| 操作类型 | 管理员 | 普通用户 | 访客 |
|---|---:|---:|---:|
| 登录系统 | 是 | 是 | 是 |
| 数据基础操作菜单 | 是 | 是 | 否 |
| 数据预处理菜单 | 是 | 是 | 否 |
| 统计分析菜单 | 是 | 是 | 是 |
| 预测分析菜单 | 是 | 是 | 否 |
| 备份与恢复菜单 | 是 | 是 | 否 |
| 用户管理菜单 | 是 | 否 | 否 |
| 加载/保存水质数据 | 是 | 是 | 否 |
| 分页浏览/筛选/排序数据 | 是 | 是 | 否 |
| 修改/删除数据 | 是 | 否 | 否 |
| 数据预处理 | 是 | 是 | 否 |
| 统计分析 | 是 | 是 | 是 |
| 预警分析 | 是 | 是 | 否 |
| 预测分析 | 是 | 是 | 否 |
| 查看数据概览 | 是 | 是 | 是 |
| 查看预警报告 | 是 | 是 | 否 |
| 查看统计/分析报告 | 是 | 是 | 是 |
| 手动备份数据 | 是 | 是 | 否 |
| 恢复备份数据 | 是 | 否 | 否 |
| 创建用户 | 是 | 否 | 否 |
| 导出用户列表 CSV | 是 | 否 | 否 |
| 清屏/退出系统 | 是 | 是 | 是 |

## 8. 菜单映射约定

主菜单选项映射函数：

```c
WQOperation wq_main_menu_option_to_operation(int menu_option);
```

当前约定：

| 主菜单选项 | 菜单含义 | 映射操作 |
|---:|---|---|
| 1 | 数据基础操作 | `WQ_OP_DATA_MENU` |
| 2 | 数据预处理 | `WQ_OP_PREPROCESS_MENU` |
| 3 | 统计分析 | `WQ_OP_STATISTICS_MENU` |
| 4 | 预测分析 | `WQ_OP_PREDICTION_MENU` |
| 5 | 查看数据概览 | `WQ_OP_VIEW_OVERVIEW` |
| 6 | 查看预警报告 | `WQ_OP_VIEW_WARNING_REPORT` |
| 7 | 查看分析报告 | `WQ_OP_VIEW_ANALYSIS_REPORT` |
| 8 | 数据备份与恢复 | `WQ_OP_BACKUP_MENU` |
| 9 | 清屏 | `WQ_OP_CLEAR_SCREEN` |
| 10 | 用户管理 | `WQ_OP_USER_MENU` |
| 0 | 退出系统 | `WQ_OP_EXIT_SYSTEM` |

数据基础操作子菜单映射：

| 子菜单选项 | 功能 | 映射操作 |
|---:|---|---|
| 1 | 加载数据 | `WQ_OP_LOAD_DATA` |
| 2 | 保存数据 | `WQ_OP_SAVE_DATA` |
| 3 | 分页浏览 | `WQ_OP_BROWSE_DATA` |
| 4 | 条件筛选 | `WQ_OP_FILTER_DATA` |
| 5 | 参数排序 | `WQ_OP_SORT_DATA` |
| 6 | 修改数据 | `WQ_OP_MODIFY_DATA` |
| 7 | 删除数据 | `WQ_OP_DELETE_DATA` |

备份子菜单映射：

| 子菜单选项 | 功能 | 映射操作 |
|---:|---|---|
| 1 | 备份数据 | `WQ_OP_BACKUP_DATA` |
| 2 | 恢复数据 | `WQ_OP_RESTORE_DATA` |

用户管理子菜单映射：

| 子菜单选项 | 功能 | 映射操作 |
|---:|---|---|
| 1 | 创建用户 | `WQ_OP_CREATE_USER` |
| 2 | 导出用户列表 | `WQ_OP_EXPORT_USERS` |

## 9. 报告输出设计

报告接口定义在 `include/report.h`，实现框架位于 `src/report.c`。

### 9.1 数据概览报告

```c
int wq_write_overview_report(const char *filename, const DataOverview *overview);
```

建议写入内容：

1. 总记录数。
2. 有效记录数。
3. 异常记录数。
4. 缺失值数量。
5. 修复缺失值数量。
6. 修复异常值记录数。
7. 删除异常值记录数。
8. 异常数据时间跨度。
9. 异常数据错误参数个数。

### 9.2 统计分析报告

```c
int wq_write_statistics_report(const char *filename,
                               const StatisticsResult *statistics,
                               const WaterQualityDataset *dataset);
```

建议写入内容：

1. 每个参数的均值、最大值、最小值、标准差。
2. 6×6 皮尔逊相关系数矩阵。
3. 最强正相关、最强负相关。
4. 水温-DO、pH-DO、水温-气温、水温-盐度等指定关系分析。
5. 可追加“相关性分析讨论”。

### 9.3 预警报告

```c
int wq_write_warning_report(const char *filename,
                            const WarningRecord *warnings,
                            size_t warning_count);
```

建议写入内容：

1. 预警时间。
2. 预警种类。
3. 预警说明。
4. 处理建议。

### 9.4 预测报告

```c
int wq_write_prediction_report(const char *filename,
                               const LinearRegressionModel *models,
                               size_t model_count);
```

建议写入内容：

1. 回归模型自变量和因变量。
2. 斜率 `slope`。
3. 截距 `intercept`。
4. 决定系数 `R²`。
5. 留出法 `RMSE`。
6. 气温-DO、水温-DO、pH-DO、盐度-DO 等模型比较。
7. 可追加“线性回归预测准确度讨论”。

### 9.5 分析讨论报告

分析讨论结构体：

```c
typedef struct AnalysisDiscussion {
    WQDiscussionTopic topic;
    char title[WQ_MAX_DISCUSSION_TITLE_LENGTH];
    char content[WQ_MAX_DISCUSSION_TEXT_LENGTH];
} AnalysisDiscussion;
```

讨论主题枚举：

```c
WQ_DISCUSSION_STORAGE_FORMAT
WQ_DISCUSSION_OUTLIER_PROCESSING
WQ_DISCUSSION_FILTER_WINDOW
WQ_DISCUSSION_CORRELATION
WQ_DISCUSSION_REGRESSION_ACCURACY
WQ_DISCUSSION_CUSTOM
```

输出接口：

```c
int wq_write_discussion_report(const char *filename,
                               const AnalysisDiscussion *discussions,
                               size_t discussion_count,
                               bool append);
```

用途：

1. `append=false`：单独生成讨论报告或覆盖旧文件。
2. `append=true`：追加到统计报告、预测报告或其他报告末尾。
3. `report.c` 只负责输出讨论文本，不负责计算结论。
4. 讨论内容由 `file_io.c`、`preprocess.c`、`statistics.c`、`prediction.c` 等模块生成。

可覆盖的任务书讨论项：

| 讨论内容 | 建议主题 |
|---|---|
| CSV 与二进制格式适用场景、空间/时间差异 | `WQ_DISCUSSION_STORAGE_FORMAT` |
| 异常值处理方法及合理性 | `WQ_DISCUSSION_OUTLIER_PROCESSING` |
| 滤波窗口大小与噪声抑制程度关系 | `WQ_DISCUSSION_FILTER_WINDOW` |
| 相关性矩阵结论 | `WQ_DISCUSSION_CORRELATION` |
| 单因素线性回归预测准确度和原因 | `WQ_DISCUSSION_REGRESSION_ACCURACY` |

## 10. 启动流程约定

```text
main()
│
└── wq_run_system()
    │
    ├── wq_initialize_system_context()
    │   │
    │   ├── 初始化 SystemContext
    │   ├── wq_auth_startup()
    │   │   ├── 初始化 UserStore
    │   │   ├── 加入默认 admin / guest
    │   │   └── 自动加载 data/auth/users.dat 中的创建用户
    │   │
    │   └── wq_auto_load_dataset()
    │       ├── 优先读取 output/clean_data.bin
    │       └── 若失败则尝试读取 data/data_modify.csv
    │
    ├── wq_login()
    │   ├── 最多允许尝试 3 次
    │   └── 登录成功后保存 current_user
    │
    ├── 菜单循环
    │   ├── 主菜单入口鉴权
    │   ├── 子菜单具体操作鉴权
    │   └── 调用对应业务模块
    │
    └── wq_destroy_system_context()
        ├── 保存创建用户到 data/auth/users.dat
        ├── 释放 Dataset
        ├── 释放 UserStore
        └── 清理当前登录用户敏感信息
```

## 11. 模块说明

### 11.1 `config.h`

功能：全局配置文件。

负责内容：

1. 系统名称和版本号。
2. 数据文件路径。
3. 报告文件路径。
4. 备份目录路径。
5. 用户二进制文件路径。
6. 用户导出 CSV 路径。
7. 动态数组初始容量。
8. 分页大小。
9. 用户名、密码散列、盐值长度。
10. 水质参数合理范围。
11. 缺失值标记。
12. 采样时间规则。
13. 预警阈值。
14. 报告讨论段落长度限制。

### 11.2 `types.h`

功能：核心数据结构定义。

负责内容：

1. 水质参数枚举。
2. 单条水质记录结构体。
3. 动态数据集结构体。
4. 用户角色、来源、状态。
5. 操作级权限枚举 `WQOperation`。
6. 菜单入口级操作枚举。
7. 用户账户结构体。
8. 用户动态表。
9. 用户二进制文件头与记录。
10. 数据概览结构体。
11. 统计结果结构体。
12. 预警记录结构体。
13. 线性回归模型结构体。
14. 存储性能对比结构体。
15. 分析讨论结构体。
16. 系统运行上下文。

### 11.3 `auth.h` / `auth.c`

功能：登录、用户创建、账户文件读写、权限控制接口。

负责内容：

1. 用户表初始化、扩容、追加、释放。
2. 默认 `admin` / `guest` 加载。
3. 创建用户从 `users.dat` 自动加载。
4. 创建用户保存为 `users.dat`。
5. 盐值生成接口。
6. MD5+salt 密码散列接口。
7. 登录验证接口。
8. 三次登录限制接口。
9. 管理员创建用户接口。
10. 管理员导出用户 CSV 接口。
11. 角色到操作的权限判断。
12. 当前用户权限判断。
13. 主菜单选项到操作枚举映射。
14. 角色、操作枚举转中文文本。

### 11.4 `menu.h` / `menu.c`

功能：菜单交互和系统流程框架。

负责内容：

1. 主菜单显示接口。
2. 数据基础操作子菜单显示接口。
3. 数据预处理子菜单显示接口。
4. 统计分析子菜单显示接口。
5. 预测分析子菜单显示接口。
6. 备份与恢复子菜单显示接口。
7. 用户管理子菜单显示接口。
8. 主菜单入口级鉴权。
9. 子菜单具体操作级鉴权。
10. 系统初始化。
11. 系统销毁。
12. 自动加载数据。
13. 系统运行入口流程。

### 11.5 `file_io.h` / `file_io.c`

功能：文件输入输出和数据集内存管理接口。

负责内容：

1. CSV 读取。
2. CSV 写入。
3. 二进制数据读取。
4. 二进制数据写入。
5. 数据集初始化。
6. 数据集扩容。
7. 数据集追加。
8. 数据集释放。
9. 存储性能对比。

### 11.6 `preprocess.h` / `preprocess.c`

功能：数据预处理接口。

负责内容：

1. 缺失值判断。
2. 参数合理范围判断。
3. 异常值检测。
4. 异常值处理。
5. 缺失值均值逼近填充。
6. 移动平均滤波。
7. 滤波前后标准差变化分析。

### 11.7 `data_manage.h` / `data_manage.c`

功能：数据浏览、筛选、排序、修改、删除接口。

负责内容：

1. 分页浏览。
2. 按参数范围筛选。
3. 按任意参数升序/降序排序。
4. 修改指定记录参数。
5. 修改前合理范围校验。
6. 删除单条记录。
7. 按条件批量删除。
8. 修改/删除前自动备份接口调用约定。

### 11.8 `backup.h` / `backup.c`

功能：数据备份与恢复接口。

负责内容：

1. 手动备份。
2. 修改前自动备份。
3. 删除前自动备份。
4. 带时间戳或版本号的备份文件命名。
5. 列出备份文件。
6. 从备份文件恢复数据。
7. 恢复前格式校验。

### 11.9 `statistics.h` / `statistics.c`

功能：统计分析和预警分析接口。

负责内容：

1. 均值计算。
2. 最大值计算。
3. 最小值计算。
4. 标准差计算。
5. 全参数基本统计。
6. 凌晨缺氧预警。
7. 盐度突变预警。
8. 皮尔逊相关系数计算。
9. 6×6 相关系数矩阵计算。
10. 相关性结论生成约定。

### 11.10 `prediction.h` / `prediction.c`

功能：单因素线性回归预测接口。

负责内容：

1. 线性回归模型训练。
2. 单点预测。
3. 决定系数 `R²` 评估。
4. 留出法 `RMSE` 评估。
5. 气温-DO 模型。
6. 水温-DO、pH-DO、盐度-DO 等多模型比较。
7. 单因素线性回归局限性分析约定。

### 11.11 `report.h` / `report.c`

功能：报告输出接口。

负责内容：

1. 数据概览报告。
2. 统计分析报告。
3. 预警报告。
4. 预测模型报告。
5. 分析讨论报告。
6. 文本报告查看。

### 11.12 `utils.h` / `utils.c`

功能：通用辅助函数接口。

负责内容：

1. 安全读取一行输入。
2. 去除换行符。
3. 字符串转 double。
4. 参数枚举与名称转换。
5. 参数合理范围获取。
6. 时间推算。
7. 时间转字符串。
8. 清屏。
9. 暂停。

## 12. 推荐开发顺序

```text
第一阶段：基础框架
1. config.h
2. types.h
3. utils.h / utils.c
4. main.c
5. menu.h / menu.c

第二阶段：认证与权限
6. auth.h / auth.c
7. 默认账户加载
8. users.dat 二进制文件加载/保存
9. MD5+salt 密码散列
10. 创建用户
11. 操作级鉴权
12. 管理员导出用户 CSV

第三阶段：文件读取与数据结构
13. file_io.h / file_io.c
14. report.h / report.c
15. 启动时自动加载 clean_data.bin 或 data_modify.csv

第四阶段：数据清洗
16. preprocess.h / preprocess.c
17. 异常值检测与处理
18. 缺失值填充
19. 移动平均滤波
20. 保存 clean_data.csv 和 clean_data.bin
21. 生成异常值处理与滤波窗口分析讨论

第五阶段：数据维护
22. data_manage.h / data_manage.c
23. 分页浏览、筛选、排序
24. 修改记录、删除记录
25. 修改和删除前鉴权并自动备份

第六阶段：数据安全
26. backup.h / backup.c
27. 手动备份
28. 自动备份
29. 备份恢复

第七阶段：统计分析
30. statistics.h / statistics.c
31. 基本统计量
32. 凌晨缺氧预警
33. 盐度突变预警
34. 相关性分析
35. 生成相关性分析讨论

第八阶段：预测模型
36. prediction.h / prediction.c
37. 单因素线性回归
38. R²评估
39. RMSE评估
40. 多模型比较
41. 生成预测准确度分析讨论

第九阶段：系统集成
42. 主菜单整合
43. 权限测试
44. 内存释放检查
45. 报告输出检查
46. 最终测试
```

## 13. 任务书功能对应关系

| 任务书功能 / 扩展需求 | 对应代码文件 |
|---|---|
| 数据结构设计 | `types.h` |
| 全局参数配置 | `config.h` |
| CSV 读取 | `file_io.c` |
| CSV / 二进制存储 | `file_io.c` |
| 启动时自动加载数据 | `menu.c` + `file_io.c` |
| 存储性能对比 | `file_io.c` + `report.c` |
| 存储格式分析讨论 | `report.c` + `AnalysisDiscussion` |
| 分页浏览 | `data_manage.c` |
| 条件筛选 | `data_manage.c` |
| 参数排序 | `data_manage.c` |
| 数据修改 | `data_manage.c` + `backup.c` + `auth.c` |
| 数据删除 | `data_manage.c` + `backup.c` + `auth.c` |
| 数据备份与恢复 | `backup.c` + `auth.c` |
| 异常值检测 | `preprocess.c` |
| 异常值处理讨论 | `preprocess.c` + `report.c` |
| 缺失值处理 | `preprocess.c` |
| 移动平均滤波 | `preprocess.c` |
| 滤波窗口讨论 | `preprocess.c` + `report.c` |
| 基本统计量 | `statistics.c` |
| 凌晨缺氧预警 | `statistics.c` |
| 盐度突变预警 | `statistics.c` |
| 皮尔逊相关性分析 | `statistics.c` |
| 相关性分析讨论 | `statistics.c` + `report.c` |
| 线性回归预测 | `prediction.c` |
| R² / RMSE 评估 | `prediction.c` |
| 预测准确度讨论 | `prediction.c` + `report.c` |
| 登录验证 | `auth.c` |
| 用户创建 | `auth.c` |
| MD5+salt 密码散列 | `auth.c` |
| 创建用户二进制落盘 | `auth.c` |
| 启动时自动加载账户文件 | `auth.c` + `menu.c` |
| 管理员导出用户 CSV | `auth.c` |
| 菜单入口级权限控制 | `auth.c` + `menu.c` |
| 具体业务操作权限控制 | `auth.c` + `menu.c` |
| 菜单交互 | `menu.c` |
| 报告输出 | `report.c` |
| 通用输入与辅助函数 | `utils.c` |
| 程序入口 | `main.c` |

## 14. 后续实现注意事项

1. 所有动态内存申请失败时，应释放已申请资源并返回 `WQ_ERROR` 或 `NULL`。
2. CSV 读取时应跳过表头，并处理空字段、`-999`、`-9999`、`NaN`、`nan`。
3. 修改和删除数据前必须自动备份。
4. 删除操作必须二次确认。
5. 访客不能绕过菜单直接调用受限功能。
6. 报告输出不应只写数值，也应写入任务书要求的分析讨论。
7. `report.c` 只负责输出，具体结论应由对应业务模块计算或组织。
8. 创建用户密码不得保存明文。
9. 默认账户不写入 `users.dat`。
10. 程序退出前应释放数据集、用户表并清理敏感信息。
