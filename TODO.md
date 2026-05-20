# TODO.md

## 0. 当前结论

当前工程是可继续开发的 C 语言项目骨架：目录划分、头文件接口、核心结构体、权限枚举、报告接口和 README 说明整体合理，已覆盖任务书中的数据基础操作、数据预处理、统计分析、预测模型、登录权限、菜单集成、备份恢复等主要模块。

当前代码可通过 `make gcc` 编译检查，但多数业务函数仍为占位实现，程序尚不能作为完整成品运行。未发现会立即破坏数据的严重结构性 bug；后续开发重点是按接口补齐真实逻辑，并在修改/删除/恢复等操作前强制备份和确认。

---

## 1. 总体开发原则

1. 保持现有多文件结构：`include/*.h` 放声明和数据结构，`src/*.c` 放实现。
2. 尽量不破坏已有公开接口；确需新增辅助函数时优先放到对应模块内部或 `utils` 模块。
3. 使用 C11 标准，保证 `make gcc` 可编译；尽量兼容 Windows/MSVC。
4. 所有动态内存申请失败必须释放已申请资源并返回 `WQ_ERROR` 或 `NULL`。
5. 所有文件打开失败、格式错误、写入失败都要有错误返回，不能静默成功。
6. 修改、删除、恢复数据前必须做权限校验、用户确认和备份。
7. 报告不仅输出数值，还要包含任务书要求的分析讨论。
8. 访客不能通过隐藏菜单以外的路径绕过权限；具体业务执行前也必须调用 `wq_require_permission()`。

---

## 2. 第一阶段：补齐工具函数与基础可运行闭环

### 2.1 `src/utils.c`

- [ ] 实现 `wq_trim()`：去除首尾空白、换行、制表符。
- [ ] 实现 `wq_parse_double()`：基于 `strtod`，检查非法字符、空字符串、溢出。
- [ ] 实现 `wq_parse_datetime_by_index()`：从 `2025-01-01 12:00` 起，按 5 分钟间隔推算时间。
- [ ] 实现 `wq_compare_datetime()`：按年月日时分比较。
- [ ] 实现 `wq_clear_screen()`：Windows 使用 `cls`，其他平台使用 `clear` 或 ANSI 控制。
- [ ] 实现 `wq_pause()`。
- [ ] 实现 `wq_read_int()`、`wq_read_double()`：循环读取一行并解析，处理非法输入。

验收：工具函数单独可用，工程无警告编译。

---

## 3. 第二阶段：CSV/二进制文件与数据集管理

### 3.1 `src/file_io.c`

- [ ] 完善 `wq_dataset_reserve()`：加入容量乘法溢出检查。
- [ ] 实现 `wq_read_csv()`：
  - 自动跳过表头。
  - 按 6 列解析：`Temp, Salinity, pH, DO, precipitation, Air_temp`。
  - 支持空字段、`NaN/nan`、`-999`、`-9999` 作为缺失值。
  - 使用动态数组，初始容量 `WQ_INITIAL_CAPACITY`，满后翻倍扩容。
  - 为每条记录写入 `index` 和推算时间。
  - 统计 `overview.total_records`、`overview.valid_records`、`overview.missing_values`。
  - 任一步失败时释放数据集并返回 `NULL`。
- [ ] 实现 `wq_write_csv()`：输出表头和清洗后的 6 个参数。
- [ ] 实现 `wq_write_binary()`：写入文件头、记录数量、`WaterQualityRecord` 数组。
- [ ] 实现 `wq_read_binary()`：校验文件头和记录数量后读取。
- [ ] 实现 `wq_read_binary_record()`：用 `fseek` 随机读取指定记录。
- [ ] 实现 `wq_compare_storage_formats()`：统计 CSV/二进制文件大小、写入时间、读取时间、人类可读性。

建议新增二进制数据文件头结构，包含 magic、version、record_size、record_count，避免直接读取错误格式文件。

验收：能读取 `data/data_modify.csv` 的 23203 条数据；能写出 `output/clean_data.csv` 和 `output/clean_data.bin`；二进制随机读取正常。

---

## 4. 第三阶段：报告输出

### 4.1 `src/report.c`

