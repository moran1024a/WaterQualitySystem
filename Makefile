PROJECT = WaterQualitySystem
SRCS = src/main.c src/file_io.c src/preprocess.c src/data_manage.c src/backup.c src/statistics.c src/prediction.c src/auth.c src/menu.c src/report.c src/utils.c

all: gcc

# Linux / MinGW gcc
# -std=c11：使用 C11 标准
# -Wall -Wextra -pedantic：打开常见警告，便于后续完善实现
gcc:
	gcc -std=c11 -Wall -Wextra -pedantic -Iinclude $(SRCS) -o $(PROJECT)

# Windows MSVC
# /TC：强制按 C 文件编译
# /utf-8：保证中文注释、中文字符串在新版 MSVC 下处理稳定
msvc:
	cl /nologo /W4 /TC /utf-8 /Iinclude src\main.c src\file_io.c src\preprocess.c src\data_manage.c src\backup.c src\statistics.c src\prediction.c src\auth.c src\menu.c src\report.c src\utils.c /Fe:$(PROJECT).exe

clean:
	-rm -f $(PROJECT) $(PROJECT).exe *.o *.obj
	-del /Q $(PROJECT).exe *.obj 2>NUL

.PHONY: all gcc msvc clean
