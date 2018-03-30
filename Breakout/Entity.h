#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <mstd/mstd.h>

#pragma warning( disable : 4201 )
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#pragma warning( default : 4201 )

#include "BB.h"

#include "Renderer.h"

/*
================================================================================================

	Breakout Entity

	Handles all game-logic for quad entities in the game.

================================================================================================
*/

class Entity {
public:
	static const glm::vec4		COLORS[];

public:
								Entity();
								Entity( const glm::vec3& position, const glm::vec2& halfSize, const glm::vec4& color, const u32 scoreValue = 0 );
								~Entity() {}

	void						Init( const glm::vec3& position, const glm::vec2& halfSize, const glm::vec4& color, const u32 scoreValue = 0 );

	inline const glm::vec3&		GetPosition() const { return mPosition; }
	inline void					SetPosition( const glm::vec3& position ) { mPosition = position; }
	inline void					Translate( const glm::vec3& direction ) { mPosition += direction; }

	inline void					TranslateX( const float32 dx ) { mPosition.x += dx; }
	inline void					TranslateY( const float32 dy ) { mPosition.y += dy; }

	inline const glm::vec4&		GetColor() const { return mColor; }
	inline void					SetColor( const glm::vec4& color );

	inline const glm::vec2&		GetHalfSize() const { return mHalfSize; }
	inline void					SetHalfSize( const glm::vec2& halfSize );

	inline const BB&			GetBB() const { return mBB; }
	inline bbCollisionSide_t	GetSideCollidedWith( const Entity* entity ) const;

	inline bool32				IsActive() const { return mActive; }
	inline void					SetActive( const bool32 active ) { mActive = active; }

	inline u32					GetScoreValue() const { return mScoreValue; }

	void						UpdateBB();

	void						Render();

private:
	uniformDataQuad_t			mUniformData;

	glm::vec4					mColor;
	glm::vec3					mPosition;
	glm::vec2					mHalfSize;

	BB							mBB;

	u32							mScoreValue;

	bool32						mActive;
};

/*
========================
Entity::SetColor
========================
*/
void Entity::SetColor( const glm::vec4& color ) {
	mColor = color;
	mUniformData.mColor = color;
}

/*
========================
Entity::SetHalfSize
========================
*/
void Entity::SetHalfSize( const glm::vec2& halfSize ) {
	mHalfSize = halfSize;
	mUniformData.mScale = halfSize;
}

/*
========================
Entity::Intersects
========================
*/
bbCollisionSide_t Entity::GetSideCollidedWith( const Entity* entity ) const {
	if ( !entity ) {
		return BB_COLLISION_SIDE_NONE;
	}

	if ( !IsActive() || !entity->IsActive() ) {
		return BB_COLLISION_SIDE_NONE;
	}

	return mBB.GetSideCollidedWith( entity->GetBB() );
}

#endif // __ENTITY_H__