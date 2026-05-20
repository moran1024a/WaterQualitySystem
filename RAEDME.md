# WaterQualitySystem  

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
│   │   4. 定义参数数量
│   │   5. 定义每页显示记录数
│   │   6. 定义动态数组初始容量
│   │   7. 定义异常值合理范围
│   │   8. 定义缺失值标记
│   │   9. 定义起始时间：2025-01-01 12:00
│   │   10. 定义采样间隔：5分钟
│   │
│   │   主要内容：
│   │   - DATA_FILE
│   │   - CLEAN_CSV_FILE
│   │   - CLEAN_BIN_FILE
│   │   - OVERVIEW_FILE
│   │   - ANALYSIS_REPORT_FILE
│   │   - WARNING_REPORT_FILE
│   │   - STORAGE_COMPARE_FILE
│   │   - BACKUP_DIR
│   │   - PARAM_COUNT
│   │   - PAGE_SIZE
│   │   - INIT_CAPACITY
│   │   - TEMP_MIN / TEMP_MAX
│   │   - SALINITY_MIN / SALINITY_MAX
│   │   - PH_MIN / PH_MAX
│   │   - DO_MIN / DO_MAX
│   │   - PRECIP_MIN / PRECIP_MAX
│   │   - AIR_TEMP_MIN / AIR_TEMP_MAX
│   │
│   ├── types.h
│   │   功能：核心数据结构定义
│   │   负责内容：
│   │   1. 定义水质参数枚举
│   │   2. 定义单条水质记录结构体
│   │   3. 定义动态数据集结构体
│   │   4. 定义用户角色
│   │   5. 定义用户结构体
│   │   6. 定义统计结果结构体
│   │   7. 定义线性回归模型结构体
│   │
│   │   主要结构：
│   │   - ParamType
│   │   - WaterRecord
│   │   - Dataset
│   │   - Role
│   │   - User
│   │   - StatResult
│   │   - LinearModel
│   │
│   ├── file_io.h
│   │   功能：文件输入输出接口声明
│   │   负责内容：
│   │   1. CSV文件读取
│   │   2. CSV文件写入
│   │   3. 二进制文件写入
│   │   4. 二进制文件读取
│   │   5. 数据集动态内存释放
│   │   6. CSV与二进制存储性能测试
│   │
│   │   主要函数：
│   │   - Dataset *read_csv(const char *filename);
│   │   - int write_csv(const char *filename, const Dataset *dataset);
│   │   - int write_binary(const char *filename, const Dataset *dataset);
│   │   - Dataset *read_binary(const char *filename);
│   │   - void free_dataset(Dataset *dataset);
│   │   - int compare_storage_performance(const Dataset *dataset);
│   │
│   ├── preprocess.h
│   │   功能：数据预处理接口声明
│   │   负责内容：
│   │   1. 异常值检测
│   │   2. 异常值处理
│   │   3. 缺失值处理
│   │   4. 均值逼近法填充
│   │   5. 移动平均滤波
│   │   6. 滤波前后标准差比较
│   │
│   │   主要函数：
│   │   - int detect_abnormal_values(Dataset *dataset);
│   │   - int handle_abnormal_values(Dataset *dataset);
│   │   - int fill_missing_values(Dataset *dataset);
│   │   - double approximate_mean_fill(const Dataset *dataset, int index, ParamType param);
│   │   - int moving_average_filter(const Dataset *src, Dataset *dest, int window, ParamType param);
│   │   - int run_all_filters(const Dataset *dataset);
│   │   - double calc_filter_std_change(const Dataset *before, const Dataset *after, ParamType param);
│   │
│   ├── data_manage.h
│   │   功能：数据查询、修改、删除接口声明
│   │   负责内容：
│   │   1. 分页浏览数据
│   │   2. 下一页、上一页、跳转页
│   │   3. 按参数范围筛选
│   │   4. 按任意参数排序
│   │   5. 单条数据修改
│   │   6. 修改前参数合法性检查
│   │   7. 单条删除
│   │   8. 条件批量删除
│   │
│   │   主要函数：
│   │   - void browse_page(const Dataset *dataset, int page);
│   │   - void browse_dataset_menu(const Dataset *dataset);
│   │   - Dataset *filter_by_range(const Dataset *dataset, ParamType param, double min, double max);
│   │   - void sort_by_param(Dataset *dataset, ParamType param, int ascending);
│   │   - int modify_record(Dataset *dataset, int index, ParamType param, double new_value);
│   │   - int delete_record(Dataset *dataset, int index);
│   │   - int batch_delete_by_range(Dataset *dataset, ParamType param, double min, double max);
│   │
│   ├── backup.h
│   │   功能：数据备份与恢复接口声明
│   │   负责内容：
│   │   1. 手动备份当前数据
│   │   2. 修改前自动备份
│   │   3. 删除前自动备份
│   │   4. 生成带时间戳的备份文件名
│   │   5. 列出可用备份文件
│   │   6. 从备份文件恢复数据
│   │
│   │   主要函数：
│   │   - int backup_dataset(const Dataset *dataset);
│   │   - int auto_backup_before_modify(const Dataset *dataset);
│   │   - int auto_backup_before_delete(const Dataset *dataset);
│   │   - int list_backup_files(void);
│   │   - Dataset *restore_dataset(const char *backup_file);
│   │   - void generate_backup_filename(char *filename, int size);
│   │
│   ├── statistics.h
│   │   功能：统计分析接口声明
│   │   负责内容：
│   │   1. 均值计算
│   │   2. 最大值计算
│   │   3. 最小值计算
│   │   4. 标准差计算
│   │   5. 基本统计报告
│   │   6. 凌晨缺氧预警
│   │   7. 盐度突变预警
│   │   8. 皮尔逊相关系数
│   │   9. 6×6相关系数矩阵
│   │
│   │   主要函数：
│   │   - double calc_mean(const Dataset *dataset, ParamType param);
│   │   - double calc_max(const Dataset *dataset, ParamType param);
│   │   - double calc_min(const Dataset *dataset, ParamType param);
│   │   - double calc_std(const Dataset *dataset, ParamType param);
│   │   - StatResult calc_stat_result(const Dataset *dataset, ParamType param);
│   │   - int basic_statistics_report(const Dataset *dataset);
│   │   - int early_morning_do_warning(const Dataset *dataset);
│   │   - int salinity_change_warning(const Dataset *dataset);
│   │   - double pearson_correlation(const Dataset *dataset, ParamType x, ParamType y);
│   │   - int correlation_matrix(const Dataset *dataset, double matrix[PARAM_COUNT][PARAM_COUNT]);
│   │
│   ├── prediction.h
│   │   功能：线性回归预测接口声明
│   │   负责内容：
│   │   1. 单因素线性回归建模
│   │   2. 气温预测溶解氧
│   │   3. 水温预测溶解氧
│   │   4. pH预测溶解氧
│   │   5. 盐度预测溶解氧
│   │   6. R²模型评估
│   │   7. 训练集/测试集留出法评估
│   │   8. RMSE计算
│   │   9. 多个单因素模型比较
│   │
│   │   主要函数：
│   │   - LinearModel train_linear_model(const Dataset *dataset, ParamType x, ParamType y);
│   │   - double linear_predict(LinearModel model, double x);
│   │   - double calc_r2(const Dataset *dataset, ParamType x, ParamType y, LinearModel model);
│   │   - double calc_rmse(const Dataset *dataset, ParamType x, ParamType y, LinearModel model, int start, int end);
│   │   - int holdout_evaluate(const Dataset *dataset, ParamType x, ParamType y);
│   │   - int compare_do_models(const Dataset *dataset);
│   │
│   ├── auth.h
│   │   功能：登录与权限管理接口声明
│   │   负责内容：
│   │   1. 预设管理员账户
│   │   2. 预设访客账户
│   │   3. 用户登录验证
│   │   4. 最多三次登录尝试
│   │   5. 保存当前用户角色
│   │   6. 根据角色判断功能权限
│   │   7. 根据角色显示不同菜单
│   │
│   │   主要函数：
│   │   - int login(User *current_user);
│   │   - int check_user(const char *username, const char *password, User *current_user);
│   │   - int is_admin(const User *user);
│   │   - int is_guest(const User *user);
│   │   - int check_permission(const User *user, int menu_option);
│   │
│   ├── menu.h
│   │   功能：菜单交互接口声明
│   │   负责内容：
│   │   1. 主菜单循环
│   │   2. 数据基础操作子菜单
│   │   3. 数据预处理子菜单
│   │   4. 统计分析子菜单
│   │   5. 预测分析子菜单
│   │   6. 备份恢复子菜单
│   │   7. 根据权限显示菜单
│   │   8. 未加载数据时提示错误
│   │   9. 退出系统确认
│   │
│   │   主要函数：
│   │   - void main_menu(User current_user);
│   │   - void data_basic_menu(Dataset **dataset);
│   │   - void preprocess_menu(Dataset **dataset);
│   │   - void statistics_menu(Dataset *dataset);
│   │   - void prediction_menu(Dataset *dataset);
│   │   - void backup_menu(Dataset **dataset);
│   │   - void report_menu(void);
│   │
│   ├── report.h
│   │   功能：报告文件输出接口声明
│   │   负责内容：
│   │   1. 写入数据概览
│   │   2. 追加预处理结果
│   │   3. 写入统计分析报告
│   │   4. 写入预警报告
│   │   5. 写入存储性能对比报告
│   │   6. 查看报告文件内容
│   │
│   │   主要函数：
│   │   - int write_overview_report(const Dataset *dataset);
│   │   - int append_preprocess_report(const char *message);
│   │   - int write_analysis_report(const Dataset *dataset);
│   │   - int write_warning_report(const char *warning_time, const char *type, const char *suggestion);
│   │   - int write_storage_compare_report(long csv_size, long bin_size, double csv_write, double bin_write, double csv_read, double bin_read);
│   │   - void show_report_file(const char *filename);
│   │
│   └── utils.h
│       功能：通用工具函数接口声明
│       负责内容：
│       1. 安全输入整数
│       2. 安全输入小数
│       3. 字符串去除换行
│       4. 判断缺失值字符串
│       5. 判断参数值是否在合理范围
│       6. 获取参数名称
│       7. 根据记录序号计算时间
│       8. 清屏
│       9. 暂停等待用户输入
│
│       主要函数：
│       - int input_int(const char *prompt);
│       - double input_double(const char *prompt);
│       - void trim_newline(char *str);
│       - int is_missing_value(const char *str);
│       - int validate_param_value(ParamType param, double value);
│       - const char *get_param_name(ParamType param);
│       - void index_to_time(int index, char *time_str, int size);
│       - void clear_screen(void);
│       - void pause_screen(void);
│
├── src/
│   │
│   ├── main.c
│   │   功能：程序入口
│   │   负责内容：
│   │   1. 初始化系统
│   │   2. 调用登录模块
│   │   3. 登录成功后进入主菜单
│   │   4. 程序退出前释放内存
│   │
│   │   执行流程：
│   │   main()
│   │     ↓
│   │   login()
│   │     ↓
│   │   main_menu()
│   │     ↓
│   │   free_dataset()
│   │     ↓
│   │   exit
│   │
│   ├── file_io.c
│   │   功能：实现 file_io.h 中声明的文件操作函数
│   │   具体实现：
│   │   1. 打开CSV文件
│   │   2. 判断文件是否存在
│   │   3. 跳过第一行表头
│   │   4. 按行读取数据
│   │   5. 使用strtok或sscanf解析字段
│   │   6. 判断空值、NaN、-999、-9999
│   │   7. 使用malloc申请初始容量
│   │   8. 容量不足时使用realloc扩容
│   │   9. 读取失败时释放内存
│   │   10. 写入CSV清洗数据
│   │   11. 写入二进制清洗数据
│   │   12. 支持二进制顺序读取
│   │   13. 支持二进制随机读取
│   │   14. 统计CSV和二进制文件大小
│   │   15. 统计读写时间
│   │
│   ├── preprocess.c
│   │   功能：实现数据清洗与滤波
│   │   具体实现：
│   │   1. 遍历每条记录
│   │   2. 判断六个参数是否超出合理范围
│   │   3. 统计异常记录数
│   │   4. 统计异常参数个数
│   │   5. 若一条记录异常参数数 ≥ 3，删除整条记录
│   │   6. 若异常参数数 < 3，将异常参数标记为缺失
│   │   7. 对缺失值使用均值逼近法填充
│   │   8. 向前寻找最多10个有效值
│   │   9. 向后寻找最多10个有效值
│   │   10. 单方向存在有效值时使用单方向均值
│   │   11. 双方向都不存在时使用该参数全集均值
│   │   12. 对Temp、DO、pH、Salinity进行移动平均滤波
│   │   13. 分别测试窗口3、5、7、9、11
│   │   14. 计算滤波前后标准差变化
│   │   15. 将处理结果写入overview.txt
│   │
│   ├── data_manage.c
│   │   功能：实现用户对数据的浏览、查询、排序、修改、删除
│   │   具体实现：
│   │   1. 每页显示15条记录
│   │   2. 支持下一页
│   │   3. 支持上一页
│   │   4. 支持跳转指定页
│   │   5. 支持选择参数
│   │   6. 支持输入最小值和最大值
│   │   7. 输出符合范围条件的记录
│   │   8. 支持选择排序参数
│   │   9. 支持升序和降序排序
│   │   10. 支持按记录编号修改数据
│   │   11. 修改前调用validate_param_value()
│   │   12. 修改前调用backup_dataset()
│   │   13. 修改后询问是否保存
│   │   14. 支持按记录编号删除单条记录
│   │   15. 支持按参数范围批量删除
│   │   16. 删除前二次确认
│   │   17. 删除前自动备份
│   │
│   ├── backup.c
│   │   功能：实现数据备份和恢复
│   │   具体实现：
│   │   1. 检查backup目录是否存在
│   │   2. 不存在则创建backup目录
│   │   3. 使用当前日期或编号生成备份文件名
│   │   4. 将当前内存数据写入backup目录
│   │   5. 修改和删除前自动调用备份
│   │   6. 列出backup目录下的备份文件
│   │   7. 让用户选择恢复文件
│   │   8. 读取备份CSV
│   │   9. 验证列数和数据格式
│   │   10. 验证通过后替换当前内存数据
│   │
│   ├── statistics.c
│   │   功能：实现统计分析和预警分析
│   │   具体实现：
│   │   1. 对六个参数分别计算均值
│   │   2. 对六个参数分别计算最大值
│   │   3. 对六个参数分别计算最小值
│   │   4. 对六个参数分别计算标准差
│   │   5. 将基本统计结果写入analysis_report.txt
│   │   6. 根据记录序号计算日期时间
│   │   7. 筛选每天03:00-05:00的DO数据
│   │   8. 计算凌晨DO均值
│   │   9. DO均值 < 4.0 输出亚缺氧预警
│   │   10. DO均值 < 3.0 输出严重缺氧预警
│   │   11. 计算1小时盐度变化：当前值 - 前12条记录值
│   │   12. 判断1小时下降是否超过2 PSU
│   │   13. 判断24小时累计下降是否超过5 PSU
│   │   14. 输出盐度突变预警
│   │   15. 计算任意两个参数的皮尔逊相关系数
│   │   16. 生成6×6相关系数矩阵
│   │   17. 查找最强正相关
│   │   18. 查找最强负相关
│   │   19. 分析Temp-DO、pH-DO、Temp-Air_temp、Temp-Salinity关系
│   │
│   ├── prediction.c
│   │   功能：实现单因素线性回归预测
│   │   具体实现：
│   │   1. 输入自变量参数x
│   │   2. 输入因变量参数y，主要为DO
│   │   3. 使用最小二乘法计算斜率a
│   │   4. 使用最小二乘法计算截距b
│   │   5. 得到模型 y = a*x + b
│   │   6. 使用Air_temp预测DO
│   │   7. 使用Temp预测DO
│   │   8. 使用pH预测DO
│   │   9. 使用Salinity预测DO
│   │   10. 计算R²
│   │   11. 按前80%训练、后20%测试划分数据
│   │   12. 在训练集上训练模型
│   │   13. 在测试集上预测DO
│   │   14. 计算RMSE
│   │   15. 比较不同单因素模型效果
│   │   16. 将模型结果写入analysis_report.txt
│   │
│   ├── auth.c
│   │   功能：实现用户登录和权限判断
│   │   具体实现：
│   │   1. 定义预设用户数组
│   │   2. admin / 123456 / 管理员
│   │   3. guest / guest / 访客
│   │   4. 程序启动后输入用户名
│   │   5. 输入密码
│   │   6. 遍历用户数组匹配账户
│   │   7. 登录成功后保存当前角色
│   │   8. 登录失败提示错误
│   │   9. 最多允许尝试3次
│   │   10. 超过3次退出系统
│   │   11. 管理员允许访问所有功能
│   │   12. 访客只允许查看数据概览和统计分析报告
│   │
│   ├── menu.c
│   │   功能：实现所有菜单和子菜单
│   │   具体实现：
│   │   1. 显示系统标题
│   │   2. 根据用户角色显示不同主菜单
│   │   3. 接收用户菜单输入
│   │   4. 判断输入是否合法
│   │   5. 调用对应功能函数
│   │   6. 功能执行完成后返回菜单
│   │   7. 未加载数据时禁止预处理、统计、预测、修改、删除
│   │   8. 数据基础操作子菜单
│   │   9. 数据预处理子菜单
│   │   10. 统计分析子菜单
│   │   11. 预测分析子菜单
│   │   12. 备份恢复子菜单
│   │   13. 查看报告菜单
│   │   14. 清屏
│   │   15. 退出确认
│   │
│   ├── report.c
│   │   功能：统一实现所有报告文件输出
│   │   具体实现：
│   │   1. 创建output目录
│   │   2. 写入overview.txt
│   │   3. 写入总记录数
│   │   4. 写入有效记录数
│   │   5. 写入异常记录数
│   │   6. 写入修复异常值记录数
│   │   7. 写入删除异常值记录数
│   │   8. 写入缺失值处理个数
│   │   9. 写入analysis_report.txt
│   │   10. 写入基本统计量
│   │   11. 写入相关系数矩阵
│   │   12. 写入线性回归模型结果
│   │   13. 写入warning_report.txt
│   │   14. 写入凌晨缺氧预警
│   │   15. 写入盐度突变预警
│   │   16. 写入storage_compare.txt
│   │   17. 支持在控制台查看报告内容
│   │
│   └── utils.c
│       功能：实现通用工具函数
│       具体实现：
│       1. 安全读取整数
│       2. 安全读取浮点数
│       3. 清除输入缓冲区
│       4. 去除字符串换行符
│       5. 判断字符串是否为空值
│       6. 判断字符串是否为NaN
│       7. 判断字符串是否为-999或-9999
│       8. 判断参数值是否在合理范围
│       9. 参数编号转参数名称
│       10. 参数编号转单位
│       11. 根据记录编号计算日期时间
│       12. 判断文件是否存在
│       13. 获取文件大小
│       14. 清屏
│       15. 暂停等待用户继续
│
├── data/
│   │
│   ├── data.csv
│   │   功能：原始数据文件
│   │   内容：
│   │   1. 从传感器导出的原始CSV数据
│   │   2. 第一行为表头
│   │   3. 后续每行包含六个参数
│   │
│   ├── clean_data.csv
│   │   功能：清洗后的CSV数据
│   │   内容：
│   │   1. 异常值处理后的数据
│   │   2. 缺失值填充后的数据
│   │   3. 滤波后的数据
│   │   4. 便于人工查看
│   │
│   └── clean_data.bin
│       功能：清洗后的二进制数据
│       内容：
│       1. 与clean_data.csv内容一致
│       2. 便于快速读取
│       3. 支持随机访问
│
├── output/
│   │
│   ├── overview.txt
│   │   功能：数据概览文件
│   │   内容：
│   │   1. 总记录数
│   │   2. 有效记录数
│   │   3. 异常记录数
│   │   4. 异常数据时间跨度
│   │   5. 异常参数个数
│   │   6. 修复异常值记录数
│   │   7. 删除异常值记录数
│   │   8. 缺失值处理个数
│   │
│   ├── analysis_report.txt
│   │   功能：统计分析报告
│   │   内容：
│   │   1. 六个参数的均值
│   │   2. 六个参数的最大值
│   │   3. 六个参数的最小值
│   │   4. 六个参数的标准差
│   │   5. 6×6相关系数矩阵
│   │   6. 最强正相关
│   │   7. 最强负相关
│   │   8. 重点参数关系分析
│   │   9. 线性回归模型结果
│   │   10. R²和RMSE评估结果
│   │
│   ├── warning_report.txt
│   │   功能：预警报告
│   │   内容：
│   │   1. 预警时间
│   │   2. 预警类型
│   │   3. 预警等级
│   │   4. 触发原因
│   │   5. 处理建议
│   │
│   └── storage_compare.txt
│       功能：存储性能对比报告
│       内容：
│       1. CSV文件大小
│       2. 二进制文件大小
│       3. CSV写入时间
│       4. 二进制写入时间
│       5. CSV读取时间
│       6. 二进制读取时间
│       7. 是否人类可读
│       8. CSV和二进制适用场景分析
│
├── backup/
│   │
│   ├── backup_YYYYMMDD_HHMMSS.csv
│   │   功能：自动或手动生成的备份文件
│   │
│   └── backup_001.csv
│       功能：编号式备份文件
│
├── Makefile
│   功能：项目编译脚本
│   负责内容：
│   1. 指定编译器gcc
│   2. 指定include头文件目录
│   3. 编译src目录下所有.c文件
│   4. 生成可执行文件
│   5. 支持make clean清理中间文件
│
└── README.md
    功能：项目说明文档
    负责内容：
    1. 项目简介
    2. 功能模块说明
    3. 编译方法
    4. 运行方法
    5. 菜单功能说明
    6. 输入输出文件说明
    7. 用户账户说明
