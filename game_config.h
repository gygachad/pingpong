#pragma once

#define SRV_TRESHOLD 60

#define MAIN_FIELD_X 0
#define MAIN_FIELD_Y 0
#define MAIN_FIELD_W 25
#define MAIN_FIELD_H 30

#define SHADOW_BALL_OFFSET_X 18
#define SHADOW_BALL_OFFSET_Y -25

#define MAIN_PLAYERNAME_FIELD_X MAIN_FIELD_W + 1
#define MAIN_PLAYERNAME_FIELD_Y MAIN_FIELD_H - 3
#define SHADOW_PLAYER_NAME_FIELD_X MAIN_FIELD_W + 1
#define SHADOW_PLAYER_NAME_FIELD_Y MAIN_FIELD_Y

#define READY_TIMER_X MAIN_FIELD_W + 5
#define READY_TIMER_Y MAIN_FIELD_H/2

#define BAR_LEN 5
#define MAIN_BAR_X MAIN_FIELD_X + 1
#define MAIN_BAR_Y MAIN_FIELD_H - 2
#define SHADOW_BAR_X MAIN_FIELD_W - BAR_LEN - 1
#define SHADOW_BAR_Y MAIN_FIELD_Y + 1

#define MAIN_BALL_X MAIN_FIELD_X + 3
#define MAIN_BALL_Y MAIN_FIELD_H - 3
#define SHADOW_BALL_X MAIN_FIELD_W - 4
#define SHADOW_BALL_Y MAIN_FIELD_Y + 2

#define KEY_UP 72
#define KEY_DOWN 80
#define KEY_LEFT 75
#define KEY_RIGHT 77
#define KEY_SPACEBAR 32

enum class player_state
{
	wait,
	ready,
	start,
	stop
};

enum class game_state
{
	wait,
	start,
	stop
};