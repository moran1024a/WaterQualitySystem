# WaterQualitySystem

本工程是“海水养殖水质分析系统”的纯 C 工程骨架，用于先明确多文件结构、数据结构、函数接口和模块调用边界。当前版本只保留函数框架与占位返回值，不实现具体业务逻辑。

## 编译

Linux / MinGW gcc：

```bash
make gcc
```

Windows MSVC Developer Command Prompt：

```bat
nmake /f Makefile msvc
```

## 目录

- `include/`：头文件，定义结构体、枚举、宏和函数声明。
- `src/`：源文件，保留函数框架和必要占位返回值。
- `data/`：原始数据、清洗数据、账户二进制文件的默认位置。
- `output/`：数据概览、统计报告、预警报告、预测报告、用户导出 CSV 的默认位置。
- `backup/`：自动或手动备份文件的默认位置。

## 默认账户

| 用户名 | 密码 | 角色 | 来源 | 是否落盘 |
|---|---|---|---|---|
| `admin` | `123456` | 管理员 | 系统内置 | 否 |
| `guest` | `guest` | 访客 | 系统内置 | 否 |

说明：

1. 默认账户由程序启动时写入内存，不保存到 `users.dat`。
2. 后续创建的用户保存为二进制文件 `data/auth/users.dat`。
3. 创建用户的密码不保存明文，只保存 `MD5(密码 + salt)` 后的散列值和盐值。
4. 当前接口不提供修改用户信息或修改密码功能。

## 用户权限约定

| 操作类型 | 管理员 | 普通用户 | 访客 |
|---|---:|---:|---:|
| 登录系统 | 是 | 是 | 是 |
| 创建用户 | 是 | 否 | 否 |
| 导出用户列表 CSV | 是 | 否 | 否 |
| 自动加载账户文件 | 是 | 是 | 是 |
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
| 清屏/退出系统 | 是 | 是 | 是 |

鉴权原则：

1. 不只在菜单显示时限制权限。
2. 所有实际操作前都应映射到 `WQOperation`。
3. 调用业务函数前统一使用 `wq_require_permission()` 进行鉴权。
4. 子菜单内部仍需再次鉴权，避免绕过主菜单直接调用功能。

## 启动流程约定

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
    │       └── 若失败则尝试读取 data/data.csv
    │
    ├── wq_login()
    │   ├── 最多允许尝试 3 次
    │   └── 登录成功后保存 current_user
    │
    ├── 菜单循环
    │   ├── 主菜单鉴权
    │   ├── 子菜单鉴权
    │   └── 调用对应业务模块
    │
    └── wq_destroy_system_context()
        ├── 保存创建用户到 data/auth/users.dat
        ├── 释放 Dataset
        ├── 释放 UserStore
        └── 清理当前登录用户敏感信息
```

## 账户文件格式约定

文件路径：

```text
data/auth/users.dat
```

只保存创建用户，不保存默认 `admin` 和 `guest`。

```text
UserFileHeader
│
├── magic[8]      固定为 WQUSR001
├── version       当前为 1
└── count         后续用户记录数

