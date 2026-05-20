#ifndef WQ_FILE_IO_H
#define WQ_FILE_IO_H

#include <stddef.h>
#include "types.h"

/*
 * file_io.h
 *
 * 文件读写与数据集内存管理接口。
 * 负责 CSV 读取、清洗结果保存、二进制读写、随机读取和存储性能对比。
 */

/* 创建空数据集，initial_capacity 为动态数组初始容量。失败返回 NULL。 */
WaterQualityDataset *wq_dataset_create(size_t initial_capacity);

/* 释放数据集及其内部动态数组。允许传入 NULL。 */
void wq_dataset_destroy(WaterQualityDataset *dataset);

/* 调整数据集容量。new_capacity 小于当前容量时不缩容。 */
int wq_dataset_reserve(WaterQualityDataset *dataset, size_t new_capacity);

/* 向数据集末尾追加一条记录，容量不足时自动扩容。 */
int wq_dataset_push(WaterQualityDataset *dataset, const WaterQualityRecord *record);

/* 清空数据集内容，但保留已分配容量。 */
void wq_dataset_clear(WaterQualityDataset *dataset);

/* 读取 CSV 文件，自动跳过表头，并更新数据概览。失败返回 NULL。 */
WaterQualityDataset *wq_read_csv(const char *filename, DataOverview *overview);

/* 将数据集写为 CSV 文本文件。 */
int wq_write_csv(const char *filename, const WaterQualityDataset *dataset);

/* 读取二进制数据文件。 */
WaterQualityDataset *wq_read_binary(const char *filename);

/* 将数据集写为二进制文件。 */
int wq_write_binary(const char *filename, const WaterQualityDataset *dataset);

/* 从二进制文件中随机读取第 index 条记录。 */
int wq_read_binary_record(const char *filename, size_t index, WaterQualityRecord *record_out);

/* 对比 CSV 与二进制文件的大小、写入时间、读取时间。 */
int wq_compare_storage_formats(const WaterQualityDataset *dataset,
                               const char *csv_file,
                               const char *binary_file,
                               StorageBenchmark *csv_result,
                               StorageBenchmark *binary_result);

#endif
