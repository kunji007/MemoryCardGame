#include "memory_card.h"
#include <conio.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

// 关卡配置表：每行对应一关的 行数、列数
static const struct {
    int rows;
    int cols;
    int std_time;   // 本关标准通关时间（秒）
} g_level_config[MAX_LEVEL] = {
    {4, 4,45},   // 第1关：4×4 = 16张 = 8对（入门）
    {4, 5,65},   // 第2关：4×5 = 20张 = 10对
    {4, 6,100},   // 第3关：4×6 = 24张 = 12对
    {5, 6,130},   // 第4关：5×6 = 30张 = 15对
    {6, 6,170}    // 第5关：6×6 = 36张 = 18对（困难）
};

// 卡牌随机颜色池（7种高亮色，黑底高对比度）
static const WORD g_card_colors[] = {
    FOREGROUND_RED | FOREGROUND_INTENSITY,                    // 亮红
    FOREGROUND_GREEN | FOREGROUND_INTENSITY,                  // 亮绿
    FOREGROUND_BLUE | FOREGROUND_INTENSITY,                   // 亮蓝
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, // 亮黄
    FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,// 亮青
    FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,  // 亮品红
    FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY // 亮白
};
#define COLOR_COUNT (sizeof(g_card_colors) / sizeof(g_card_colors[0]))

// 内部私有洗牌函数
static void ShuffleArray(int arr[], int len)
{
    for(int i = len - 1; i > 0; i--)
    {
        int rand_pos = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[rand_pos];
        arr[rand_pos] = temp;
    }
}

// 读取历史最佳记录，不存在文件则初始化默认值
static void LoadBestRecord(GameData *game)
{
    FILE *fp = fopen("best_record.dat", "rb");
    if(fp == NULL)
    {
        // 第一次玩，无记录，设为极大值
        game->best_time = 9999;
        game->best_flip = 9999;
        return;
    }
    fread(&game->best_time, sizeof(int), 1, fp);
    fread(&game->best_flip, sizeof(int), 1, fp);
    fclose(fp);
}

// 保存最佳记录到本地文件
static void SaveBestRecord(GameData *game)
{
    FILE *fp = fopen("best_record.dat", "wb");
    if(fp == NULL) return;
    fwrite(&game->best_time, sizeof(int), 1, fp);
    fwrite(&game->best_flip, sizeof(int), 1, fp);
    fclose(fp);
}

// 计算游戏评分与等级
static void CalcScore(GameData *game)
{
    int perfect_flip = game->pair_num * 2;

    // 1. 翻牌准确度得分（满分600，核心权重）
    float flip_rate = (float)perfect_flip / game->flip_count;
    if(flip_rate > 1.0f) flip_rate = 1.0f;
    int flip_score = (int)(600 * flip_rate);

    // 2. 时间效率得分（满分400，使用含继承时间的总基准时间）
    float time_rate = (float)game->total_std_time / game->used_time;
    if(time_rate > 1.0f) time_rate = 1.0f;
    int time_score = (int)(400 * time_rate);

    // 3. 连击奖励（最高100分）
    int combo_bonus = game->max_combo * 10;
    if(combo_bonus > 100) combo_bonus = 100;

    // 计算总分
    int total = flip_score + time_score + combo_bonus;
    if(total < 0) total = 0;
    game->score = total;

    // 等级评定
    if(total >= 850)
        game->grade = 'S';
    else if(total >= 750)
        game->grade = 'A';
    else if(total >= 600)
        game->grade = 'B';
    else
        game->grade = 'C';
}

