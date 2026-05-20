#include "file_io.h"
#include "preprocess.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>

typedef struct WQBinaryHeader
{
    char magic[8];
    unsigned int version;
    unsigned int record_size;
    size_t record_count;
} WQBinaryHeader;

static void wq_init_record(WaterQualityRecord *record, size_t index)
{
    size_t i;
    memset(record, 0, sizeof(*record));
    record->index = index;
    record->valid = true;
    (void)wq_parse_datetime_by_index(index, &record->time);
    for (i = 0U; i < (size_t)WQ_PARAM_COUNT; ++i)
    {
        record->missing[i] = false;
    }
}

WaterQualityDataset *wq_dataset_create(size_t initial_capacity)
{
    WaterQualityDataset *dataset = (WaterQualityDataset *)malloc(sizeof(WaterQualityDataset));
    if (dataset == NULL)
        return NULL;
    dataset->records = NULL;
    dataset->size = 0U;
    dataset->capacity = 0U;
    if (initial_capacity > 0U && wq_dataset_reserve(dataset, initial_capacity) != WQ_SUCCESS)
    {
        free(dataset);
        return NULL;
    }
    return dataset;
}

void wq_dataset_destroy(WaterQualityDataset *dataset)
{
    if (dataset == NULL)
        return;
    free(dataset->records);
    free(dataset);
}

int wq_dataset_reserve(WaterQualityDataset *dataset, size_t new_capacity)
{
    WaterQualityRecord *new_records;
    if (dataset == NULL)
        return WQ_ERROR;
    if (new_capacity <= dataset->capacity)
        return WQ_SUCCESS;
    if (new_capacity > (SIZE_MAX / sizeof(WaterQualityRecord)))
        return WQ_ERROR;

    new_records = (WaterQualityRecord *)realloc(dataset->records, new_capacity * sizeof(WaterQualityRecord));
    if (new_records == NULL)
        return WQ_ERROR;
    dataset->records = new_records;
    dataset->capacity = new_capacity;
    return WQ_SUCCESS;
}

int wq_dataset_push(WaterQualityDataset *dataset, const WaterQualityRecord *record)
{
    size_t new_capacity;
    if (dataset == NULL || record == NULL)
        return WQ_ERROR;

    if (dataset->size >= dataset->capacity)
    {
        if (dataset->capacity == 0U)
            new_capacity = WQ_INITIAL_CAPACITY;
        else
        {
            if (dataset->capacity > SIZE_MAX / 2U)
                return WQ_ERROR;
            new_capacity = dataset->capacity * 2U;
        }
        if (wq_dataset_reserve(dataset, new_capacity) != WQ_SUCCESS)
            return WQ_ERROR;
    }

    dataset->records[dataset->size] = *record;
    dataset->size++;
    return WQ_SUCCESS;
}

void wq_dataset_clear(WaterQualityDataset *dataset)
{
    if (dataset == NULL)
        return;
    dataset->size = 0U;
}

