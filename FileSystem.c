#include "FileSystem.h"

int init_fs(void)
{
    strcpy(path,"root@localhost: /");
    fseek(Disk, SuperBeg, SEEK_SET);
    fread(&super_blk, sizeof(SuperBlk), 1, Disk);       //从磁盘中读出超级块

    inode_num = 0;                                      //根目录的inode编号是0

    if(open_dir(inode_num)){
        printf("ERROR: CAN'T OPEN ROOT DIRECTORY\nPlease format the disk.\n");
        return 1;
    }

    return 0;
}

int close_fs(void)
{
    fseek(Disk, SuperBeg, SEEK_SET);
    fwrite(&super_blk, sizeof(SuperBlk), 1, Disk);      //将超级块写回磁盘

    close_dir(inode_num);
    return 0;
}

int format_fs(void)
{
    memset(super_blk.inode_map, 0, sizeof(super_blk.inode_map));
    super_blk.inode_map[0] = 1;
    super_blk.inode_used = 1;                           //格式化inode位图，保留第一个给根节点

    memset(super_blk.blk_map, 0, sizeof(super_blk.blk_map));
    super_blk.blk_map[0] = 1;
    super_blk.blk_used = 1;                             //格式化块位图，保留第一个块给根节点使用

    inode_num = 0;

    fseek(Disk, InodeBeg, SEEK_SET);
    fread(&curr_inode, sizeof(Inode), 1, Disk);         //读取根节点的inode

    curr_inode.file_size = 2 * sizeof(Dir);
    curr_inode.blk_num = 1;
    curr_inode.blk_identifier[0] = 0;
    curr_inode.type = Directory;                        //填写根目录的inode，当前大小占用了两个Dir（.和..），占用一个块

    dir_num = 2;                                        //当前目录，根目录下只有两个目录，.和..

    strcpy(dir_table[0].name, ".");
    strcpy(dir_table[1].name, "..");
    dir_table[0].inode_num = 0;
    dir_table[1].inode_num = 0;                         //当前目录，根目录下两个目录，添加到dir_table中

    strcpy(path, "root@localhost: /");

    return 0;
}

int open_dir(int inode)
{
    fseek(Disk, InodeBeg + sizeof(Inode)*inode, SEEK_SET);
    fread(&curr_inode, sizeof(Inode), 1, Disk);         //读取对应的inode

    if(curr_inode.type == File){
        printf("Not a directory\n");
        return -1;
    }                                                   //若打开的inode是文件，直接返回

    if(curr_inode.blk_num != 1){
        printf("Not initial\n");
        return 1;
    }                                                   //若打开的目录未初始化，直接返回

    dir_num = curr_inode.file_size/sizeof(Dir);
    fseek(Disk, BlockBeg + BlkSize * curr_inode.blk_identifier[0], SEEK_SET);
    fread(dir_table, sizeof(Dir), dir_num, Disk);       //从磁盘中读取目录内容

    return 0;
}

int close_dir(int inode)
{
    fseek(Disk,BlockBeg + BlkSize * curr_inode.blk_identifier[0], SEEK_SET);
    fwrite(dir_table,sizeof(Dir), dir_num, Disk);       //目录内容写回磁盘

    curr_inode.file_size=dir_num*sizeof(Dir);  
    fseek(Disk,InodeBeg+inode*sizeof(Inode),SEEK_SET);  
    fwrite(&curr_inode,sizeof(curr_inode),1,Disk);      //inode写回磁盘

    return 0;
}

int make_dir(int inode, char * name)
{
    int new_node;

    if(dir_num == MaxDirNum){
        printf("mkdir: cannot create directory '%s' :Directory full\n", name);
        return -1;
    }                                                   //检查目录是否已满
    if(check_name(inode,name)!=-1){
        printf("mkdir: cannot create directory '%s' :File exist\n", name);
        return 1;
    }                                                   //检查目录是否重名

    if(super_blk.blk_used == BlkNum){
        printf("mkdir: cannot create file '%s' :Block used up\n", name);
        return 2;
    }                                                   //检查数据块是否用完

    new_node = apply_inode();                           //申请inode

    if(new_node==-1){
        printf("mkdir: cannot create file '%s' :Inode used up\n", name);
        return 3;
    }

    init_dir_inode(new_node, inode);                    //初始化目录inode

    strcpy(dir_table[dir_num].name, name);
    dir_table[dir_num++].inode_num = new_node;          //将新建立的目录信息写入当前目录内容

    return 0;
}

