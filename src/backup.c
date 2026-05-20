#include "backup.h"
#include "file_io.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

static int wq_backup_dir_ready(const char *backup_dir)
{
    if (backup_dir == NULL) return WQ_ERROR;
    return wq_ensure_directory(backup_dir);
}

int wq_create_backup(const WaterQualityDataset *dataset,
                     const char *backup_dir,
                     char *backup_path_out,
                     size_t path_size)
{
    char path[WQ_MAX_PATH_LENGTH];
    time_t now;
    struct tm *tm_now;
    int seq;

    if (dataset == NULL || backup_dir == NULL || backup_path_out == NULL || path_size == 0U) return WQ_ERROR;
    if (wq_backup_dir_ready(backup_dir) != WQ_SUCCESS) return WQ_ERROR;

    now = time(NULL);
    tm_now = localtime(&now);
    if (tm_now == NULL) return WQ_ERROR;

    for (seq = 0; seq < 1000; ++seq) {
        snprintf(path, sizeof(path), "%s/backup_%04d%02d%02d_%02d%02d%02d_%03d.csv",
                 backup_dir,
                 tm_now->tm_year + 1900, tm_now->tm_mon + 1, tm_now->tm_mday,
                 tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec, seq);
        {
            FILE *fp = fopen(path, "rb");
            if (fp == NULL) break;
            fclose(fp);
        }
    }

    if (wq_write_csv(path, dataset) != WQ_SUCCESS) return WQ_ERROR;
    strncpy(backup_path_out, path, path_size - 1U);
    backup_path_out[path_size - 1U] = '\0';
    return WQ_SUCCESS;
}

int wq_list_backup_files(const char *backup_dir,
                         char backup_files[][WQ_MAX_PATH_LENGTH],
                         size_t max_files,
                         size_t *count_out)
{
    size_t count = 0U;
    if (backup_dir == NULL || backup_files == NULL || count_out == NULL) return WQ_ERROR;
    *count_out = 0U;
#ifdef _WIN32
    {
        char cmd[512];
        FILE *pipe;
        if (wq_backup_dir_ready(backup_dir) != WQ_SUCCESS) return WQ_ERROR;
        snprintf(cmd, sizeof(cmd), "dir /b \"%s\\*.csv\" 2>nul", backup_dir);
        pipe = _popen(cmd, "r");
        if (pipe == NULL) return WQ_ERROR;
        while (count < max_files && fgets(backup_files[count], WQ_MAX_PATH_LENGTH, pipe) != NULL) {
            wq_trim(backup_files[count]);
            count++;
        }
        _pclose(pipe);
    }
#else
    {
        DIR *dir;
        struct dirent *ent;
        if (wq_backup_dir_ready(backup_dir) != WQ_SUCCESS) return WQ_ERROR;
        dir = opendir(backup_dir);
        if (dir == NULL) return WQ_ERROR;
        while ((ent = readdir(dir)) != NULL && count < max_files) {
            size_t n = strlen(ent->d_name);
            if (n >= 4U && strcmp(ent->d_name + n - 4U, ".csv") == 0) {
                strncpy(backup_files[count], ent->d_name, WQ_MAX_PATH_LENGTH - 1U);
                backup_files[count][WQ_MAX_PATH_LENGTH - 1U] = '\0';
                count++;
            }
        }
        closedir(dir);
    }
#endif
    *count_out = count;
    return WQ_SUCCESS;
}

int wq_validate_backup_file(const char *backup_file)
{
    FILE *fp;
    char line[WQ_MAX_LINE_LENGTH];
    int commas = 0;
    const char *expect = "Temp,Salinity,pH,DO,precipitation,Air_temp";
    if (backup_file == NULL) return WQ_ERROR;
    fp = fopen(backup_file, "r");
    if (fp == NULL) return WQ_ERROR;
    if (fgets(line, sizeof(line), fp) == NULL) { fclose(fp); return WQ_ERROR; }
    wq_trim(line);
    if (strcmp(line, expect) != 0) { fclose(fp); return WQ_ERROR; }
    if (fgets(line, sizeof(line), fp) != NULL) {
        size_t i;
        for (i = 0U; line[i] != '\0'; ++i) if (line[i] == ',') commas++;
        if (commas != 5) { fclose(fp); return WQ_ERROR; }
    }
    fclose(fp);
    return WQ_SUCCESS;
}

WaterQualityDataset *wq_restore_backup(const char *backup_file, DataOverview *overview)
{
    WaterQualityDataset *dataset;
    if (backup_file == NULL) return NULL;
    if (wq_validate_backup_file(backup_file) != WQ_SUCCESS) return NULL;
    dataset = wq_read_csv(backup_file, overview);
    if (dataset == NULL) return NULL;
    return dataset;
}
