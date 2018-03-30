#ifndef __DEFINES_H__
#define __DEFINES_H__

#define GAME_NAME					"BreakoutVK"
#define GAME_WIDTH					640
#define GAME_HEIGHT					480

#define RENDERER_NUM_BUFFERS		3
#define ORTHO_SIZE					5.0f

#define NUM_BLOCKS_COLUMNS			11
#define NUM_BLOCKS_ROWS				6
#define NUM_BLOCKS_MAX				( NUM_BLOCKS_ROWS * NUM_BLOCKS_COLUMNS )

#define PLAYER_MOVE_SPEED			10.0f
#define BALL_START_MOVE_SPEED		6.0f
#define BALL_MOVE_SPEED_INCREASE	0.2f

#define NUM_MAX_PLAYER_LIVES		3

#define BASE_PATH					"res/"
#define SCORES_FILE_PATH			BASE_PATH "scores.dat"

#define SCORE_NAME_LENGTH_MAX		3
#define NUM_MAX_SCORE_ENTRIES		10

#define SOUND_VOLUME				0.1f

#endif // __DEFINES_H__