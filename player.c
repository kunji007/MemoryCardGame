#include "player.h"
#include <stdio.h>


void InputPlayerName(Player *player)
{
    printf("请输入玩家名字:");

    scanf("%31s", player->name);
}


void ShowPlayer(Player player)
{
    printf("当前玩家:%s\n", player.name);
}