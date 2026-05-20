#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

static bool wq_is_leap_year(int year)
{
    return ((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0);
}

static int wq_days_in_month(int year, int month)
{
    static const int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month < 1 || month > 12)
        return 30;
    if (month == 2 && wq_is_leap_year(year))
        return 29;
    return days[month - 1];
}

void wq_trim(char *text)
{
    size_t len;
    size_t start = 0U;

    if (text == NULL)
        return;

    len = strlen(text);
    while (start < len && isspace((unsigned char)text[start]))
        start++;
    if (start > 0U)
    {
        memmove(text, text + start, len - start + 1U);
        len -= start;
    }

    while (len > 0U && isspace((unsigned char)text[len - 1U]))
    {
        text[len - 1U] = '\0';
        len--;
    }
}

bool wq_parse_double(const char *text, double *value_out)
{
    char buf[WQ_MAX_LINE_LENGTH];
    char *endptr;
    double value;

    if (text == NULL || value_out == NULL)
        return false;

    strncpy(buf, text, sizeof(buf) - 1U);
    buf[sizeof(buf) - 1U] = '\0';
    wq_trim(buf);
    if (buf[0] == '\0')
        return false;

    errno = 0;
    value = strtod(buf, &endptr);
    if (endptr == buf || *endptr != '\0' || errno == ERANGE)
        return false;

    *value_out = value;
    return true;
}

bool wq_parse_datetime_by_index(size_t index, WQDateTime *time_out)
{
    size_t total_minutes;
    int year = 2025;
    int month = 1;
    int day = 1;
    int hour = 12;
    int minute = 0;

    if (time_out == NULL)
        return false;

    total_minutes = index * (size_t)WQ_SAMPLE_INTERVAL_MINUTES;
    minute += (int)(total_minutes % 60U);
    hour += (int)((total_minutes / 60U) % 24U);

    day += (int)(total_minutes / (24U * 60U));
    hour += minute / 60;
    minute %= 60;
    day += hour / 24;
    hour %= 24;

    while (day > wq_days_in_month(year, month))
    {
        day -= wq_days_in_month(year, month);
        month++;
        if (month > 12)
        {
            month = 1;
            year++;
        }
    }

    time_out->year = year;
    time_out->month = month;
    time_out->day = day;
    time_out->hour = hour;
    time_out->minute = minute;
    return true;
}

int wq_compare_datetime(const WQDateTime *a, const WQDateTime *b)
{
    if (a == NULL || b == NULL)
        return 0;
    if (a->year != b->year)
        return (a->year < b->year) ? -1 : 1;
    if (a->month != b->month)
        return (a->month < b->month) ? -1 : 1;
    if (a->day != b->day)
        return (a->day < b->day) ? -1 : 1;
    if (a->hour != b->hour)
        return (a->hour < b->hour) ? -1 : 1;
    if (a->minute != b->minute)
        return (a->minute < b->minute) ? -1 : 1;
    return 0;
}

const char *wq_parameter_to_string(WQParameter parameter)
{
    switch (parameter)
    {
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
    switch (parameter)
    {
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
    switch (parameter)
    {
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
#ifdef _WIN32
    (void)system("cls");
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}

void wq_pause(void)
{
    char line[8];
    printf("按回车继续...");
    (void)fgets(line, sizeof(line), stdin);
}

int wq_read_int(const char *prompt, int *value_out)
{
    char line[64];
    char *endptr;
    long value;

    if (prompt == NULL || value_out == NULL)
        return WQ_ERROR;

    for (;;)
    {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL)
            return WQ_ERROR;
        errno = 0;
        value = strtol(line, &endptr, 10);
        while (*endptr != '\0' && isspace((unsigned char)*endptr))
            endptr++;
        if (endptr != line && *endptr == '\0' && errno != ERANGE && value >= INT_MIN && value <= INT_MAX)
        {
            *value_out = (int)value;
            return WQ_SUCCESS;
        }
        printf("输入无效，请重试。\n");
    }
}

int wq_read_double(const char *prompt, double *value_out)
{
    char line[128];
    if (prompt == NULL || value_out == NULL)
        return WQ_ERROR;
    for (;;)
    {
        printf("%s", prompt);
        if (fgets(line, sizeof(line), stdin) == NULL)
            return WQ_ERROR;
        if (wq_parse_double(line, value_out))
            return WQ_SUCCESS;
        printf("输入无效，请重试。\n");
    }
}

int wq_ensure_directory(const char *path)
{
#ifdef _WIN32
    if (path == NULL)
        return WQ_ERROR;
    if (_mkdir(path) == 0 || errno == EEXIST)
        return WQ_SUCCESS;
    return WQ_ERROR;
#else
    if (path == NULL)
        return WQ_ERROR;
    if (mkdir(path, 0755) == 0 || errno == EEXIST)
        return WQ_SUCCESS;
    return WQ_ERROR;
#endif
}
