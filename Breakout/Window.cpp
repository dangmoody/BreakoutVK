#include "Window.h"
#include "Defines.h"

/*
================================================================================================

	Window

================================================================================================
*/

Window* gWindow = nullptr;

/*
========================
Window::Window
========================
*/
Window::Window() {
	mSDLWindow = nullptr;
	
	mTitle = string();
	mWidth = 0;
	mHeight = 0;

	mInitialised = false;

#if MSTD_OS_WINDOWS
	mHinstance = nullptr;
	mHwnd = nullptr;
#endif
}

/*
========================
Window::~Window
========================
*/
Window::~Window() {
	Shutdown();
}

/*
========================
Window::Init
========================
*/
void Window::Init() {
	if ( IsInitialised() ) {
		error( "Window already initialised. Nothing will happen this time!\n" );
		return;
	}

#if MSTD_DEBUG
	timestamp_t start, end;
	float64 delta;

	printf( "------- Initialising Window -------\n" );

	start = timeNow();
#endif

	SetTitle( GAME_NAME );
	Resize( GAME_WIDTH, GAME_HEIGHT );

	u32 flags = SDL_WINDOW_ALLOW_HIGHDPI /*| SDL_WINDOW_INPUT_FOCUS | SDL_WINDOW_MOUSE_CAPTURE*/;
	mSDLWindow = SDL_CreateWindow( mTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mWidth, mHeight, flags );
	if ( !mSDLWindow ) {
		error( "Failed to create window: %s\n", SDL_GetError() );
		return;
	}

	SDL_SysWMinfo wmInfo = {};
	SDL_VERSION( &wmInfo.version );

	if ( SDL_GetWindowWMInfo( mSDLWindow, &wmInfo ) ) {
#if MSTD_OS_WINDOWS
		mHwnd = wmInfo.info.win.window;
		mHinstance = GetModuleHandle( NULL );
#endif
	} else {
		fatalError( "Window failed to query OS WM information: %s\n", SDL_GetError() );
		return;
	}

#if MSTD_DEBUG
	end = timeNow();
	delta = deltaMilliseconds( start, end );

	printf( "------- Window initialised. Time Taken: %f ms -------\n\n", delta );
#endif

	mInitialised = true;
}

/*
========================
Window::Shutdown
========================
*/
void Window::Shutdown() {
	if ( !IsInitialised() ) {
		return;
	}

	SDL_DestroyWindow( mSDLWindow );
	mSDLWindow = nullptr;

	mInitialised = false;
}