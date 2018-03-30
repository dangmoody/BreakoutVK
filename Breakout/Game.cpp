#include "Game.h"

#include "Window.h"
#include "Renderer.h"
#include "InputHandler.h"
#include "SoundSystem.h"
#include "Entity.h"
#include "UI.h"
#include "ScoresManager.h"

/*
================================================================================================

	Game

================================================================================================
*/

Game* gGame = nullptr;

enum key_t {
	KEY_QUIT		= SDL_SCANCODE_ESCAPE,
	KEY_MOVE_LEFT	= SDL_SCANCODE_A,
	KEY_MOVE_RIGHT	= SDL_SCANCODE_D,

	KEY_START_GAME	= SDL_SCANCODE_SPACE,

	KEY_MUTE_SOUND	= SDL_SCANCODE_S,
	KEY_SHOW_DEBUG	= SDL_SCANCODE_F3,
};

const u32 Game::BLOCK_ROW_SCORES[] = {
	7, 7, 4, 4, 1, 1
};

/*
========================
Game::Game
========================
*/
Game::Game() {
	mRunning = false;

	mSoundHitPlayer = nullptr;
	mSoundHitWalls = nullptr;
	mSoundHitBlock = nullptr;

	mPlayer = nullptr;
	mBall = nullptr;

	mMute = false;
	mShowDebug = false;
}

/*
========================
Game::~Game
========================
*/
Game::~Game() {
	Shutdown();
}

/*
========================
Game::Init
========================
*/
bool32 Game::Init() {
	if ( IsRunning() ) {
		return false;
	}

	printf( "------- Game init called -------\n" );

	seedRandom();

	SDL_Init( SDL_INIT_VIDEO | SDL_INIT_EVENTS );

	gWindow = new Window();
	gInput = new InputHandler();
	gRenderer = new Renderer();
	gSoundSystem = new SoundSystem();
	gUI = new UI();
	gScoresManager = new ScoresManager();

	gWindow->Init();

	gInput->Init();

	gRenderer->Init();

	gSoundSystem->Init();

	gSoundSystem->SetMainChannelVolume( 0.1f );

	mSoundHitPlayer = gSoundSystem->CreateAudioObject( BASE_PATH "sound/hit_player.wav" );
	mSoundHitWalls = gSoundSystem->CreateAudioObject( BASE_PATH "sound/hit_walls.wav" );
	mSoundHitBlock = gSoundSystem->CreateAudioObject( BASE_PATH "sound/hit_block.wav" );

	gUI->Init( GAME_WIDTH, GAME_HEIGHT );

	gScoresManager->Init();

	// init game entities
	mBlocks.resize( NUM_BLOCKS_MAX );
	for ( u32 rowIndex = 0; rowIndex < NUM_BLOCKS_ROWS; rowIndex++ ) {
		for ( u32 columnIndex = 0; columnIndex < NUM_BLOCKS_COLUMNS; columnIndex++ ) {
			size_t blockIndex = columnIndex + ( rowIndex * NUM_BLOCKS_COLUMNS );

			float32 blockX = -5.0f + columnIndex;
			float32 blockY = 4.0f - ( rowIndex * 0.5f );
			glm::vec3 pos( blockX, blockY, 1.0f );

			glm::vec2 blockSize( 0.5f, 0.25f );

			mBlocks[blockIndex] = new Entity( pos, blockSize, Entity::COLORS[rowIndex], BLOCK_ROW_SCORES[rowIndex] );
		}
	}

	mPlayer = new Entity();
	mBall = new Entity();

	ResetLevel();

	printf( "------- Game init complete -------\n\n" );

	mRunning = true;

	return true;
}

/*
========================
Game::Shutdown
========================
*/
void Game::Shutdown() {
	gSoundSystem->DestroyAudioObject( mSoundHitBlock );
	gSoundSystem->DestroyAudioObject( mSoundHitWalls );
	gSoundSystem->DestroyAudioObject( mSoundHitPlayer );

	delete mPlayer;
	mPlayer = nullptr;

	delete mBall;
	mBall = nullptr;

	for ( Entity* block : mBlocks ) {
		delete block;
		block = nullptr;
	}

	delete gUI;
	gUI = nullptr;

	delete gSoundSystem;
	gSoundSystem = nullptr;

	delete gRenderer;
	gRenderer = nullptr;

	delete gInput;
	gInput = nullptr;

	delete gWindow;
	gWindow = nullptr;

	SDL_Quit();

	mRunning = false;
}

