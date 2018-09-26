#include "fsAPI.h"

int format()
{
    Disk=fopen(DISK,"r+");
    format_fs();                                        //格式化文件系统
    close_dir(0);
    fclose(Disk);
}

int cd()
{
    Disk=fopen(DISK,"r+");
    char name[30];
    scanf("%s",name);
    enter_dir(inode_num,name);
    fclose(Disk);   
    return 0;
}

int ls()
{
    Disk=fopen(DISK,"r+");
    show_dir(inode_num);                                //显示当前目录内容
    fclose(Disk);
}

int mkdir()
{
    Disk=fopen(DISK,"r+");
    char name[30];
    scanf("%s",name);
    make_dir(inode_num, name);                          //新建目录
    close_dir(inode_num);
    fclose(Disk);
}

int touch()
{
    Disk=fopen(DISK,"r+");
    char name[30];
    scanf("%s",name);
    make_file(inode_num, name);                         //新建文件
    close_dir(inode_num);
    fclose(Disk);
}

int more()
{
    Disk=fopen(DISK,"r+");
    int     inode,i,blk_num;  
    Inode   temp;  
    char    buff[BlkSize];  
    int     left;
    char name[30];

    scanf("%s",name);

    inode=check_name(inode_num,name);                   //检查文件是否重名

    fseek(Disk, InodeBeg+sizeof(Inode)*inode, SEEK_SET);  
    fread(&temp, sizeof(Inode), 1, Disk); 
    
    if(temp.blk_num==0){
        return 1;  
    }                                                   //如果源文件没有内容,则直接退出  
    left = temp.file_size;
    for(i = 0; i < temp.blk_num - 1; ++i){  
        blk_num = temp.blk_identifier[i];  

        fseek(Disk, BlockBeg+BlkSize*blk_num, SEEK_SET);  
        fread(buff, sizeof(char), BlkSize, Disk);  

        printf("%s",buff);   
        left -= BlkSize; 
    }                                                   //按块读取  
      
    
    blk_num = temp.blk_identifier[i];  
    fseek(Disk,BlockBeg+BlkSize*blk_num,SEEK_SET);  
    fread(buff, sizeof(char), left, Disk);  
    printf("%s",buff);                                   //最后一块可能未占满，所以单独处理
  
    printf("\n");
    fclose(Disk);
    return 0;  
}

int cp()
{
    Disk=fopen(DISK,"r+");
    char cur_name[30], det_path[30];
    int det_path_inode, cur_name_inode;
    int new_inode;
    int pos;
    Inode temp;

    scanf("%s %s",cur_name,det_path);

    if((cur_name_inode = check_name(inode_num, cur_name)) == -1)
    {
        printf("cp: '%s' No such file or diretory\n",cur_name);
        return 1;
    }

    if((det_path_inode = check_name(inode_num, det_path)) == -1)
    {
        printf("cp: '%s' No such diretory\n",det_path);
        return 1;
    }

    if(type_check(det_path_inode) == File)
    {
        printf("cp: '%s' is not a directory\n",det_path);
        return 1;
    }

    fseek(Disk, InodeBeg + sizeof(Inode)*cur_name_inode, SEEK_SET);
    fread(&temp, sizeof(Inode), 1, Disk); 

    enter_dir(inode_num, det_path);
    make_dir(inode_num, cur_name);
    for(pos = 0; pos < dir_num; pos ++)
        if(strcmp(dir_table[pos].name, cur_name) == 0)
            break;
    fseek(Disk,InodeBeg+sizeof(Inode)*dir_table[pos].inode_num,SEEK_SET);  
    fwrite(&temp,sizeof(Inode),1,Disk);
                                                        //分目录和文件
    if(type_check(cur_name_inode) == Directory)
    {
        enter_dir(inode_num, cur_name);
        strcpy(dir_table[1].name, det_path);
        dir_table[1].inode_num = det_path_inode;

        enter_dir(inode_num, "..");
        enter_dir(inode_num, "..");
    }
    else
    {
        enter_dir(inode_num, "..");
    }
    fclose(Disk);
}

int rm()
{
    Disk=fopen(DISK,"r+");
    char name[30];
    scanf("%s",name);
    del_file(inode_num, name, 0);                       //删除文件或目录
    close_dir(inode_num);
    fclose(Disk);
}

int mv()
{
    Disk=fopen(DISK,"r+");
    char cur_name[30], det_path[30];
    int det_path_inode, cur_name_inode;
    int pos;
    Dir temp;

    scanf("%s %s",cur_name,det_path);

    if((cur_name_inode = check_name(inode_num, cur_name)) == -1)
    {
        printf("mv: '%s' No such file or diretory\n",cur_name);
        return 1;
    }

    if((det_path_inode = check_name(inode_num, det_path)) == -1)
    {
        printf("mv: '%s' No such diretory\n",det_path);
        return 1;
    }

    if(type_check(det_path_inode) == File)
    {
        printf("mv: '%s' is not a directory\n",det_path);
        return 1;
    }
   
    for(pos = 0; pos < dir_num; pos ++)
        if(strcmp(dir_table[pos].name, cur_name) == 0)
            break;   
    strcpy(temp.name, dir_table[pos].name);
    temp.inode_num = dir_table[pos].inode_num;

    adjust_dir(cur_name);

    enter_dir(inode_num, det_path);

    strcpy(dir_table[dir_num].name, temp.name);
    dir_table[dir_num++].inode_num = temp.inode_num;
    
                                                        //分目录和文件分别处理
    if(type_check(cur_name_inode) == Directory)
    {
        enter_dir(inode_num, temp.name);
        strcpy(dir_table[1].name, det_path);
        dir_table[1].inode_num = det_path_inode;

        enter_dir(inode_num, "..");
        enter_dir(inode_num, "..");
    }
    else
    {
        enter_dir(inode_num, "..");
    }
    fclose(Disk);
    return 0;
}