```

## 推荐的主程序调用结构

```text
main.c
│
├── 初始化 Dataset *dataset = NULL
│
├── 调用 login()
│   │
│   ├── 登录成功
│   │   └── 保存 current_user
│   │
│   └── 登录失败超过3次
│       └── 退出系统
│
├── 进入 main_menu(current_user)
│   │
│   ├── [1] 数据基础操作
│   │   ├── 读取CSV数据
│   │   ├── 保存CSV数据
│   │   ├── 保存二进制数据
│   │   ├── 分页浏览
│   │   ├── 条件筛选
│   │   ├── 参数排序
│   │   ├── 修改记录
│   │   └── 删除记录
│   │
│   ├── [2] 数据预处理
│   │   ├── 异常值检测
│   │   ├── 异常值处理
│   │   ├── 缺失值填充
│   │   ├── 移动平均滤波
│   │   └── 保存清洗数据
│   │
│   ├── [3] 统计分析
│   │   ├── 基本统计量
│   │   ├── 凌晨缺氧预警
│   │   ├── 盐度突变预警
│   │   ├── 相关性分析
│   │   └── 生成统计分析报告
│   │
│   ├── [4] 预测分析
│   │   ├── Air_temp → DO 线性回归
│   │   ├── Temp → DO 线性回归
│   │   ├── pH → DO 线性回归
│   │   ├── Salinity → DO 线性回归
│   │   ├── R²评估
│   │   ├── RMSE评估
│   │   └── 多模型比较
│   │
│   ├── [5] 查看数据概览
│   │   └── 打开 overview.txt
│   │
│   ├── [6] 查看预警报告
│   │   └── 打开 warning_report.txt
│   │
│   ├── [7] 查看分析报告
│   │   └── 打开 analysis_report.txt
│   │
│   ├── [8] 数据备份与恢复
│   │   ├── 手动备份
│   │   ├── 查看备份列表
│   │   └── 恢复备份
│   │
│   ├── [9] 清屏
│   │
│   └── [0] 退出系统
│
└── 退出前 free_dataset(dataset)
```

## 推荐开发顺序

```text
第一阶段：基础框架
1. config.h
2. types.h
3. utils.h / utils.c
4. main.c
5. menu.h / menu.c