/*
========================
Game::Frame
========================
*/
void Game::Frame() {
	float64 frameDelta = 0.0;
	mFrameStart = timeNow();

	// update
	{
		gInput->Update();

		if ( gInput->IsKeyPressed( KEY_QUIT ) ) {
			mRunning = false;
		}

		if ( gInput->IsKeyPressed( KEY_SHOW_DEBUG ) ) {
			mShowDebug = !mShowDebug;
		}

		// update key states from input to ImGui
		memcpy( ImGui::GetIO().KeysDown, gInput->GetCurrentKeyState(), gInput->GetNumMaxKeys() );
		ImGui::GetIO().DeltaTime = mDeltaTime;

		while ( SDL_PollEvent( &mEvent ) ) {
			switch ( mEvent.type ) {
			case SDL_QUIT:
				mRunning = false;
				break;

			case SDL_WINDOWEVENT:
				switch ( mEvent.window.event ) {
				case SDL_WINDOWEVENT_RESIZED: {
					u32 newWidth = mEvent.window.data1;
					u32 newHeight = mEvent.window.data2;

					gWindow->Resize( newWidth, newHeight );

					gRenderer->Resize( newWidth, newHeight );
					break;
				}
				}
				break;

			case SDL_TEXTINPUT:
				ImGui::GetIO().AddInputCharactersUTF8( static_cast<char*>( mEvent.text.text ) );
				break;
			}
		}

		gSoundSystem->Update();

		gUI->Begin();

		switch ( mCurrentState ) {
		case GAME_STATE_WAITING:
			StateWaiting();
			break;

		case GAME_STATE_PLAYING:
			StatePlaying();
			break;

		case GAME_STATE_DIED:
			StateDied();
			break;

		case GAME_STATE_HIGH_SCORE:
			StateHighScore();
			break;
		}

		// show hud
		{
			gUI->PushWindow( ImVec2( 0, 0 ), ImVec4( 0, 0, 0, 0 ) );

			ImGui::Text( "SCORE: %d", mPlayerScore );
			ImGui::SameLine(); ImGui::Text( "LIVES: %d", mPlayerLives );
			ImGui::Text( "MUTE: %s", mMute ? "ON" : "OFF" );

			if ( mShowDebug ) {
				ImGui::Text( "%s", mDebugText.c_str() );
			}

			gUI->PopWindow();
		}

		gUI->End();
	}

	// render
	{
		gRenderer->StartFrame();

		for ( Entity* block : mBlocks ) {
			block->Render();
		}
		mBall->Render();
		mPlayer->Render();

		gRenderer->DrawElements();

		gUI->Render();

		gRenderer->EndFrame();
	}

	// end of frame, get stats
	{
		mFrames++;

		mFrameEnd = timeNow();
		frameDelta = deltaMilliseconds( mFrameStart, mFrameEnd );

		mDeltaTime = static_cast<float32>( frameDelta / 1000.0 );
		mFPSTimer += static_cast<float32>( frameDelta );

		if ( mFPSTimer > 1000.0f ) {
			setf( mDebugText, "MS/FRAME: %f (%d FPS)", mDeltaTime * 1000.0, mFrames );

			mFPSTimer = 0.0f;
			mFrames = 0;
		}
	}
}

/*
========================
Game::StateWaiting
========================
*/
void Game::StateWaiting() {
	if ( gInput->IsKeyPressed( KEY_START_GAME ) ) {
		mCurrentState = GAME_STATE_PLAYING;
	}
}

/*
========================
Game::StatePlaying
========================
*/
void Game::StatePlaying() {
	UpdatePlayer();
	UpdateBall();

	if ( mHitBlocks == mBlocks.length() ) {
		GameOver();
	}
}

/*
========================
Game::StateDied
========================
*/
void Game::StateDied() {
	if ( gInput->IsKeyPressed( KEY_START_GAME ) ) {
		if ( mPlayerLives > 0 ) {
			ResetPlayerAndBall();
		} else {
			GameOver();
		}
	}
}