UserFileRecord[count]
│
├── username
├── password_hash     MD5 + salt 后的十六进制散列
├── salt              16 字节盐值
├── role              用户角色
└── status            用户状态
```

对应结构体在 `types.h` 中定义：

- `UserAccount`
- `UserStore`
- `UserFileHeader`
- `UserFileRecord`

## 用户导出 CSV 约定

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
2. 默认建议不导出明文密码。
3. 可通过参数选择是否包含内置用户。
4. 创建用户导出的密码字段为加密散列，不是原始密码。

## 架构

```text
WaterQualitySystem 功能结构说明
│
├── include/
│   │
│   ├── config.h
│   │   功能：全局配置文件
│   │   负责内容：
│   │   1. 定义数据文件路径
│   │   2. 定义输出报告路径
│   │   3. 定义备份文件夹路径
│   │   4. 定义账户二进制文件路径
│   │   5. 定义用户导出 CSV 路径
│   │   6. 定义参数数量、分页大小、动态数组初始容量
│   │   7. 定义用户名、密码散列、盐值长度
│   │   8. 定义异常值合理范围
│   │   9. 定义缺失值标记
│   │   10. 定义起始时间和采样间隔
│   │
│   ├── types.h
│   │   功能：核心数据结构定义
│   │   负责内容：
│   │   1. 定义水质参数枚举
│   │   2. 定义单条水质记录结构体
│   │   3. 定义动态数据集结构体
│   │   4. 定义用户角色、用户来源、用户状态
│   │   5. 定义操作级权限枚举 WQOperation
│   │   6. 定义用户账户、用户动态表、用户文件头、用户文件记录
│   │   7. 定义统计结果、预警记录、线性回归模型
│   │   8. 定义系统运行上下文 SystemContext
│   │
│   ├── auth.h
│   │   功能：登录、用户创建、账户文件读写、权限控制接口声明
│   │   负责内容：
│   │   1. 用户表初始化、扩容、追加、释放
│   │   2. 默认 admin / guest 加载
│   │   3. 创建用户从 users.dat 自动加载
│   │   4. 创建用户保存为 users.dat
│   │   5. 盐值生成和 MD5+salt 密码散列接口
│   │   6. 登录验证和三次登录限制
│   │   7. 管理员创建用户
│   │   8. 管理员导出用户 CSV
│   │   9. 操作级权限判断
│   │
│   ├── file_io.h
│   │   功能：文件输入输出接口声明
│   │   负责内容：CSV读取、CSV写入、二进制读写、数据集动态内存释放、存储性能测试
│   │
│   ├── preprocess.h
│   │   功能：数据预处理接口声明
│   │   负责内容：异常值检测、异常值处理、缺失值处理、均值逼近填充、移动平均滤波
│   │
│   ├── data_manage.h
│   │   功能：数据查询、修改、删除接口声明
│   │   负责内容：分页浏览、范围筛选、参数排序、单条修改、单条删除、批量删除
│   │
│   ├── backup.h
│   │   功能：数据备份与恢复接口声明
│   │   负责内容：手动备份、修改前备份、删除前备份、备份列表、备份恢复
│   │
│   ├── statistics.h
│   │   功能：统计分析接口声明
│   │   负责内容：均值、最大值、最小值、标准差、凌晨缺氧预警、盐度突变预警、相关系数矩阵
│   │
│   ├── prediction.h
│   │   功能：线性回归预测接口声明
│   │   负责内容：单因素线性回归、R²评估、留出法 RMSE、多模型比较
│   │
│   ├── menu.h
│   │   功能：菜单交互接口声明
│   │   负责内容：主菜单、子菜单、菜单选项到权限操作的映射、系统初始化和销毁
│   │
│   ├── report.h
│   │   功能：报告文件输出接口声明
│   │   负责内容：数据概览、统计分析报告、预警报告、预测报告、文本报告查看
│   │
│   └── utils.h
│       功能：通用工具函数接口声明
│       负责内容：字符串处理、输入读取、参数映射、时间推算、清屏、暂停
│
├── src/
│   │
│   ├── main.c
│   │   功能：程序入口，只调用 wq_run_system()
│   │
│   ├── auth.c
│   │   功能：实现 auth.h 中的函数框架
│   │   具体约定：
│   │   1. 程序启动时初始化 UserStore
│   │   2. 加载默认 admin / guest
│   │   3. 自动读取 data/auth/users.dat
│   │   4. 支持管理员创建用户
│   │   5. 创建用户密码使用 MD5+salt 后保存
│   │   6. 退出前保存创建用户
│   │   7. 管理员可导出用户 CSV
│   │   8. 所有操作使用 WQOperation 鉴权
│   │
│   ├── menu.c
│   │   功能：实现菜单和系统流程框架
│   │   具体约定：
│   │   1. 启动时自动加载账户文件
│   │   2. 启动时自动加载数据
│   │   3. 登录后进入菜单循环
│   │   4. 每个菜单项调用前都进行权限判断
│   │   5. 子菜单项也必须再次鉴权
│   │
│   ├── file_io.c
│   │   功能：实现文件读写和数据集内存管理函数框架
│   │
│   ├── preprocess.c
│   │   功能：实现数据清洗与滤波函数框架
│   │
│   ├── data_manage.c
│   │   功能：实现数据浏览、查询、排序、修改、删除函数框架
│   │
│   ├── backup.c
│   │   功能：实现数据备份和恢复函数框架
│   │
│   ├── statistics.c
│   │   功能：实现统计分析和预警分析函数框架
│   │
│   ├── prediction.c
│   │   功能：实现单因素线性回归预测函数框架
│   │
│   ├── report.c
│   │   功能：实现报告输出函数框架
│   │
│   └── utils.c
│       功能：实现通用工具函数框架
│
├── data/
│   ├── data.csv
│   ├── auth/users.dat
│   ├── clean_data.csv
│   └── clean_data.bin
│
├── output/
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

## 推荐开发顺序

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
15. 启动时自动加载 clean_data.bin 或 data.csv

第四阶段：数据清洗
16. preprocess.h / preprocess.c
17. 异常值检测与处理
18. 缺失值填充
19. 移动平均滤波
20. 保存 clean_data.csv 和 clean_data.bin

第五阶段：数据维护
21. data_manage.h / data_manage.c
22. 分页浏览、筛选、排序
23. 修改记录、删除记录
24. 修改和删除前鉴权并自动备份

第六阶段：数据安全
25. backup.h / backup.c
26. 手动备份
27. 自动备份
28. 备份恢复

第七阶段：统计分析
29. statistics.h / statistics.c
30. 基本统计量
31. 凌晨缺氧预警
32. 盐度突变预警
33. 相关性分析

第八阶段：预测模型
34. prediction.h / prediction.c
35. 单因素线性回归
36. R²评估
37. RMSE评估
38. 多模型比较

第九阶段：系统集成
39. 主菜单整合
40. 权限测试
41. 内存释放检查
42. 最终测试
```

## 最终模块对应关系

```text
任务书功能 / 新增需求             对应代码文件
──────────────────────────────────────────────
数据结构设计                     types.h
全局参数配置                     config.h
CSV读取                          file_io.c
CSV/二进制存储                   file_io.c
启动时自动加载数据               menu.c + file_io.c
存储性能对比                     file_io.c + report.c
分页浏览                         data_manage.c
条件筛选                         data_manage.c
参数排序                         data_manage.c
数据修改                         data_manage.c + backup.c + auth.c
数据删除                         data_manage.c + backup.c + auth.c
数据备份与恢复                   backup.c + auth.c
异常值检测                       preprocess.c
缺失值处理                       preprocess.c
移动平均滤波                     preprocess.c
基本统计量                       statistics.c
凌晨缺氧预警                     statistics.c
盐度突变预警                     statistics.c
皮尔逊相关性分析                 statistics.c
线性回归预测                     prediction.c
R² / RMSE评估                    prediction.c
登录验证                         auth.c
用户创建                         auth.c
MD5+salt密码散列                 auth.c
创建用户二进制落盘               auth.c
启动时自动加载账户文件           auth.c + menu.c
管理员导出用户CSV                auth.c
操作级权限控制                   auth.c + menu.c
菜单交互                         menu.c
报告输出                         report.c
通用输入与辅助函数               utils.c
程序入口                         main.c
```
