#ifndef __BB_H__
#define __BB_H__

#include <mstd/mstd.h>

#pragma warning( disable : 4201 )
#include <glm/glm.hpp>
#pragma warning( default : 4201 )

/*
================================================================================================

	Breakout Axis Aligned Bounding Box

================================================================================================
*/

#define VEC2_UP				glm::vec2( 0.0f, 1.0 )

enum bbCollisionSide_t {
	BB_COLLISION_SIDE_NONE	= 0,	// no collision

	BB_COLLISION_SIDE_TOP,
	BB_COLLISION_SIDE_LEFT,
	BB_COLLISION_SIDE_RIGHT,
	BB_COLLISION_SIDE_BOTTOM,

	BB_COLLISION_SIDE_COUNT
};

class BB {
public:
							BB();
							BB( const glm::vec2& position, const glm::vec2& halfSize );
							~BB() {}

	inline float32			GetLeft() const { return mPosition.x - mHalfSize.x; }
	inline float32			GetRight() const { return mPosition.x + mHalfSize.x; }
	inline float32			GetTop() const { return mPosition.y + mHalfSize.y; }
	inline float32			GetBottom() const { return mPosition.y - mHalfSize.y; }

	inline void				SetPosition( const glm::vec2& position ) { mPosition = position; }
	inline void				SetHalfSize( const glm::vec2& halfSize ) { mHalfSize = halfSize; }

	bbCollisionSide_t		GetSideCollidedWith( const BB& bb ) const;

	bool32 operator==( const BB& other ) const {
		return mPosition == other.mPosition && mHalfSize == other.mHalfSize;
	}

private:
	glm::vec2				mPosition;
	glm::vec2				mHalfSize;
};

#endif // __BB_H__