int chname()
{
    Disk=fopen(DISK,"r+");
    int i;
    char name[30], newname[30];
    scanf("%s %s", name, newname);

    if(check_name(inode_num, newname) != -1)
    {
        printf("rename: '%s' : File exist\n");
        fclose(Disk);
        return 1;
    }

    for(i = 0; i < dir_num; i++)
        if(strcmp(name, dir_table[i].name) == 0)
            {
                strcpy(dir_table[i].name, newname);  
                close_dir(inode_num); 
                fclose(Disk);          
                return 0;
            }
    printf("rename: '%s' No such file or directory\n",name);
    fclose(Disk);
    return -1; 
}

int help()
{
    printf("   format                                                   格式化磁盘\n");
    printf("   cd <pathname>                                            进入子目录\n");
    printf("   ls                                                       列出当前目录下的文件和目录\n");
    printf("   mkdir <pathname>                                         新建子目录\n");
    printf("   touch <filename>                                         新建文件\n");
    printf("   more <filename>                                          显示文件内容\n");
    printf("   cp <cur_filename|cur_pathname> <pathname>                拷贝目录或文件\n");
    printf("   rm <filename|pathname>                                   删除文件或目录\n");
    printf("   mv <cur_filename|cur_pathname> <pathname>                移动文件或目录\n");
    printf("   rename <filename|pathname>                               重命名文件或目录\n");
    printf("   help                                                     显示帮助\n");
    printf("   vi <filename>                                            编辑文件\n");
    printf("   import <out_filename> <pathname>                         拷贝外部文件到内部\n");
    printf("   export <filename> <out_pathname>                         拷贝内部文件到外部\n");
    return 0;
}

int vi()
{
    Disk=fopen(DISK,"r+");
    char name[30];
    char *arg[]={"vim",BUFF,NULL};
    int status;
    scanf("%s",name);  
    if(type_check(check_name(inode_num,name))!=File){  
        printf("vim: cannot edit '%s': Not a file\n",name);  
        return 1;
    }  
  
    file_read(name);                                    //将数据从文件写入BUFF  
    if(!fork()){  
        execvp("vim",arg);  
    }  
    wait(&status);  
    file_write(name);
    fclose(Disk);
    return 0;
}

int import()
{
    Disk=fopen(DISK,"r+");
    int     inode,i;  
    int     num, blk_num;  
    FILE*   fp;  
    Inode   temp;  
    char    buff[BlkSize];  
    char    filename[30], name[30];

    scanf("%s %s",filename, name);

    fp = fopen(filename,"r");
    
    make_file(inode_num, name);

    inode = check_name(inode_num,name);
  
    fseek(Disk, InodeBeg+sizeof(Inode)*inode, SEEK_SET);  
    fread(&temp, sizeof(Inode), 1, Disk);               //从磁盘读取inode
  
    while(num = fread(buff,sizeof(char),BlkSize,fp)){  

        if((blk_num=apply_blk())==-1){  
            printf("error:  block has been used up\n");  
            break;  
        }  
 
        temp.blk_identifier[temp.blk_num++] = blk_num;  
        temp.file_size += num;  
          
        fseek(Disk, BlockBeg+BlkSize*blk_num, SEEK_SET);  
        fwrite(buff, sizeof(char), num, Disk);  
    }                                                   //按块将文件写入磁盘
   
    fseek(Disk,InodeBeg+sizeof(Inode)*inode,SEEK_SET);  
    fwrite(&temp,sizeof(Inode),1,Disk);                 //inode写回磁盘
    close_dir(inode_num);
  
    fclose(fp);
    fclose(Disk);
    return 0; 
}

int export()
{
    Disk=fopen(DISK,"r+");
    int     inode,i,blk_num;  
    Inode   temp;  
    FILE*   fp;  
    char    buff[BlkSize];  
    int     left;
    char    filename[100], name[30];

    scanf("%s %s",filename, name);

    strcat(name,filename);

    fp = fopen(name,"w");
  
    inode=check_name(inode_num,filename);  
    fseek(Disk, InodeBeg+sizeof(Inode)*inode, SEEK_SET);  
    fread(&temp, sizeof(Inode), 1, Disk);  

    if(temp.blk_num==0){
        fclose(fp);  
        return 1;  
    }                                                   //如果源文件没有内容,则直接退出  
    left = temp.file_size;
    for(i = 0; i < temp.blk_num - 1; ++i){  
        blk_num = temp.blk_identifier[i];  

        fseek(Disk, BlockBeg+BlkSize*blk_num, SEEK_SET);  
        fread(buff, sizeof(char), BlkSize, Disk);  

        fwrite(buff, sizeof(char), BlkSize, fp);   
        left -= BlkSize; 
    }                                                   //按块读取  
        
    blk_num = temp.blk_identifier[i];  
    fseek(Disk,BlockBeg+BlkSize*blk_num,SEEK_SET);  
    fread(buff, sizeof(char), left, Disk);  
    fwrite(buff, sizeof(char), left, fp);               //最后一块可能未占满，所以单独处理
  
    close_dir(inode_num);
    fclose(fp);  
    fclose(Disk);
    return 0;  
}