#include "Game.h"

// SDL moans what main define gets used between debug/release builds, which is very annoying
// so I've done this to get around the issue, though not sure what the real problem is
// something to do with Subsystem: Windows in release build config
#define main SDL_main

int main( int argc, char** argv ) {
	UNUSED( argc );
	UNUSED( argv );

	gGame = new Game();

	bool32 result = gGame->Init();
	if ( !result ) {
		fatalError( "Game failed to initialise!\n" );
		return EXIT_FAILURE;
	}

	while ( gGame->IsRunning() ) {
		gGame->Frame();
	}

	delete gGame;
	gGame = nullptr;

	return 0;
}