- [ ] 实现 `wq_write_overview_report()`：输出总记录数、有效记录数、缺失值、异常值、修复数、删除数、异常时间跨度等。
- [ ] 实现 `wq_write_statistics_report()`：输出 6 个参数的均值、最大值、最小值、标准差、6×6 相关矩阵和相关性结论。
- [ ] 实现 `wq_write_warning_report()`：输出预警时间、预警种类、说明、处理建议。
- [ ] 实现 `wq_write_prediction_report()`：输出模型参数、R²、RMSE、多模型比较结果。
- [ ] 实现 `wq_write_discussion_report()`：支持覆盖或追加写入“分析讨论”。
- [ ] 实现 `wq_view_text_report()`：逐行输出文本报告。

验收：所有报告文件均能正常创建，路径不存在时能创建目录或返回明确错误。

---

## 5. 第四阶段：认证、用户与权限

### 5.1 `src/auth.c`

- [ ] 实现 `wq_generate_salt()`：生成 16 字节盐，至少结合时间、进程内随机数；可在程序启动时调用 `srand()`。
- [ ] 实现 `wq_md5_with_salt()`：计算 `MD5(password + salt)`，输出 32 位十六进制字符串。
- [ ] 实现 `wq_secure_hash_equal()`：常量时间比较。
- [ ] 实现默认账户验证：`admin/123456`、`guest/guest`。
- [ ] 实现 `wq_verify_login()`：区分内置账户和创建账户。
- [ ] 实现 `wq_login()`：最多 3 次登录，失败退出。
- [ ] 实现 `wq_create_user()`：
  - 校验用户名长度、密码长度、重复用户名。
  - 只允许管理员通过菜单调用。
  - 创建用户保存用户名、角色、salt、password_hash。
  - 不保存明文密码。
- [ ] 实现 `wq_load_user_store()`：读取 `data/auth/users.dat`，校验 magic、version、count。
- [ ] 实现 `wq_save_created_users()`：只保存 `WQ_USER_SOURCE_CREATED` 用户；无创建用户时也应成功写入空表或直接成功返回。
- [ ] 实现 `wq_export_users_to_csv()`：导出用户名、角色、来源、状态、salt、密码散列，不导出明文。

注意：当前 `wq_auth_shutdown()` 会调用 `wq_save_created_users()`；保存函数实现后，正常退出不应返回错误。

验收：默认账户可登录；创建用户可落盘；重启后可加载；访客无法访问受限操作。

---

## 6. 第五阶段：数据预处理

### 6.1 `src/preprocess.c`

- [ ] 实现 `wq_is_missing_text()`：识别空字符串、空白字符串、`NaN`、`nan`。
- [ ] 完善 `wq_is_missing_value()`：加入 `isnan(value)` 判断。
- [ ] 实现 `wq_detect_outliers()`：统计异常记录数、异常参数个数、异常时间跨度。
- [ ] 实现 `wq_process_outliers()`：
  - 单条记录异常参数数 `>= 3`：删除整条记录。
  - 单条记录异常参数数 `< 3`：将异常参数标记为缺失，后续填充。
  - 更新修复异常值记录数、删除异常值记录数。
- [ ] 实现 `wq_fill_missing_values()`：均值逼近法，默认前后各 10 个有效值；边界无值时使用另一侧；两侧均无值时使用全集均值。
- [ ] 实现 `wq_moving_average_filter()`：支持窗口 3、5、7、9、11；边界处使用实际可取到的窗口范围。
- [ ] 实现 `wq_filter_main_parameters()`：对 Temp、Salinity、pH、DO 滤波，保留其他参数。
- [ ] 实现 `wq_preprocess_dataset()`：串联异常值处理、缺失值填充、必要报告统计和清洗数据保存。

验收：清洗后数据能保存；概览报告包含缺失值、异常值、修复/删除统计；滤波窗口分析能写入报告讨论。

---

## 7. 第六阶段：数据浏览、筛选、排序、修改、删除

### 7.1 `src/data_manage.c`

- [ ] 实现 `wq_print_record()`：表格输出编号、时间和 6 个参数。
- [ ] 实现 `wq_print_page()`：每页默认 15 条，支持页码边界处理。
- [ ] 实现 `wq_filter_by_range()`：结果复制到目标数据集。
- [ ] 实现 `wq_sort_dataset()`：按任意参数升序/降序排序。
- [ ] 完善 `wq_modify_record_value()`：检查参数枚举合法性，避免越界。
- [ ] 实现 `wq_delete_record()`：单条删除，后续元素前移，更新 size。
- [ ] 实现 `wq_delete_records_by_range()`：批量删除。

