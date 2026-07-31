#ifndef PLAYER_H
#define PLAYER_H


#define NAME_LENGTH 32


typedef struct
{
    char name[NAME_LENGTH];

}Player;


void InputPlayerName(Player *player);

void ShowPlayer(Player player);


#endif