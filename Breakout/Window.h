#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

#include <mstd/mstd.h>

struct SDL_Window;

/*
================================================================================================

	Yeti Window

	Uses SDL.

================================================================================================
*/

// TODO: window flags
class Window {
public:
								Window();
	virtual						~Window();

	void						Init();
	void						Shutdown();
	inline bool32				IsInitialised() const { return mInitialised; }

	inline const string&		GetTitle() const { return mTitle; }
	inline void					SetTitle( const string& title );

	inline u32					GetWidth() const { return mWidth; }
	inline u32					GetHeight() const { return mHeight; }
	inline void					Resize( const u32 width, const u32 height );

	inline SDL_Window*			GetSDLWindow() { return mSDLWindow; }
	inline const SDL_Window*	GetSDLWindow() const { return mSDLWindow; }

#if MSTD_OS_WINDOWS
	inline HINSTANCE			GetHInstance() const { return mHinstance; }
	inline HWND					GetHwnd() const { return mHwnd; }
#endif
	
private:
	SDL_Event					mEvent;

	string						mTitle;
	
	SDL_Window*					mSDLWindow;

	u32							mWidth, mHeight;

#if MSTD_OS_WINDOWS
	HINSTANCE					mHinstance;
	HWND						mHwnd;
#endif

	bool32						mInitialised;
};

/*
========================
Window::SetTitle
========================
*/
void Window::SetTitle( const string& title )  {
	mTitle = title;

	if ( mSDLWindow ) {
		SDL_SetWindowTitle( mSDLWindow, mTitle.c_str() );
	}
}

/*
========================
Window::Resize
========================
*/
void Window::Resize( const u32 width, const u32 height ) {
	assertf( width > 0, "You cannot set the window to have a width of 0!\n" );
	assertf( height > 0, "You cannot set the window to have a height of 0!\n" );

	mWidth = width;
	mHeight = height;

	if ( mSDLWindow ) {
		SDL_SetWindowSize( mSDLWindow, mWidth, mHeight );
	}
}

extern Window* gWindow;

#endif // __WINDOW_H__