/*
========================
Game::StateHighScore
========================
*/
void Game::StateHighScore() {
	scoreEntry_t score = gScoresManager->GetScore( 0 );

	string highScoreTableFormat;
	setf( highScoreTableFormat, "%-6d %-6s %03d\n", 0, score.mPlayerName.c_str(), score.mValue );

	float32 fontSize = ImGui::GetFontSize();
	float32 tableRowLength = highScoreTableFormat.length() * fontSize;
	float32 tableColumnHeight = NUM_MAX_SCORE_ENTRIES * fontSize;

	float32 windowX = ( GAME_WIDTH - tableRowLength ) * 0.5f;
	float32 windowY = ( GAME_HEIGHT - tableColumnHeight ) * 0.4f;
	gUI->PushWindow( ImVec2( windowX, windowY ), ImVec4( 0, 0, 0, 1 ) );

	// show highscores
	for ( u32 i = 0; i < NUM_MAX_SCORE_ENTRIES; i++ ) {
		const scoreEntry_t& scoreEntry = gScoresManager->GetScore( i );
		ImGui::Text( "%-6d %-6s %03d\n", i + 1, scoreEntry.mPlayerName.c_str(), scoreEntry.mValue );
	}

	// if player got a new high score let them enter their name
	// otherwise wait for them to press the continue button
	if ( gScoresManager->RankScore( mPlayerScore ) == -1 ) {
		if ( gInput->IsKeyPressed( KEY_START_GAME ) ) {
			ResetLevel();
		}
	} else {
		char inputBuffer[SCORE_NAME_LENGTH_MAX + 1] = { 0 };

		ImGui::Text( "NAME:" ); ImGui::SameLine();
		ImGui::SetKeyboardFocusHere();

		if ( ImGui::InputText( "", inputBuffer, SCORE_NAME_LENGTH_MAX + 1, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsUppercase ) ) {
			inputBuffer[SCORE_NAME_LENGTH_MAX] = 0;

			gScoresManager->TryAddScore( inputBuffer, mPlayerScore );
			gScoresManager->WriteScores();

			ResetLevel();
		}
	}

	gUI->PopWindow();
}

/*
========================
Game::UpdatePlayer
========================
*/
void Game::UpdatePlayer() {
	mPlayer->UpdateBB();

	// reset direction on a per-frame basis
	mPlayerDirection = glm::vec3( 0.0f );

	if ( gInput->IsKeyDown( KEY_MOVE_LEFT ) && mPlayer->GetBB().GetLeft() >= -6.5f ) {
		mPlayerDirection.x = -1.0f;
	}

	if ( gInput->IsKeyDown( KEY_MOVE_RIGHT ) && mPlayer->GetBB().GetRight() <= 6.5f ) {
		mPlayerDirection.x = 1.0f;
	}

	if ( gInput->IsKeyPressed( KEY_MUTE_SOUND ) ) {
		mMute = !mMute;
		gSoundSystem->MuteMainChannel( mMute );
	}

	mPlayer->Translate( mPlayerDirection * PLAYER_MOVE_SPEED * mDeltaTime );
}

