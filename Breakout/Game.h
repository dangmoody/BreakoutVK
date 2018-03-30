#ifndef __GAME_H__
#define __GAME_H__

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL

#include <mstd/mstd.h>
#include <SDL2/SDL.h>

// undef SDL main because we can't step debug using it
#undef main

#pragma warning( disable : 4201 )
#include <glm/glm.hpp>
#pragma warning( default : 4201 )

#include "Defines.h"

class Window;
class InputHandler;
class Renderer;
class SoundSystem;
struct audioObject_t;

class Entity;

enum gameState_t {
	GAME_STATE_WAITING,
	GAME_STATE_PLAYING,
	GAME_STATE_DIED,
	GAME_STATE_HIGH_SCORE,
};

/*
================================================================================================

	Breakout Game

	Handles main game logic. etc.

================================================================================================
*/

class Game {
public:
						Game();
						~Game();

	bool32				Init();
	void				Shutdown();

	void				Frame();

	inline bool32		IsRunning() const { return mRunning; }

	inline float32		GetDeltaTime() const { return mDeltaTime; }

private:
	static const u32	BLOCK_ROW_SCORES[];

	// not sure where else these could live
	audioObject_t*		mSoundHitPlayer;
	audioObject_t*		mSoundHitWalls;
	audioObject_t*		mSoundHitBlock;

	string				mDebugText;

	array<Entity*>		mBlocks;
	Entity*				mPlayer;
	Entity*				mBall;

	glm::vec3			mBallDirection, mPlayerDirection;
	
	float32				mBallMoveSpeed;
	float32				mBallSpeedIncrease;

	u32					mPlayerScore;
	u32					mPlayerLives;
	u32					mHitBlocks;

	SDL_Event			mEvent;

	string				mWindowTitle;
	bool32				mRunning;

	timestamp_t			mFrameStart, mFrameEnd;
	float32				mDeltaTime;

	float32				mFPSTimer;
	u32					mFrames;

	gameState_t			mCurrentState;

	bool32				mMute;
	bool32				mShowDebug;

private:
	void				StateWaiting();
	void				StatePlaying();
	void				StateDied();
	void				StateHighScore();

	void				UpdatePlayer();
	void				UpdateBall();

	void				ResetPlayerAndBall();
	void				ResetLevel();

	void				GameOver();
};

extern Game* gGame;

#endif // __GAME_H__