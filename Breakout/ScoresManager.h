#ifndef __SCORES_MANAGER_H__
#define __SCORES_MANAGER_H__

#include <mstd/mstd.h>

#include "Defines.h"

struct scoreEntry_t {
	string						mPlayerName;
	u32							mValue;

	scoreEntry_t operator=( const scoreEntry_t& other ) {
		mPlayerName = other.mPlayerName;
		mValue = other.mValue;

		return *this;
	}
};

/*
================================================================================================

	Breakout Scores Manager

	Stores high scores loaded from a file. If that fails for whatever reason then just uses
	some pre-cached defaults.

================================================================================================
*/

class ScoresManager {
public:
								ScoresManager();
								~ScoresManager();

	void						Init();
	void						Shutdown();
	inline bool32				IsInitialised() const { return mInitialised; }

	inline const scoreEntry_t&	GetScore( const u32 index ) const;

	void						WriteScores() const;
	void						LoadScores();

	// if the score value is higher than any of the current scores then
	// returns the index that the score would be at, otherwise returns -1
	s32							RankScore( const u32 scoreValue ) const;

	bool32						TryAddScore( const string& playerName, const u32 scoreValue );

private:
	// TODO: convert to SOA
	scoreEntry_t				mScores[NUM_MAX_SCORE_ENTRIES];

	static const scoreEntry_t	DEFAULT_SCORES[NUM_MAX_SCORE_ENTRIES];

	bool32						mInitialised;
};

extern ScoresManager* gScoresManager;

/*
========================
ScoresManager::GetScore
========================
*/
const scoreEntry_t& ScoresManager::GetScore( const u32 index ) const {
	assertf( index >= 0, "Tried to access a score where index < 0!" );
	assertf( index < NUM_MAX_SCORE_ENTRIES, "Tried to access a score where index >= max score entries!" );

	return mScores[index];
}

#endif // __SCORES_MANAGER_H__