第二阶段：文件读取与数据结构
6. file_io.h / file_io.c
7. report.h / report.c
8. 能读取data.csv并生成overview.txt

第三阶段：数据清洗
9. preprocess.h / preprocess.c
10. 异常值检测
11. 异常值处理
12. 缺失值填充
13. 移动平均滤波
14. 保存clean_data.csv和clean_data.bin

第四阶段：数据维护
15. data_manage.h / data_manage.c
16. 分页浏览
17. 条件筛选
18. 参数排序
19. 修改记录
20. 删除记录

第五阶段：数据安全
21. backup.h / backup.c
22. 自动备份
23. 手动备份
24. 备份恢复

第六阶段：统计分析
25. statistics.h / statistics.c
26. 基本统计量
27. 凌晨缺氧预警
28. 盐度突变预警
29. 相关性分析

第七阶段：预测模型
30. prediction.h / prediction.c
31. 单因素线性回归
32. R²评估
33. RMSE评估
34. 多模型比较

第八阶段：系统集成
35. auth.h / auth.c
36. 登录验证
37. 权限控制
38. 主菜单整合
39. 最终测试
```

## 最终模块对应关系

```text
任务书功能                  对应代码文件
────────────────────────────────────────────
数据结构设计                types.h
全局参数配置                config.h
CSV读取                     file_io.c
CSV/二进制存储              file_io.c
存储性能对比                file_io.c + report.c
分页浏览                    data_manage.c
条件筛选                    data_manage.c
参数排序                    data_manage.c
数据修改                    data_manage.c + backup.c
数据删除                    data_manage.c + backup.c
数据备份与恢复              backup.c
异常值检测                  preprocess.c
缺失值处理                  preprocess.c
移动平均滤波                preprocess.c
基本统计量                  statistics.c
凌晨缺氧预警                statistics.c
盐度突变预警                statistics.c
皮尔逊相关性分析            statistics.c
线性回归预测                prediction.c
R² / RMSE评估               prediction.c
登录与权限管理              auth.c
菜单交互                    menu.c
报告输出                    report.c
通用输入与辅助函数          utils.c
程序入口                    main.c
```