菜单层必须补充：修改/删除前自动备份，删除前二次确认，修改后询问是否保存。

验收：分页、筛选、排序、修改、删除均能在加载数据后正常工作；未加载数据时提示错误。

---

## 8. 第七阶段：备份与恢复

### 8.1 `src/backup.c`

- [ ] 实现 `wq_create_backup()`：创建备份目录，生成时间戳文件名，如 `backup_YYYYMMDD_HHMMSS.csv`。
- [ ] 实现 `wq_list_backup_files()`：列出备份目录下可恢复文件。
- [ ] 实现 `wq_validate_backup_file()`：校验文件存在、表头、列数、数据格式。
- [ ] 实现 `wq_restore_backup()`：验证通过后读取数据并恢复到内存。

验收：手动备份、修改前备份、删除前备份、从备份恢复均可用；恢复前需要确认。

---

## 9. 第八阶段：统计分析与预警

### 9.1 `src/statistics.c`

- [ ] 实现 `wq_compute_mean()`：忽略缺失值和无效记录。
- [ ] 实现 `wq_compute_stddev()`。
- [ ] 实现 `wq_compute_parameter_statistics()`。
- [ ] 实现 `wq_pearson_correlation()`：处理样本数不足、零方差情况。
- [ ] 实现 `wq_compute_correlation_matrix()`：输出完整 6×6 矩阵。
- [ ] 实现 `wq_compute_all_statistics()`。
- [ ] 实现 `wq_analyze_dawn_hypoxia()`：每天 03:00-05:00 的 DO 均值，低于 4.0 亚缺氧，低于 3.0 严重缺氧。
- [ ] 实现 `wq_analyze_salinity_mutation()`：1 小时下降超过 2 PSU 或 24 小时累计下降超过 5 PSU 时预警。
- [ ] 实现 `wq_analyze_segment_warnings()`。

报告中必须写入最强正相关、最强负相关，以及水温-DO、pH-DO、水温-气温、水温-盐度的分析讨论。

验收：能生成统计报告和预警报告。

---

## 10. 第九阶段：预测模型

### 10.1 `src/prediction.c`

- [ ] 实现 `wq_train_linear_regression()`：最小二乘法计算 `slope`、`intercept`。
- [ ] 实现 `wq_evaluate_r_squared()`。
- [ ] 实现 `wq_evaluate_holdout_rmse()`：前 80% 训练，后 20% 测试。
- [ ] 实现 `wq_compare_do_single_factor_models()`：比较 Air_temp、Temp、pH、Salinity 对 DO 的预测效果。
- [ ] 报告中输出每个模型的 R²、RMSE，并给出哪个单因素影响更明显。

验收：能生成预测报告，并包含预测准确度与单因素线性回归局限分析。

---

## 11. 第十阶段：菜单集成

### 11.1 `src/menu.c`

- [ ] 实现所有 `wq_show_*_menu()`：按角色动态显示可用菜单。
- [ ] 实现 `wq_handle_main_menu()`：入口级鉴权、数据加载检查、分发到子菜单。
- [ ] 为数据基础操作、预处理、统计、预测、备份、用户管理分别实现子菜单循环。
- [ ] 每个具体业务调用前再次调用 `wq_require_permission()`。
- [ ] 未加载数据时，所有依赖数据的功能要提示错误，不得崩溃。
- [ ] 退出系统前确认。
- [ ] `wq_run_system()` 中补齐登录后的主菜单循环。

验收：管理员能看到全部功能；访客只能查看数据概览和统计分析相关内容；非法输入能重新提示。

---

## 12. 最终检查清单

- [ ] `make gcc` 无错误，尽量无警告。
- [ ] 程序启动能登录。
- [ ] 默认 `admin/123456`、`guest/guest` 可用。
- [ ] 数据能自动加载；加载失败不崩溃。
- [ ] CSV 读取、清洗、保存、二进制读写可用。
- [ ] 数据概览、统计报告、预警报告、预测报告均能生成。
- [ ] 修改/删除/恢复前有权限校验、确认和备份。
- [ ] 所有动态内存退出前释放。
- [ ] 任务书要求的分析讨论均有输出。
- [ ] README 中的功能说明与实际实现保持一致。
