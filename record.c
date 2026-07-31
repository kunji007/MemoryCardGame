#include "record.h"
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>


void ShowBestRecord()
{
    FILE *fp = fopen("best_record.dat","rb");

    int best_time;
    int best_flip;


    system("cls");


    printf("====================\n");
    printf("      最佳记录\n");
    printf("====================\n");


    if(fp == NULL)
    {
        printf("暂无记录\n");
    }
    else
    {
        fread(&best_time,sizeof(int),1,fp);
        fread(&best_flip,sizeof(int),1,fp);

        fclose(fp);


        printf("最快时间 : %d 秒\n",best_time);
        printf("最少翻牌 : %d 次\n",best_flip);
    }


    printf("====================\n");
    printf("按任意键返回\n");

    _getch();
}