/*
========================
Game::UpdateBall
========================
*/
void Game::UpdateBall() {
	mBall->UpdateBB();

	glm::vec4 pointScreen( 2.0f * GAME_WIDTH / GAME_WIDTH - 1.0f, 2.0f * GAME_HEIGHT / GAME_HEIGHT - 1.0f, 1.0f, 1.0f );

	glm::vec3 screenToWorld = gRenderer->GetClipToWorld() * pointScreen;
	float32 screenBoundRight = screenToWorld.x;
	float32 screenBoundTop = screenToWorld.y;

	// check collision with screen bounds
	if ( mBall->GetBB().GetLeft() <= -screenBoundRight ) {
		mBall->TranslateX( mBallMoveSpeed * mDeltaTime );
		mBallDirection.x *= -1.0f;

		gSoundSystem->PlaySound( mSoundHitWalls );
	}

	if ( mBall->GetBB().GetRight() >= screenBoundRight ) {
		mBall->TranslateX( -mBallMoveSpeed * mDeltaTime );
		mBallDirection.x *= -1.0f;

		gSoundSystem->PlaySound( mSoundHitWalls );
	}

	if ( mBall->GetBB().GetBottom() <= -screenBoundTop ) {
		mCurrentState = GAME_STATE_DIED;
		mPlayerLives--;
	}

	if ( mBall->GetBB().GetTop() >= screenBoundTop ) {
		mBall->TranslateY( -mBallMoveSpeed * mDeltaTime );
		mBallDirection.y *= -1.0f;

		gSoundSystem->PlaySound( mSoundHitWalls );
	}

	// check collision with blocks
	for ( Entity* block : mBlocks ) {
		if ( !block->IsActive() ) {
			continue;
		}

		bbCollisionSide_t collision = mBall->GetSideCollidedWith( block );

		// I imagine this can be condensed some more
		// but this is good _enough_?
		if ( collision != BB_COLLISION_SIDE_NONE ) {
			switch ( collision ) {
			case BB_COLLISION_SIDE_TOP:
			case BB_COLLISION_SIDE_BOTTOM:
				mBallDirection.y *= -1.0f;
				mBall->TranslateY( mBallMoveSpeed * mBallDirection.y * mDeltaTime );
				break;

			case BB_COLLISION_SIDE_LEFT:
			case BB_COLLISION_SIDE_RIGHT:
				mBallDirection.x *= -1.0f;
				mBall->TranslateX( mBallMoveSpeed * mBallDirection.x * mDeltaTime );
				break;

			default:
				// nothing
				break;
			}

			mPlayerScore += block->GetScoreValue();
			block->SetActive( false );
			mHitBlocks++;

			gSoundSystem->PlaySound( mSoundHitBlock );

			break;
		}
	}

	// check collision with player
	bbCollisionSide_t collisionSide = mBall->GetSideCollidedWith( mPlayer );
	switch ( collisionSide ) {
	case BB_COLLISION_SIDE_TOP:
		mBall->TranslateY( -mBallMoveSpeed * mDeltaTime );
		mBallDirection.y *= -1.0f;

		gSoundSystem->PlaySound( mSoundHitPlayer );
		break;

	case BB_COLLISION_SIDE_BOTTOM: {
		mBall->TranslateY( mBallMoveSpeed * mDeltaTime );

		float32 dx = mBall->GetPosition().x - mPlayer->GetPosition().x;
		float32 variance = random<float32>( 0.25f, 1.0f );
		float32 newDirX = ( mBallDirection.x + mPlayerDirection.x + dx ) * variance;

		mBallDirection.x = glm::clamp( newDirX, -1.0f, 1.0f );
		mBallDirection.y *= -1.0f;

		mBallMoveSpeed += mBallSpeedIncrease;

		gSoundSystem->PlaySound( mSoundHitPlayer );
		break;
	}

	case BB_COLLISION_SIDE_LEFT:
		mBall->TranslateX( mBallMoveSpeed * PLAYER_MOVE_SPEED * mDeltaTime );
		mBallDirection.x = 1.0f;

		gSoundSystem->PlaySound( mSoundHitPlayer );
		break;

	case BB_COLLISION_SIDE_RIGHT:
		mBall->TranslateX( -mBallMoveSpeed * PLAYER_MOVE_SPEED * mDeltaTime );
		mBallDirection.x = -1.0f;

		gSoundSystem->PlaySound( mSoundHitPlayer );
		break;

	case BB_COLLISION_SIDE_NONE:
	default:
		// nothing
		break;
	}

	mBall->Translate( mBallDirection * mBallMoveSpeed * mDeltaTime );
}

/*
========================
Game::ResetPlayerAndBall
========================
*/
void Game::ResetPlayerAndBall() {
	// init player
	glm::vec3 playerStartPos( 0.0f, -4.0f, 1.0f );
	glm::vec2 playerSize( 1.0f, 0.15f );
	mPlayer->Init( playerStartPos, playerSize, Entity::COLORS[0] );

	// init ball
	glm::vec3 ballStartPos = playerStartPos + glm::vec3( 0.0f, 0.5f, 0.0f );
	glm::vec2 ballSize( 0.15f, 0.15f );
	mBall->Init( ballStartPos, ballSize, Entity::COLORS[0] );
	
	mBallDirection = glm::vec3( 1.0f, 1.0f, 0.0f );
	mBallMoveSpeed = BALL_START_MOVE_SPEED;

	mCurrentState = GAME_STATE_WAITING;
}

/*
========================
Game::ResetLevel
========================
*/
void Game::ResetLevel() {
	for ( Entity* block : mBlocks ) {
		block->SetActive( true );
	}

	mPlayerLives = NUM_MAX_PLAYER_LIVES;
	mPlayerScore = 0;
	mHitBlocks = 0;

	ResetPlayerAndBall();
}

/*
========================
Game::GameOver
========================
*/
void Game::GameOver() {
	mCurrentState = GAME_STATE_HIGH_SCORE;
}