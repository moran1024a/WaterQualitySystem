#include "menu.h"

/*
 * 程序入口。
 * 当前文件只负责启动系统流程，具体登录、菜单和模块调用由 menu.c 组织。
 */
int main(void)
{
    wq_run_system();
    return 0;
}
