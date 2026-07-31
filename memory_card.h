#ifndef _MEMORY_CARD_H
#define _MEMORY_CARD_H

#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <windows.h>
#include <time.h>
// 棋盘尺寸配置，修改这里就能切换难度
// 棋盘最大尺寸（支持最大6行7列）
#define MAX_ROWS  6
#define MAX_COLS  7
#define MAX_LEVEL 5  // 总关卡数
//总卡牌数量
#define TOTAL_CARD (ROWS * COLS)
//卡牌对数(一对两个相同数字)
#define PAIR_NUM (TOTAL_CARD / 2)

//卡牌三种状态枚举
typedef enum{
    CARD_HIDE,// 0：卡牌盖住，看不到数字
    CARD_SHOW,// 1：卡牌翻开，显示数字
    CARD_CLEAR,// 2：配对成功，消除消失
}CardState;
// 单张卡牌结构体
typedef struct {
    int id;        // 卡牌上的数字
    CardState state;   // 卡牌状态
    WORD color;  // 新增：该卡牌的专属颜色属性
} Card;

// 游戏完整数据结构体，存储整局游戏所有状态
typedef struct {
    Card board[MAX_ROWS][MAX_COLS]; // 二维数组：整个棋盘所有卡牌
    int cur_r, cur_c;         // 光标当前选中的 行、列坐标
    int first_r, first_c;     // 第一次翻开卡牌的坐标
    int open_count;           // 当前翻开卡牌数量，只能0/1/2
    int clear_cnt;            // 已经成功配对消除的对数
    int game_win;             // 游戏通关标记 0未通关 1全部配对完成
    int flip_count;  // 新增：有效翻牌次数统计
    int match_success;      // 新增：配对成功次数
    int match_fail;         // 新增：配对失败次数
    time_t start_time;      // 新增：游戏开始时间戳
    int used_time;          // 通关用时（秒）
    // ========== 新增：进阶玩法字段 ==========
    int total_steps;        // 总操作步数（移动+翻牌）
    int combo;              // 当前连击数
    int max_combo;          // 本局最高连击
    int score;              // 游戏最终得分
    char grade;             // 评级 S/A/B/C
    int best_time;          // 历史最快通关时间（秒）
    int best_flip;          // 历史最少翻牌次数
    // ========== 新增：关卡相关动态字段 ==========
    int level;      // 当前关卡
    int rows;       // 当前关卡行数
    int cols;       // 当前关卡列数
    int pair_num;   // 当前关卡总对数
    // ... 原有所有字段保持不变 ...
    int carry_time;     // 从上一关继承的奖励时间（秒）
    int base_std_time;  // 本关原始基础标准时间
    int total_std_time; // 本关加上继承时间后的总标准时间（用于评分）
} GameData;

// ==================== 对外公有接口函数声明 ====================
// 1. 游戏初始化:洗牌,重置所有游戏状态
void Game_Init(GameData *game, int level,int carry_time);

// 2. 移动光标
// dir参数：0上 1下 2左 3右
void Game_MoveCursor(GameData *game, int dir);

// 3. 翻开当前光标选中的卡牌
void Game_OpenCard(GameData *game);

// 4. 两张卡牌翻开后，判断是否配对、处理消除/盖回逻辑
void Game_CheckPair(GameData *game);

#endif