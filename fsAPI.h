#include <stdio.h>
#include <string.h>  
#include <stdlib.h>
#include "FileSystem.h"

#define CmdNum      (sizeof(cmdSet)/sizeof(char *))



int format(void);                           //格式化磁盘
int cd(void);                               //切换目录
int ls(void);                               //列出当前目录内容
int mkdir(void);                            //新建目录
int touch(void);                            //新建文件
int more(void);                             //显示文件内容
int cp(void);                               //拷贝
int rm(void);                               //删除
int mv(void);                               //移动
int chname(void);                           //重命名
int help(void);                             //帮助
int vi(void);                               //编辑
int import(void);                           //从外部拷入文件
int export(void);                           //拷出文件到外部