void Game_Init(GameData *game, int level,int carry_time)
{
    // 关卡合法性校验
    if(level < 1 || level > MAX_LEVEL) level = 1;
    game->level = level;
    game->rows = g_level_config[level - 1].rows;
    game->cols = g_level_config[level - 1].cols;
    game->pair_num = (game->rows * game->cols) / 2;

    // 时间继承逻辑
    game->carry_time = carry_time;
    game->base_std_time = g_level_config[level - 1].std_time;
    game->total_std_time = game->base_std_time + carry_time;
    if(carry_time < 0) game->total_std_time = game->base_std_time; // 防御：负时间不生效

    // 生成配对卡牌ID
    int ids[MAX_ROWS * MAX_COLS];
    for(int i = 0; i < game->pair_num; i++)
    {
        ids[i * 2] = i + 1;
        ids[i * 2 + 1] = i + 1;
    }

    // 洗牌
    srand((unsigned int)time(NULL));
    ShuffleArray(ids, game->rows * game->cols);

    // 填充棋盘
    int idx = 0;
    for(int r = 0; r < game->rows; r++)
    {
        for(int c = 0; c < game->cols; c++)
        {
            int card_id = ids[idx++];
            game->board[r][c].id = card_id;
            game->board[r][c].state = CARD_HIDE;
            // 同ID卡牌使用同一种颜色：用id取模得到颜色索引
            game->board[r][c].color = g_card_colors[card_id % COLOR_COUNT];
        }
    }

    // 初始化游戏状态
    game->cur_r = 0;
    game->cur_c = 0;
    game->first_r = -1;
    game->first_c = -1;
    game->open_count = 0;
    game->clear_cnt = 0;
    game->game_win = 0;
    game->flip_count = 0;        // 你之前加过的翻牌计数

    game->match_success = 0;     // 配对成功次数初始化为0
    game->match_fail = 0;        // 配对失败次数初始化为0
    game->start_time = time(NULL); // 记录游戏开始的时间戳
    game->used_time = 0;         // 通关用时初始化为0

    // ========== 新增：进阶字段初始化 ==========
    game->total_steps = 0;
    game->combo = 0;
    game->max_combo = 0;
    game->score = 0;
    game->grade = 0;
    LoadBestRecord(game); // 加载历史最佳记录
}

void Game_MoveCursor(GameData *game, int dir)
{
    int r = game->cur_r;
    int c = game->cur_c;
    int moved = 0;

    switch(dir)
    {
        case 0: r--; break;
        case 1: r++; break;
        case 2: c--; break;
        case 3: c++; break;
        default: break;
    }

    if(r >= 0 && r < game->rows)
    {
        if(r != game->cur_r) moved = 1;
        game->cur_r = r;
    }
    if(c >= 0 && c < game->cols)
    {
        if(c != game->cur_c) moved = 1;
        game->cur_c = c;
    }

    if(moved) game->total_steps++;
}

void Game_OpenCard(GameData *game)
{
    Card *now_card = &game->board[game->cur_r][game->cur_c];

    if(now_card->state != CARD_HIDE || game->open_count >= 2)
    {
        return;
    }

    now_card->state = CARD_SHOW;

    if(game->open_count == 0)
    {
        game->first_r = game->cur_r;
        game->first_c = game->cur_c;
    }

    game->open_count++;
    game->flip_count++;  // 新增：有效翻牌，次数+1
    game->total_steps++; // 新增：翻牌也算一次操作步数
}

void Game_CheckPair(GameData *game)
{
    if(game->open_count != 2)
    {
        return;
    }

    Card *card1 = &game->board[game->first_r][game->first_c];
    Card *card2 = &game->board[game->cur_r][game->cur_c];

    if(card1->id == card2->id)
    {
        card1->state = CARD_CLEAR;
        card2->state = CARD_CLEAR;
        game->clear_cnt++;
        game->match_success++;

        // ========== 新增：配对成功，连击+1，更新最高连击 ==========
        game->combo++;
        if(game->combo > game->max_combo)
        {
            game->max_combo = game->combo;
        }

        if(game->clear_cnt >= game->pair_num)
        {
            game->game_win = 1;
            game->used_time = (int)(time(NULL) - game->start_time);

            // ========== 新增：通关计算评分 ==========
            CalcScore(game);

            // ========== 新增：更新历史最佳记录 ==========
            if(game->used_time < game->best_time)
            {
                game->best_time = game->used_time;
            }
            if(game->flip_count < game->best_flip)
            {
                game->best_flip = game->flip_count;
            }
            SaveBestRecord(game); // 保存到本地文件
        }
    }
    else
    {
        card1->state = CARD_HIDE;
        card2->state = CARD_HIDE;
        game->match_fail++;

        // ========== 新增：配对失败，连击清零 ==========
        game->combo = 0;
    }

    game->open_count = 0;
    game->first_r = -1;
    game->first_c = -1;
}