#include "menu.h"
#include "record.h"
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>



int ShowMenu(Player player)
{
    while(1)
    {
        system("cls");

        printf("====================\n");
        printf(" 当前玩家: %s\n", player.name);
        printf("====================\n");

        printf("   记忆翻牌游戏\n");

        printf("====================\n");

        printf("1. 开始游戏\n");
        printf("2. 查看规则\n");
        printf("3. 最佳记录\n");
        printf("4. 退出游戏\n");

        printf("====================\n");

        char key = _getch();

        switch(key)
        {
            case '1':
                return 1;

            case '2':
                system("cls");
                printf("====================\n");
                printf("        游戏规则\n");
                printf("====================\n");

                printf("1. 使用方向键移动光标\n");
                printf("2. 空格键翻开卡牌\n");
                printf("3. 找到所有相同卡牌即可通关\n");
                printf("4. 越快完成，评分越高\n");
                printf("5. 连续匹配可以获得连击奖励\n");

                printf("====================\n");
                printf("按任意键返回菜单\n");
                _getch();
                break;

            case '3':
                ShowBestRecord();
                break;

            case '4':
                return 0;
        }
    }
}