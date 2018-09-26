#include <stdio.h>  
#include <string.h>  
#include <stdlib.h>  
#include <unistd.h>

#define InodeNum    1024                                //i节点数目  
#define BlkNum      (80*1024)                           //磁盘块的数目  
#define BlkSize     1024                                //磁盘块大小为1K  
#define BlkPerNode  1024                                //每个文件包含的最大的磁盘块数目  
#define DISK        "Disk"                              //虚拟磁盘
#define BUFF        "Buff"                              //读写文件时的缓冲文件  
#define SuperBeg    0                                   //超级块的起始地址  
#define InodeBeg    sizeof(SuperBlk)                    //i节点区启示地址  
#define BlockBeg    (InodeBeg+InodeNum*sizeof(Inode))   //数据区起始地址  
#define MaxDirNum   (BlkSize/sizeof(Dir))               //每个磁盘块包含的最大目录项  
#define Directory   0  
#define File        1  


typedef struct{  
    int inode_map[InodeNum];                            //i节点位图  
    int blk_map[BlkNum];                                //磁盘块位图  
    int inode_used;                                     //已被使用的i节点数目  
    int blk_used;                                       //已被使用的磁盘块数目  
}SuperBlk;  
  
typedef struct{  
    int blk_identifier[BlkPerNode];                     //占用的磁盘块编号  
    int blk_num;                                        //占用的磁盘块数目  
    int file_size;                                      //文件的大小  
    int type;                                           //文件的类型  
    int mode;                                           //文件权限
}Inode;  
  
typedef struct{  
    char  name[30];                                     //目录名  
    short inode_num;                                    //目录对应的inode  
}Dir;  


Dir     dir_table[MaxDirNum];                           //将当前目录文件的内容都载入内存  
int     dir_num;                                        //当前目录下的的目录项数  
int     inode_num;                                      //当前目录的inode编号  
Inode   curr_inode;                                     //当前目录的inode结构  
SuperBlk    super_blk;                                  //文件系统的超级块  
FILE*   Disk;

char path[40];

int init_fs(void);                                      //初始化文件系统  
int close_fs(void);                                     //关闭文件系统  
int format_fs(void);                                    //格式化文件系统  

int open_dir(int);                                      //打开相应inode对应的目录  
int close_dir(int);                                     //保存相应inode的目录  
int show_dir(int);                                      //显示目录 
int make_dir(int,char *);                               //创建新目录 
int make_file(int,char*);                               //创建新的文件  
int del_file(int,char*,int);                            //删除子目录  
int enter_dir(int,char*);                               //进入子目录  
  
int file_write(char*);                                  //写文件  
int file_read(char*);                                   //读文件  
  
int adjust_dir(char*);                                  //删除子目录后，调整原目录，使中间无空隙  
  
int check_name(int,char*);                              //检查重命名,返回-1表示名字不存在，否则返回相应inode  
int type_check(int);                                    //确定文件的类型  
  
int free_inode(int);                                    //释放相应的inode  
int apply_inode();                                      //申请inode,返还相应的inode号，返还-1则INODE用完  
int init_dir_inode(int,int);                            //初始化新建目录的inode  
int init_file_inode(int);                               //初始化新建文件的inode  
  
int free_blk(int);                                      //释放相应的磁盘块  
int apply_blk(void);                                    //获取磁盘块  
  
void change_path(char*);                                //改变前面显示的当前目录
