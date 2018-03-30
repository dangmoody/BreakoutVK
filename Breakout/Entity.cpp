#include "Entity.h"

/*
================================================================================================

	Entity

================================================================================================
*/

const glm::vec4 Entity::COLORS[] = {
	glm::vec4( 200, 72, 72, 255 ) / 255.0f,
	glm::vec4( 198, 108, 58, 255 ) / 255.0f,
	glm::vec4( 180, 122, 48, 255 ) / 255.0f,
	glm::vec4( 162, 162, 42, 255 ) / 255.0f,
	glm::vec4( 72, 160, 72, 255 ) / 255.0f,
	glm::vec4( 66, 72, 200, 255 ) / 255.0f,
};

/*
========================
Entity::Entity
========================
*/
Entity::Entity() {
	Init( glm::vec3(), glm::vec2(), glm::vec4( 1.0f ) );
}

/*
========================
Entity::Entity
========================
*/
Entity::Entity( const glm::vec3& position, const glm::vec2& halfSize, const glm::vec4& color, const u32 scoreValue ) {
	Init( position, halfSize, color, scoreValue );
}

/*
========================
Entity::Init
========================
*/
void Entity::Init( const glm::vec3& position, const glm::vec2& halfSize, const glm::vec4& color, const u32 scoreValue ) {
	SetPosition( position );
	SetHalfSize( halfSize );
	SetColor( color );

	SetActive( true );

	mScoreValue = scoreValue;

	mBB = BB( position, halfSize );
}

/*
========================
Entity::UpdateBB
========================
*/
void Entity::UpdateBB() {
	mBB.SetPosition( mPosition );
}

/*
========================
Entity::Render
========================
*/
void Entity::Render() {
	if ( !mActive ) {
		return;
	}

	// update position in uniform data per-frame because that's most likely to change
	mUniformData.mModel = glm::translate( glm::mat4(), mPosition );
	gRenderer->AddRenderJob( mUniformData );
}