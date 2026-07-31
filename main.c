#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
#include "memory_card.h"
#include "menu.h"
#include "player.h"

#define CON_WIDTH  80
#define CON_HEIGHT 30

// ========== 新增：颜色属性宏 ==========
#define GAME_COLOR_NORMAL    (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)   // 普通灰白
#define GAME_COLOR_HIGHLIGHT (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY) // 亮白高亮
#define GAME_COLOR_CARD_NUM   (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)  // 亮黄色（卡牌数字）
#define GAME_COLOR_CURSOR    (FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY) // 亮青色（光标）

// 使用宽字符 CHAR_INFO（WCHAR）支持中文、方块符号
CHAR_INFO backBuffer[CON_WIDTH * CON_HEIGHT];
COORD bufferSize = {CON_WIDTH, CON_HEIGHT};
COORD writePos = {0, 0};
SMALL_RECT writeRect = {0, 0, CON_WIDTH - 1, CON_HEIGHT - 1};

void ClearBackBuffer(void)
{
    for (int i = 0; i < CON_WIDTH * CON_HEIGHT; i++)
    {
        backBuffer[i].Char.UnicodeChar = L' ';
        backBuffer[i].Attributes = GAME_COLOR_NORMAL;
    }
}

// 修改：增加颜色参数
void BufferPrint(int x, int y, const wchar_t* str, WORD attr)
{
    int idx = y * CON_WIDTH + x;
    while (*str != L'\0')
    {
        if (idx >= CON_WIDTH * CON_HEIGHT) break;
        backBuffer[idx].Char.UnicodeChar = *str++;
        backBuffer[idx].Attributes = attr; // 使用传入的颜色属性
        idx++;
    }
}

void FlushBufferToScreen(void)
{
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    WriteConsoleOutputW(hOut, backBuffer, bufferSize, writePos, &writeRect);
}

void RenderGame(GameData *game, Player player)
{
    ClearBackBuffer();
    int line = 0;

    // 标题
    wchar_t title[64];
    swprintf(title, 64, L"===== 记忆翻牌游戏  第 %d 关 =====", game->level);
    BufferPrint(0, line++, title, GAME_COLOR_HIGHLIGHT);

    // 显示玩家名字
    wchar_t playerInfo[64];
    swprintf(playerInfo, 64, L"当前玩家：%S", player.name);
    BufferPrint(0, line++, playerInfo, GAME_COLOR_NORMAL);

    // 状态栏
    wchar_t status[128];
    swprintf(status, 128, L"进度：%d/%d对 | 步数：%d | 连击：%d | 继承时间：%ds | 最佳：%ds/%d次",
             game->clear_cnt, game->pair_num,
             game->total_steps, game->combo, game->carry_time,
             game->best_time, game->best_flip);
    BufferPrint(0, line++, status, GAME_COLOR_NORMAL);

    // 操作说明
    BufferPrint(0, line++, L"操作：W(上) S(下) A(左) D(右) 空格(翻牌) Q(退出)", GAME_COLOR_NORMAL);
    line++;

    // 棋盘渲染
    for(int r = 0; r < game->rows; r++)
    {
        int col = 0;
        for(int c = 0; c < game->cols; c++)
        {
            wchar_t temp[16];
            Card *card = &game->board[r][c];

            // 光标：亮青色
            if(r == game->cur_r && c == game->cur_c)
                wcscpy(temp, L">");
            else
                wcscpy(temp, L" ");
            BufferPrint(col, line, temp, GAME_COLOR_CURSOR);
            col += 1;

            switch(card->state)
            {
                case CARD_HIDE:
                    wcscpy(temp, L"■ ");
                    BufferPrint(col, line, temp, GAME_COLOR_NORMAL);
                    break;
                case CARD_SHOW:
                    swprintf(temp, 16, L"%d ", card->id);
                    // 使用卡牌自身的随机颜色
                    BufferPrint(col, line, temp, card->color);
                    break;
                case CARD_CLEAR:
                    wcscpy(temp, L"  ");
                    BufferPrint(col, line, temp, GAME_COLOR_NORMAL);
                    break;
            }
            col += 2;
        }
        line++;
    }

    // 结算界面
    if(game->game_win == 1)
    {
        line++;
        wchar_t buf[64];
        swprintf(buf, 64, L"翻牌次数：%d", game->flip_count);
        BufferPrint(0, line++, buf, GAME_COLOR_NORMAL);

        swprintf(buf, 64, L"配对成功：%d  |  配对失败：%d", game->match_success, game->match_fail);
        BufferPrint(0, line++, buf, GAME_COLOR_NORMAL);

        swprintf(buf, 64, L"通关用时：%d 秒", game->used_time);
        BufferPrint(0, line++, buf, GAME_COLOR_NORMAL);

        swprintf(buf, 64, L"总操作步数：%d  最高连击：%d", game->total_steps, game->max_combo);
        BufferPrint(0, line++, buf, GAME_COLOR_NORMAL);

        swprintf(buf, 64, L"最终得分：%d  评级：%c", game->score, game->grade);
        BufferPrint(0, line++, buf, GAME_COLOR_HIGHLIGHT);
        line++;

        if(game->level >= MAX_LEVEL)
            BufferPrint(0, line++, L"恭喜！全部关卡通关，你太强了！", GAME_COLOR_HIGHLIGHT);
        else if(game->grade != 'C')
            BufferPrint(0, line++, L"评级达标，即将进入下一关...", GAME_COLOR_HIGHLIGHT);
        else
        {
            BufferPrint(0, line++, L"本关挑战失败，评级未达到B级", GAME_COLOR_HIGHLIGHT);
            BufferPrint(0, line++, L"无法解锁下一关", GAME_COLOR_NORMAL);
        }
    }
    FlushBufferToScreen();
}