int make_file(int inode, char * name)
{
    int new_node;
    
    if(dir_num == MaxDirNum){
        printf("mk: cannot create file '%s' :Directory full\n", name);
        return -1;
    }                                                   //检查目录是否已满
    if(check_name(inode,name)!=-1){
        printf("mk: cannot create file '%s' :File exist\n", name);
        return 1;
    }                                                   //检查是否重名

    if(super_blk.blk_used == BlkNum){
        printf("mk: cannot create file '%s' :Block used up\n", name);
        return 2;
    }                                                   //检查数据块是否用完

    new_node = apply_inode();                           //申请inode

    if(new_node==-1){
        printf("mk: cannot create file '%s' :Inode used up\n", name);
        return 3;
    }

    init_file_inode(new_node);                          //初始化文件inode

    strcpy(dir_table[dir_num].name, name);
    dir_table[dir_num++].inode_num = new_node;          //将新建立的目录信息写入当前目录内容

    return 0;
}

int show_dir(int inode)
{
    int i;
    int color = 32;
    open_dir(inode);
    for(i = 0; i < dir_num; i++){
        if(type_check(dir_table[i].inode_num) == Directory){
            printf("\033[1;%dm%s\t\033[0m", color, dir_table[i].name);
        }                                               //目录用不同颜色显示
        else{
            printf("%s\t", dir_table[i].name);
        }
        if(!((i+1)%5)) printf("\n");
    }
    printf("\n");

    return 0;
}

int del_file(int inode, char * name, int deepth)
{
    int child, i, t;
    Inode temp;

    if(!strcmp(name,".")||!strcmp(name,"..")){   
        printf("rm: failed to remove '%s': Invalid argument\n",name);  
        return 1;  
    }                                                   //检查是否为.或..

    if((child = check_name(inode, name))==-1){
        printf("rm: failed to remove '%s': No such file or directory\n", name);
        return -1;
    }                                                   //要删除的目录或文件不存在

    fseek(Disk, InodeBeg+sizeof(Inode)*child, SEEK_SET);
    fread(&temp, sizeof(Inode), 1, Disk);               //读取要删除目录或文件的inode

    if(temp.type == File){
        free_inode(child);
        if(deepth == 0){
            adjust_dir(name);
        }
        return 0;
    }                                                   //如果是文件，直接释放inode记录的数据块
    else{
        enter_dir(inode,name);
    }                                                   //如果是目录，进入

    for(i=2;i<dir_num;++i){  
        del_file(child,dir_table[i].name,deepth+1);  
    }                                                   //递归调用删除子目录内容

    enter_dir(child, "..");
    free_inode(child);

    if(deepth == 0){
        adjust_dir(name);
    }                                                   //删除当前目录下的空目录

    return 0;
}

int enter_dir(int inode, char *name)
{
    int child;
    child = check_name(inode, name);                    //查找该目录，返回其inode编号

    if(child == -1){
        printf("cd: %s: No such directory\n", name);
        return 1;
    }                                                   //检查是否存在

    close_dir(inode);
    inode_num = child;
    if(open_dir(child) != 0){
        open_dir(inode);
        inode_num = inode;
        return -1;
    }                                                   //检查是否成功打开 
    change_path(name);                    
    return 0;
}

int adjust_dir(char * name)
{
    int pos;
    for(pos = 0; pos < dir_num; pos ++)
        if(strcmp(dir_table[pos].name, name) == 0)
            break;                                      //找到要删除的目录在dir_table中的位置

    for(pos = pos + 1; pos < dir_num; pos ++)
        dir_table[pos-1] = dir_table[pos];              //该位置之后的内容向前移动一个Dir大小

    dir_num--;
    return 0;
}

int check_name(int inode, char* name)
{
    int i;
    for(i = 0; i < dir_num; i++)
        if(strcmp(name, dir_table[i].name) == 0)
            return dir_table[i].inode_num;
    return -1;                                          //检查名字是否重复使用，若重复，返回已使用该名字的inode编号
}

int type_check(int inode)
{
    Inode temp;
    fseek(Disk, InodeBeg+sizeof(Inode)*inode, SEEK_SET);  
    fread(&temp, sizeof(Inode), 1, Disk); 

    return temp.type;                                   //根据inode编号从磁盘读出inode，并返回inode中的type
}

int free_inode(int inode)
{
    Inode temp;
    int i;
    fseek(Disk, InodeBeg+sizeof(Inode)*inode, SEEK_SET);  
    fread(&temp, sizeof(Inode), 1, Disk);                  //从磁盘读出对应inode的内容 

    for(i=0;i<temp.blk_num;++i){  
        free_blk(temp.blk_identifier[i]);  
    }                                                   //释放inode中记录的占用数据块
  
    super_blk.inode_map[inode]=0;  
    super_blk.inode_used--;                             //在超级块中改变inode位图

    return 0;
}

