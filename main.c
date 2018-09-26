#include<stdio.h>
#include"fsAPI.h"

char*   cmdSet[]={"format","cd","ls","mkdir","touch","more","cp","rm","mv","chname","help","vi","exit","import","export"};


int main(void)
{
    char cmd[30];    
    int i,quit=0,choice,status;  
  
    Disk=fopen(DISK,"r+");  
    init_fs(); 
    fclose(Disk);

    while(1){
        printf("\033[1;31m%s# \033[0m",path);  
        scanf("%s",cmd);  
        choice=-1; 

        for(i=0;i<CmdNum;++i){  
            if(strcmp(cmd,cmdSet[i])==0){  
                choice=i;  
                break;  
            }  
        }

        switch(choice){
            case 0:format();break;
            case 1:cd();break;
            case 2:ls();break;
            case 3:mkdir();break;
            case 4:touch();break;
            case 5:more();break;
            case 6:cp();break;
            case 7:rm();break;
            case 8:mv();break;
            case 9:chname();break;
            case 10:help();break;
            case 11:vi();break;
            case 12:quit = 1;break;
            case 13:import();break;
            case 14:export();break;
            default:printf("%s command not found\n",cmd);
        }
        if(quit) break;

    }
    Disk=fopen(DISK,"r+");
    close_fs();  
  
    fclose(Disk); 
    return 0;
}