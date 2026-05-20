#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*
 * utils.c
 *
 * 通用工具模块函数框架。
 */

void wq_trim(char *text)
{
    /* 待实现：去除字符串首尾空白字符。 */
    (void)text;
}

bool wq_parse_double(const char *text, double *value_out)
{
    /* 待实现：使用 strtod 解析浮点数，并检查非法字符。 */
    (void)text;
    (void)value_out;
    return false;
}

bool wq_parse_datetime_by_index(size_t index, WQDateTime *time_out)
{
    /* 待实现：从 2025-01-01 12:00 起按 5 分钟间隔推算时间。 */
    (void)index;
    (void)time_out;
    return false;
}

int wq_compare_datetime(const WQDateTime *a, const WQDateTime *b)
{
    /* 待实现：按年月日时分依次比较。 */
    (void)a;
    (void)b;
    return 0;
}

const char *wq_parameter_to_string(WQParameter parameter)
{
    switch (parameter) {
    case WQ_PARAM_TEMP:
        return "Temp";
    case WQ_PARAM_SALINITY:
        return "Salinity";
    case WQ_PARAM_PH:
        return "pH";
    case WQ_PARAM_DO:
        return "DO";
    case WQ_PARAM_PRECIPITATION:
        return "precipitation";
    case WQ_PARAM_AIR_TEMP:
        return "Air_temp";
    default:
        return "Unknown";
    }
}

double wq_parameter_min(WQParameter parameter)
{
    switch (parameter) {
    case WQ_PARAM_TEMP:
        return WQ_TEMP_MIN;
    case WQ_PARAM_SALINITY:
        return WQ_SALINITY_MIN;
    case WQ_PARAM_PH:
        return WQ_PH_MIN;
    case WQ_PARAM_DO:
        return WQ_DO_MIN;
    case WQ_PARAM_PRECIPITATION:
        return WQ_PRECIPITATION_MIN;
    case WQ_PARAM_AIR_TEMP:
        return WQ_AIR_TEMP_MIN;
    default:
        return 0.0;
    }
}

double wq_parameter_max(WQParameter parameter)
{
    switch (parameter) {
    case WQ_PARAM_TEMP:
        return WQ_TEMP_MAX;
    case WQ_PARAM_SALINITY:
        return WQ_SALINITY_MAX;
    case WQ_PARAM_PH:
        return WQ_PH_MAX;
    case WQ_PARAM_DO:
        return WQ_DO_MAX;
    case WQ_PARAM_PRECIPITATION:
        return WQ_PRECIPITATION_MAX;
    case WQ_PARAM_AIR_TEMP:
        return WQ_AIR_TEMP_MAX;
    default:
        return 0.0;
    }
}

void wq_clear_screen(void)
{
    /* 待实现：Windows 可调用 cls，Linux 可调用 clear。 */
}

void wq_pause(void)
{
    /* 待实现：提示用户按回车继续。 */
}

int wq_read_int(const char *prompt, int *value_out)
{
    /* 待实现：输出 prompt 并读取整数。 */
    (void)prompt;
    (void)value_out;
    return WQ_ERROR;
}

int wq_read_double(const char *prompt, double *value_out)
{
    /* 待实现：输出 prompt 并读取浮点数。 */
    (void)prompt;
    (void)value_out;
    return WQ_ERROR;
}