int apply_inode()
{
    int i;
    if(super_blk.inode_used>=InodeNum){  
        return -1;                                      //inode节点用完  
    }  
  
    super_blk.inode_used++;  
  
    for(i = 1;i < InodeNum; ++i){  
        if(!super_blk.inode_map[i]){                    //找到一个空的inode,返回其编号
            super_blk.inode_map[i] = 1;  
            return i;  
        }  
    }
}

int init_dir_inode(int child, int father)
{
    Inode temp;
    Dir dot[2];
    int blk_pos;

    fseek(Disk, InodeBeg+sizeof(Inode)*child, SEEK_SET);  
    fread(&temp, sizeof(Inode), 1, Disk);               //读出对应位置的inode

    blk_pos = apply_blk();                              //申请空的数据块

    temp.blk_num = 1;
    temp.blk_identifier[0] = blk_pos;
    temp.type = Directory;
    temp.file_size = 2*sizeof(Dir);

    fseek(Disk, InodeBeg+sizeof(Inode)*child, SEEK_SET);  
    fwrite(&temp, sizeof(Inode), 1, Disk);              //将inode写回磁盘

    strcpy(dot[0].name,".");  
    dot[0].inode_num=child;                             //设置.目录
      
    strcpy(dot[1].name,"..");  
    dot[1].inode_num=father;                            //设置..目录

    fseek(Disk, BlockBeg+BlkSize*blk_pos, SEEK_SET);  
    fwrite(dot, sizeof(Dir), 2, Disk);                  //将目录内容写入数据块

    return 0;
}

int init_file_inode(int inode)
{
    Inode temp;  

    fseek(Disk,InodeBeg+sizeof(Inode)*inode,SEEK_SET);  
    fread(&temp,sizeof(Inode),1,Disk);                  //读出inode  
  
    temp.blk_num = 0;  
    temp.type = File;  
    temp.file_size = 0;
    temp.blk_identifier[0] =  apply_blk();
      
    fseek(Disk,InodeBeg+sizeof(Inode)*inode,SEEK_SET);  
    fwrite(&temp,sizeof(Inode),1,Disk);                 //inode写回磁盘
  
    return 0;
}

int free_blk(int blk_pos)
{
    super_blk.blk_used--;  
    super_blk.blk_map[blk_pos] = 0;                     //更新超级块中对应的位图
}

int apply_blk()
{
    int i;
    super_blk.blk_used++;  
    for(i=0;i<BlkNum;++i){ 
        if(!super_blk.blk_map[i]){  
            super_blk.blk_map[i]=1;  
            return i;  
        }  
    }                                                   //申请成功返回块号
  
    return -1;                                          //申请失败返回-1
}

void change_path(char *name)
{
    int pos;
    if(strcmp(name, ".") == 0){
        return ;
    }                                                   //进入当前目录
    else if(strcmp(name, "..") == 0){
        pos=strlen(path)-2;  
        for(;pos>=0;--pos) {  
            if(path[pos] == ':')
                break;
            else if(path[pos]=='/') {  
                path[pos+1]='\0';  
                break;  
            }  
        }
    }                                                   //进入上一级目录，如果是根目录，上级目录是自己
    else{
        strcat(path,name);
        strcat(path,"/");
    }                                                   //进入子目录
    return ;
}

int file_read(char* name)  
{  
    int     inode,i,blk_num;  
    Inode   temp;  
    FILE*   fp=fopen(BUFF,"w+");  
    char    buff[BlkSize];  
    int     left;
  
    inode=check_name(inode_num,name);  
    fseek(fp, 0, SEEK_SET);
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
  
    fclose(fp);  
    return 0;  
}  

int file_write(char* name)  
{  
    int     inode,i;  
    int     num, blk_num;  
    FILE*   fp=fopen(BUFF,"r");  
    Inode   temp;  
    char    buff[BlkSize];  
      
    inode = check_name(inode_num,name);  
  
    fseek(Disk, InodeBeg+sizeof(Inode)*inode, SEEK_SET);  
    fread(&temp, sizeof(Inode), 1, Disk);               //从磁盘读取inode

    while(num = fread(buff,sizeof(char),BlkSize,fp)){  

        printf("File size: %d Byte\n",num);

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
  
    fclose(fp);
    return 0;  
} 
