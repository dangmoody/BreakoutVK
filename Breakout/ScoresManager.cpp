#include "ScoresManager.h"

/*
================================================================================================

	ScoresManager

================================================================================================
*/

ScoresManager* gScoresManager = nullptr;

const scoreEntry_t ScoresManager::DEFAULT_SCORES[NUM_MAX_SCORE_ENTRIES] {
	{ "DAN", 250 },
	{ "DAN", 100 },
	{ "DAN", 75 },
	{ "ZAK", 50 },
	{ "BEN", 35 },
	{ "WIL", 30 },
	{ "ALX", 25 },
	{ "HRV", 20 },
	{ "NGE", 10 },
	{ "MRK", 5 }
};

/*
========================
ScoresManager::ScoresManager
========================
*/
ScoresManager::ScoresManager() {
	memset( mScores, 0, sizeof( mScores ) );

	mInitialised = false;
}

/*
========================
ScoresManager::~ScoresManager
========================
*/
ScoresManager::~ScoresManager() {
	Shutdown();
}

/*
========================
ScoresManager::Init
========================
*/
void ScoresManager::Init() {
	if ( IsInitialised() ) {
		return;
	}

	LoadScores();

	mInitialised = true;
}

/*
========================
ScoresManager::Shutdown
========================
*/
void ScoresManager::Shutdown() {
	if ( !IsInitialised() ) {
		return;
	}

	mInitialised = false;
}

/*
========================
ScoresManager::WriteScores
========================
*/
void ScoresManager::WriteScores() const {
	file_t scoresFile = openOrCreateFile( SCORES_FILE_PATH );
	if ( !scoresFile ) {
		fatalError( "Unable to open/create high scores file %s. High scores won't be saved this time around. Sorry!", SCORES_FILE_PATH );
		return;
	}

	// write out the new high scores
	for ( size_t j = 0; j < NUM_MAX_SCORE_ENTRIES; j++ ) {
		const scoreEntry_t& writeEntry = mScores[j];

		appendFile( scoresFile, writeEntry.mPlayerName.c_str(), writeEntry.mPlayerName.length() );
		appendFile( scoresFile, &writeEntry.mValue, sizeof( writeEntry.mValue ) );
	}

	closeFile( scoresFile );
}

/*
========================
ScoresManager::LoadScores
========================
*/
void ScoresManager::LoadScores() {
	char* buffer = nullptr;
	size_t bytes = readEntireFile( SCORES_FILE_PATH, &buffer );

	if ( bytes == 0 ) {
		// cant read scores file so use defaults instead
		memcpy( mScores, DEFAULT_SCORES, sizeof( DEFAULT_SCORES ) );
	} else {
		char* playerName = new char[SCORE_NAME_LENGTH_MAX + 1];
		size_t offset = 0;

		for ( size_t i = 0; i < NUM_MAX_SCORE_ENTRIES; i++ ) {
			scoreEntry_t& scoreEntry = mScores[i];

			strncpy( playerName, buffer + offset, SCORE_NAME_LENGTH_MAX );
			playerName[SCORE_NAME_LENGTH_MAX] = 0;
			offset += SCORE_NAME_LENGTH_MAX;

			u32* playerScore = reinterpret_cast<u32*>( buffer + offset );
			offset += sizeof( u32 );

			scoreEntry.mPlayerName = playerName;
			scoreEntry.mValue = *playerScore;
		}

		delete[] playerName;
		playerName = nullptr;
	}

	delete[] buffer;
	buffer = nullptr;

	printf( "\n" );
}

/*
========================
ScoresManager::RankScore
========================
*/
s32 ScoresManager::RankScore( const u32 scoreValue ) const {
	for ( s32 i = 0; i < NUM_MAX_SCORE_ENTRIES; i++ ) {
		if ( scoreValue > mScores[i].mValue ) {
			return i;
		}
	}

	return -1;
}

/*
========================
ScoresManager::TryAddScore
========================
*/
bool32 ScoresManager::TryAddScore( const string& playerName, const u32 scoreValue ) {
	s32 scoreRank = RankScore( scoreValue );

	if ( scoreRank == -1 ) {
		return false;
	}

	// move other scores down
	for ( s32 j = NUM_MAX_SCORE_ENTRIES - 1; j > scoreRank; j-- ) {
		mScores[j] = mScores[j - 1];
	}

	mScores[scoreRank] = { playerName, scoreValue };

	return true;
}