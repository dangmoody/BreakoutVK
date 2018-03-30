#ifndef __INPUT_HANDLER_H__
#define __INPUT_HANDLER_H__

#include <mstd/mstd.h>

#include <SDL2/SDL.h>

/*
================================================================================================

	Breakout Input Handler

	Doesn't use SDL event system to capture key states and instead takes a snapshot of the
	entire keyboard's state every frame. Does not handle mouse/controller input.

================================================================================================
*/

class InputHandler {
public:
					InputHandler();
	virtual			~InputHandler();

	void			Init();
	void			Shutdown();
	bool32			IsInitialised() const { return mInitialised; }

	void			Update();

	inline bool32	IsKeyPressed( const u32 key ) { return mCurrentKeyState[key] && !mPreviousKeyState[key]; }
	inline bool32	IsKeyDown( const u32 key ) { return mCurrentKeyState[key] && mPreviousKeyState[key]; }

	inline bool32	IsKeyReleased( const u32 key ) { return !mCurrentKeyState[key] && mPreviousKeyState[key]; }
	inline bool32	IsKeyUp( const u32 key ) { return !mCurrentKeyState[key] && !mPreviousKeyState[key]; }

	inline bool32*	GetCurrentKeyState() { return (bool32*) mCurrentKeyState; }

	inline s32		GetNumMaxKeys() const { return mNumMaxKeys; }

private:
	const Uint8*	mCurrentKeyState;
	Uint8*			mPreviousKeyState;

	// not allowed to use size_t
	s32				mNumMaxKeys;

	bool32			mInitialised;
};

extern InputHandler* gInput;

#endif // __INPUT_HANDLER_H__