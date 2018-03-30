#include "InputHandler.h"

/*
================================================================================================

	InputHandler

================================================================================================
*/

InputHandler* gInput = nullptr;

/*
========================
InputHandler::InputHandler
========================
*/
InputHandler::InputHandler() {
	mCurrentKeyState = nullptr;
	mPreviousKeyState = nullptr;

	mNumMaxKeys = 0;

	mInitialised = false;
}

/*
========================
InputHandler::~InputHandler
========================
*/
InputHandler::~InputHandler() {
	Shutdown();
}

/*
========================
InputHandler::Init
========================
*/
void InputHandler::Init() {
	if ( IsInitialised() ) {
		error( "InputHandler is already initialized. Nothing will happen this time.\n" );
		return;
	}

	printf( "------- Initialising input -------\n" );

	mCurrentKeyState = SDL_GetKeyboardState( &mNumMaxKeys );
	mPreviousKeyState = new Uint8[mNumMaxKeys];

	printf( "------- Input Initialised -------\n\n" );

	mInitialised = true;
}

/*
========================
InputHandler::Shutdown
========================
*/
void InputHandler::Shutdown() {
	printf( "------- Input shutting down -------\n" );

	delete[] mPreviousKeyState;
	mPreviousKeyState = nullptr;

	mInitialised = false;

	printf( "------- Input shutdown -------\n\n" );
}

/*
========================
InputHandler::Update
========================
*/
void InputHandler::Update() {
	// update key state
	const Uint8* keys = SDL_GetKeyboardState( nullptr );

	memcpy( mPreviousKeyState, mCurrentKeyState, mNumMaxKeys );
	SDL_PumpEvents();
	memcpy( const_cast<Uint8*>( mCurrentKeyState ), keys, mNumMaxKeys );
}