int main(void)
{

    

    system("chcp 65001 >nul");

    Player player;

    InputPlayerName(&player);

    ShowPlayer(player);

    // 隐藏光标
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 1;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);

    GameData game;

    if(ShowMenu(player)==0)
    {
        return 0;
    }

    Game_Init(&game,1,0);
    char key;

    while(1)
    {
        RenderGame(&game , player);

        // ========== 通关处理分支 ==========
        if(game.game_win == 1)
        {
            Sleep(1800); // 停留展示结算信息

            if(game.level >= MAX_LEVEL)
            {
                break; // 全部通关，跳出主循环
            }

                        // S/A/B级：达标，进入下一关
            if(game.grade != 'C')
            {
                // 计算本关剩余时间，作为下一关的继承时间
                int remain_time = game.total_std_time - game.used_time;
                if(remain_time < 0) remain_time = 0; // 超时不继承负时间
                // 可选优化：设置继承上限，最多继承60秒
                // if(remain_time > 60) remain_time = 60;

                ClearBackBuffer();
                wchar_t tip[64];
                swprintf(tip, 64, L"评级达标！剩余 %d 秒延续到下一关", remain_time);
                BufferPrint(0, 11, tip, GAME_COLOR_HIGHLIGHT);
                BufferPrint(0, 13, L"准备进入下一关...", GAME_COLOR_NORMAL);
                FlushBufferToScreen();
                Sleep(1500);

                Game_Init(&game, game.level + 1, remain_time);
                continue;
            }
            // C级：不达标，重玩本关
            else
            {
                while(1)
                {
                    wchar_t line_buf[64];
                    ClearBackBuffer();
                    BufferPrint(0, 8,  L"==================================", GAME_COLOR_NORMAL);

                    swprintf(line_buf, 64, L"  第 %d 关 挑战失败", game.level);
                    BufferPrint(0, 10, line_buf, GAME_COLOR_HIGHLIGHT);

                    BufferPrint(0, 12, L"  评级未达到B级，无法解锁下一关", GAME_COLOR_NORMAL);
                    BufferPrint(0, 14, L"==================================", GAME_COLOR_NORMAL);
                    BufferPrint(0, 16, L"  按 空格键 → 重玩本关", GAME_COLOR_NORMAL);
                    BufferPrint(0, 17, L"  按 Q 键   → 退出游戏", GAME_COLOR_NORMAL);
                    FlushBufferToScreen();

                    if(_kbhit())
                    {
                        char k = _getch();
                        if(k == ' ')
                        {
                            Game_Init(&game, game.level, game.carry_time); // 重玩本关，继承时间不变
                            break;
                        }
                        if(k == 'q' || k == 'Q')
                        {
                            ClearBackBuffer();
                            BufferPrint(0, 0, L"退出游戏", GAME_COLOR_NORMAL);
                            FlushBufferToScreen();
                            return 0;
                        }
                    }
                    Sleep(30);
                }
                continue;
            }
        }

        // ========== 正常游戏：按键检测（放在if外面，游戏进行时执行） ==========
        if(_kbhit())
        {
            key = _getch();
            switch(key)
            {
                case 'w': Game_MoveCursor(&game, 0); break;
                case 's': Game_MoveCursor(&game, 1); break;
                case 'a': Game_MoveCursor(&game, 2); break;
                case 'd': Game_MoveCursor(&game, 3); break;
                case ' ':
                    if(game.open_count == 2) break;
                    Game_OpenCard(&game);
                    if(game.open_count == 2)
                    {
                        RenderGame(&game, player);
                        Sleep(600);
                        Game_CheckPair(&game);
                    }
                    break;
                case 'q':
                case 'Q':
                    ClearBackBuffer();
                    BufferPrint(0, 0, L"退出游戏", GAME_COLOR_NORMAL);
                    FlushBufferToScreen();
                    return 0;
                default: break;
            }
        }
        Sleep(30);
    }

    // ========== 全部通关后，退出提示（放在循环外面） ==========
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
    ClearBackBuffer();
    BufferPrint(0, 10, L"恭喜你通关全部关卡！", GAME_COLOR_HIGHLIGHT);
    BufferPrint(0, 12, L"按下回车键退出程序", GAME_COLOR_NORMAL);
    FlushBufferToScreen();
    getchar();
    return 0;
}