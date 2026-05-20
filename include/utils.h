#ifndef WQ_UTILS_H
#define WQ_UTILS_H

#include <stddef.h>
#include <stdbool.h>
#include "types.h"

/*
 * utils.h
 *
 * 通用辅助函数接口。
 * 放置与具体业务模块无强绑定的字符串、输入、参数映射、时间推算等工具函数。
 */

/* 去除字符串首尾空白字符。 */
void wq_trim(char *text);

/* 将字符串解析为 double。成功返回 true。 */
bool wq_parse_double(const char *text, double *value_out);

/* 根据记录下标推算采集时间。起点为 2025-01-01 12:00。 */
bool wq_parse_datetime_by_index(size_t index, WQDateTime *time_out);

/* 比较两个时间。返回值 <0、=0、>0 分别表示 a 早于、等于、晚于 b。 */
int wq_compare_datetime(const WQDateTime *a, const WQDateTime *b);

/* 参数枚举转为显示名称。 */
const char *wq_parameter_to_string(WQParameter parameter);

/* 获取参数合理最小值。 */
double wq_parameter_min(WQParameter parameter);

/* 获取参数合理最大值。 */
double wq_parameter_max(WQParameter parameter);

/* 清屏。不同平台可在实现中分别处理。 */
void wq_clear_screen(void);

/* 暂停等待用户继续。 */
void wq_pause(void);

/* 从控制台读取整数。 */
int wq_read_int(const char *prompt, int *value_out);

/* 从控制台读取浮点数。 */
int wq_read_double(const char *prompt, double *value_out);

#endif
