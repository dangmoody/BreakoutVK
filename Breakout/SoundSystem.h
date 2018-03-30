#ifndef __SOUND_SYSTEM_H__
#define __SOUND_SYSTEM_H__

#include <mstd/mstd.h>

#include <fmod/fmod.h>

struct audioObject_t {
	FMOD_SOUND*			mSound;
};

/*
================================================================================================

	Yeti Sound System

	Uses FMOD. Audio objects are handled similarly to the way the Yeti Vulkan Context handles
	its Buffers, Shaders, Render States, etc.

================================================================================================
*/

class SoundSystem {
public:
	static const int	NUM_MAX_CHANNELS = 36;

public:
						SoundSystem();
	virtual				~SoundSystem();

	void				Init();
	void				Shutdown();
	bool32				IsInitialised() const { return mInitialised; }

	void				Update();

	audioObject_t*		CreateAudioObject( const string& filename );
	void				DestroyAudioObject( audioObject_t* audioObject );

	void				PlaySound( const audioObject_t* audioObject ) const;

	void				SetMainChannelVolume( const float32 newVolume );
	void				MuteMainChannel( const bool32 mute );

private:
	FMOD_SYSTEM*		mSystem;
	FMOD_CHANNELGROUP*	mMainChannel;

	bool32				mInitialised;
};

extern SoundSystem* gSoundSystem;

#endif // __SOUND_MANAGER_H__