WaterQualityDataset *wq_read_csv(const char *filename, DataOverview *overview)
{
    FILE *fp;
    char line[WQ_MAX_LINE_LENGTH];
    WaterQualityDataset *dataset;
    size_t row_index = 0U;

    if (filename == NULL)
        return NULL;
    fp = fopen(filename, "r");
    if (fp == NULL)
        return NULL;

    dataset = wq_dataset_create(WQ_INITIAL_CAPACITY);
    if (dataset == NULL)
    {
        fclose(fp);
        return NULL;
    }

    if (overview != NULL)
        memset(overview, 0, sizeof(*overview));

    if (fgets(line, sizeof(line), fp) == NULL)
    {
        fclose(fp);
        return dataset;
    }

    while (fgets(line, sizeof(line), fp) != NULL)
    {
        WaterQualityRecord record;
        char *cursor = line;
        size_t col;
        wq_init_record(&record, row_index);

        for (col = 0U; col < (size_t)WQ_PARAM_COUNT; ++col)
        {
            char *comma = strchr(cursor, ',');
            char field[WQ_MAX_LINE_LENGTH];
            size_t n;

            if (comma != NULL)
            {
                n = (size_t)(comma - cursor);
                if (n >= sizeof(field))
                    n = sizeof(field) - 1U;
                memcpy(field, cursor, n);
                field[n] = '\0';
                cursor = comma + 1;
            }
            else
            {
                strncpy(field, cursor, sizeof(field) - 1U);
                field[sizeof(field) - 1U] = '\0';
                cursor += strlen(cursor);
            }

            wq_trim(field);
            if (wq_is_missing_text(field))
            {
                record.missing[col] = true;
                if (overview != NULL)
                    overview->missing_values++;
            }
            else
            {
                double value;
                if (!wq_parse_double(field, &value) || wq_is_missing_value(value))
                {
                    record.missing[col] = true;
                    if (overview != NULL)
                        overview->missing_values++;
                }
                else
                {
                    record.value[col] = value;
                }
            }
        }

        if (wq_dataset_push(dataset, &record) != WQ_SUCCESS)
        {
            wq_dataset_destroy(dataset);
            fclose(fp);
            return NULL;
        }
        row_index++;
    }

    fclose(fp);
    if (overview != NULL)
    {
        overview->total_records = dataset->size;
        overview->valid_records = dataset->size;
    }
    return dataset;
}

int wq_write_csv(const char *filename, const WaterQualityDataset *dataset)
{
    FILE *fp;
    size_t i, j;
    if (filename == NULL || dataset == NULL)
        return WQ_ERROR;

    fp = fopen(filename, "w");
    if (fp == NULL)
        return WQ_ERROR;

    if (fprintf(fp, "Temp,Salinity,pH,DO,precipitation,Air_temp\n") < 0)
    {
        fclose(fp);
        return WQ_ERROR;
    }

    for (i = 0U; i < dataset->size; ++i)
    {
        for (j = 0U; j < (size_t)WQ_PARAM_COUNT; ++j)
        {
            int rc = dataset->records[i].missing[j] ? fprintf(fp, "NaN") : fprintf(fp, "%.6f", dataset->records[i].value[j]);
            if (rc < 0)
            {
                fclose(fp);
                return WQ_ERROR;
            }
            if (j + 1U < (size_t)WQ_PARAM_COUNT && fputc(',', fp) == EOF)
            {
                fclose(fp);
                return WQ_ERROR;
            }
        }
        if (fputc('\n', fp) == EOF)
        {
            fclose(fp);
            return WQ_ERROR;
        }
    }

    if (fclose(fp) != 0)
        return WQ_ERROR;
    return WQ_SUCCESS;
}

WaterQualityDataset *wq_read_binary(const char *filename)
{
    FILE *fp;
    WQBinaryHeader header;
    WaterQualityDataset *dataset;

    if (filename == NULL)
        return NULL;
    fp = fopen(filename, "rb");
    if (fp == NULL)
        return NULL;

    if (fread(&header, sizeof(header), 1U, fp) != 1U)
    {
        fclose(fp);
        return NULL;
    }
    if (memcmp(header.magic, "WQBIN001", 8U) != 0 || header.version != 1U || header.record_size != (unsigned int)sizeof(WaterQualityRecord))
    {
        fclose(fp);
        return NULL;
    }

    dataset = wq_dataset_create(header.record_count);
    if (dataset == NULL)
    {
        fclose(fp);
        return NULL;
    }

    if (header.record_count > 0U)
    {
        if (fread(dataset->records, sizeof(WaterQualityRecord), header.record_count, fp) != header.record_count)
        {
            wq_dataset_destroy(dataset);
            fclose(fp);
            return NULL;
        }
    }
    dataset->size = header.record_count;
    fclose(fp);
    return dataset;
}

