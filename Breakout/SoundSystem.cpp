#include "SoundSystem.h"

/*
================================================================================================

	Sound System

================================================================================================
*/

SoundSystem* gSoundSystem = nullptr;

/*
========================
SoundSystem::SoundSystem
========================
*/
SoundSystem::SoundSystem() {
	mSystem = nullptr;

	mInitialised = false;
}

/*
========================
SoundSystem::~SoundSystem
========================
*/
SoundSystem::~SoundSystem() {
	Shutdown();
}

/*
========================
SoundSystem::Init
========================
*/
void SoundSystem::Init() {
	if ( IsInitialised() ) {
		error( "Attempt to call SoundSystem::Init() when already initialised! Nothing will happen this time!\n" );
		return;
	}

	printf( "------- Initialising Sound System -------\n" );

	// TODO: better error printing
	FMOD_RESULT result = FMOD_System_Create( &mSystem );
	if ( result != FMOD_OK ) {
		error( "Unable to create FMOD sound system instance: %d\n", result );
		return;
	}

	result = FMOD_System_Init( mSystem, NUM_MAX_CHANNELS, FMOD_INIT_NORMAL, nullptr );
	if ( result != FMOD_OK ) {
		error( "Unable to init FMOD: %d\n", result );
		return;
	}

	result = FMOD_System_CreateChannelGroup( mSystem, "MainChannel", &mMainChannel );
	if ( result != FMOD_OK ) {
		error( "Unable to init FMOD main channel: %d\n", result );
		return;
	}

	printf( "------- Sound System initialised -------\n\n" );

	mInitialised = true;
}

/*
========================
SoundSystem::Shutdown
========================
*/
void SoundSystem::Shutdown() {
	if ( !IsInitialised() ) {
		error( "Attempt to call SoundSystem::Shutdown() was made when it's not initialized!\n" );
		return;
	}

	printf( "------- Sound System shutting down -------\n" );

	FMOD_RESULT result = FMOD_ChannelGroup_Release( mMainChannel );
	if ( result != FMOD_OK ) {
		error( "Unable to destroy main FMOD channel group: %d\n", result );
		return;
	}
	mMainChannel = nullptr;

	result = FMOD_System_Close( mSystem );
	if ( result != FMOD_OK ) {
		error( "Unable to close FMOD sound system: %d\n", result );
		return;
	}

	result = FMOD_System_Release( mSystem );
	if ( result != FMOD_OK ) {
		error( "Unable to release FMOD sound system: %d\n", result );
		return;
	}
	mSystem = nullptr;

	mInitialised = false;

	printf( "------- Sound System shutdown -------\n" );
}

/*
========================
SoundSystem::Update
========================
*/
void SoundSystem::Update() {
	FMOD_System_Update( mSystem );
}

/*
========================
SoundSystem::CreateAudioObject
========================
*/
audioObject_t* SoundSystem::CreateAudioObject( const string& filename ) {
	assertf( !filename.empty(), "SoundSystem::CreateAudioObject() failed because specified file name is empty.\n" );

	audioObject_t* audioObject = new audioObject_t();

	FMOD_RESULT result = FMOD_System_CreateSound( mSystem, filename.c_str(), FMOD_CREATESAMPLE, nullptr, &audioObject->mSound );
	if ( result != FMOD_OK ) {
		error( "Failed to create sound: %d\n", result );
		return nullptr;
	}

	return audioObject;
}

/*
========================
SoundSystem::DestroyAudioObject
========================
*/
void SoundSystem::DestroyAudioObject(audioObject_t* audioObject ) {
	assertf( audioObject, "SoundSystem::DestroyAudioObject() failed because specified audioObject was null!\n" );

	FMOD_Sound_Release( audioObject->mSound );
	audioObject->mSound = nullptr;

	delete audioObject;
	audioObject = nullptr;
}

/*
========================
SoundSystem::PlaySound
========================
*/
void SoundSystem::PlaySound( const audioObject_t* audioObject ) const {
	assertf( audioObject, "SoundSystem::PlaySound() failed because specified audioObject was null!\n" );
	FMOD_System_PlaySound( mSystem, audioObject->mSound, mMainChannel, false, nullptr );
}

/*
========================
SoundSystem::SetMainChannelVolume
========================
*/
void SoundSystem::SetMainChannelVolume( const float32 newVolume ) {
	float32 volume = newVolume;
	if ( volume <= 0.0f ) {
		warning( "Calling SoundSystem::SetMainChannelVolume() with a value of <= 0.0f. Sound system will now appear as muted, without actually being so.\n" );
		volume = 0.0f;
	}

	FMOD_ChannelGroup_SetVolume( mMainChannel, newVolume );
}

/*
========================
SoundSystem::MuteMainChannel
========================
*/
void SoundSystem::MuteMainChannel( const bool32 mute ) {
	FMOD_ChannelGroup_SetMute( mMainChannel, mute );
}