#ifndef WQ_BACKUP_H
#define WQ_BACKUP_H

#include <stddef.h>
#include "types.h"

/*
 * backup.h
 *
 * 数据备份与恢复接口。
 * 备份文件名建议包含时间戳或版本号，防止覆盖旧备份。
 */

/* 将当前内存数据保存到备份目录，并返回实际备份路径。 */
int wq_create_backup(const WaterQualityDataset *dataset,
                     const char *backup_dir,
                     char *backup_path_out,
                     size_t path_size);

/* 列出备份目录下可恢复的备份文件。 */
int wq_list_backup_files(const char *backup_dir,
                         char backup_files[][WQ_MAX_PATH_LENGTH],
                         size_t max_files,
                         size_t *count_out);

/* 从指定备份文件恢复数据集。失败返回 NULL。 */
WaterQualityDataset *wq_restore_backup(const char *backup_file, DataOverview *overview);

/* 验证备份文件格式是否合法。 */
int wq_validate_backup_file(const char *backup_file);

#endif