int wq_write_binary(const char *filename, const WaterQualityDataset *dataset)
{
    FILE *fp;
    WQBinaryHeader header;
    if (filename == NULL || dataset == NULL)
        return WQ_ERROR;

    fp = fopen(filename, "wb");
    if (fp == NULL)
        return WQ_ERROR;

    memcpy(header.magic, "WQBIN001", 8U);
    header.version = 1U;
    header.record_size = (unsigned int)sizeof(WaterQualityRecord);
    header.record_count = dataset->size;

    if (fwrite(&header, sizeof(header), 1U, fp) != 1U)
    {
        fclose(fp);
        return WQ_ERROR;
    }
    if (dataset->size > 0U && fwrite(dataset->records, sizeof(WaterQualityRecord), dataset->size, fp) != dataset->size)
    {
        fclose(fp);
        return WQ_ERROR;
    }

    if (fclose(fp) != 0)
        return WQ_ERROR;
    return WQ_SUCCESS;
}

int wq_read_binary_record(const char *filename, size_t index, WaterQualityRecord *record_out)
{
    FILE *fp;
    WQBinaryHeader header;
    long offset;

    if (filename == NULL || record_out == NULL)
        return WQ_ERROR;
    fp = fopen(filename, "rb");
    if (fp == NULL)
        return WQ_ERROR;

    if (fread(&header, sizeof(header), 1U, fp) != 1U)
    {
        fclose(fp);
        return WQ_ERROR;
    }
    if (memcmp(header.magic, "WQBIN001", 8U) != 0 || header.version != 1U || index >= header.record_count)
    {
        fclose(fp);
        return WQ_ERROR;
    }

    offset = (long)(sizeof(header) + index * sizeof(WaterQualityRecord));
    if (fseek(fp, offset, SEEK_SET) != 0)
    {
        fclose(fp);
        return WQ_ERROR;
    }
    if (fread(record_out, sizeof(WaterQualityRecord), 1U, fp) != 1U)
    {
        fclose(fp);
        return WQ_ERROR;
    }

    fclose(fp);
    return WQ_SUCCESS;
}

int wq_compare_storage_formats(const WaterQualityDataset *dataset,
                               const char *csv_file,
                               const char *binary_file,
                               StorageBenchmark *csv_result,
                               StorageBenchmark *binary_result)
{
    FILE *fp;
    clock_t begin, end;

    if (dataset == NULL || csv_file == NULL || binary_file == NULL || csv_result == NULL || binary_result == NULL)
    {
        return WQ_ERROR;
    }

    begin = clock();
    if (wq_write_csv(csv_file, dataset) != WQ_SUCCESS)
        return WQ_ERROR;
    end = clock();
    csv_result->write_seconds = ((double)(end - begin)) / CLOCKS_PER_SEC;

    begin = clock();
    {
        WaterQualityDataset *tmp = wq_read_csv(csv_file, NULL);
        if (tmp == NULL)
            return WQ_ERROR;
        wq_dataset_destroy(tmp);
    }
    end = clock();
    csv_result->read_seconds = ((double)(end - begin)) / CLOCKS_PER_SEC;

    fp = fopen(csv_file, "rb");
    if (fp == NULL)
        return WQ_ERROR;
    if (fseek(fp, 0L, SEEK_END) != 0)
    {
        fclose(fp);
        return WQ_ERROR;
    }
    csv_result->file_size_bytes = (unsigned long)ftell(fp);
    fclose(fp);
    csv_result->format = WQ_STORAGE_CSV;
    csv_result->human_readable = true;

    begin = clock();
    if (wq_write_binary(binary_file, dataset) != WQ_SUCCESS)
        return WQ_ERROR;
    end = clock();
    binary_result->write_seconds = ((double)(end - begin)) / CLOCKS_PER_SEC;

    begin = clock();
    {
        WaterQualityDataset *tmp = wq_read_binary(binary_file);
        if (tmp == NULL)
            return WQ_ERROR;
        wq_dataset_destroy(tmp);
    }
    end = clock();
    binary_result->read_seconds = ((double)(end - begin)) / CLOCKS_PER_SEC;

    fp = fopen(binary_file, "rb");
    if (fp == NULL)
        return WQ_ERROR;
    if (fseek(fp, 0L, SEEK_END) != 0)
    {
        fclose(fp);
        return WQ_ERROR;
    }
    binary_result->file_size_bytes = (unsigned long)ftell(fp);
    fclose(fp);
    binary_result->format = WQ_STORAGE_BINARY;
    binary_result->human_readable = false;

    return WQ_SUCCESS;
}
