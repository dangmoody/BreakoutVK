#include "BB.h"

/*
================================================================================================

	BB

================================================================================================
*/

/*
========================
BB::BB
========================
*/
BB::BB() {
	SetPosition( glm::vec2( 0.0f, 0.0f ) );
	SetHalfSize( glm::vec2( 1.0f, 1.0f ) );
}

/*
========================
BB::BB
========================
*/
BB::BB( const glm::vec2& position, const glm::vec2& halfSize ) {
	SetPosition( position );
	SetHalfSize( halfSize );
}

/*
========================
BB::GetSideCollidedWith
========================
*/
bbCollisionSide_t BB::GetSideCollidedWith( const BB& bb ) const {
	bool32 left = GetLeft() <= bb.GetRight();
	bool32 right = GetRight() >= bb.GetLeft();
	bool32 top = GetTop() >= bb.GetBottom();
	bool32 bottom = GetBottom() <= bb.GetTop();

	if ( left && right && top && bottom ) {
		// minkowski sum
		float32 dx = mPosition.x - bb.mPosition.x;
		float32 dy = mPosition.y - bb.mPosition.y;
		float32 width = ( mHalfSize.x + bb.mHalfSize.x );
		float32 height = ( mHalfSize.y + bb.mHalfSize.y );

		if ( glm::abs( dx ) <= width && glm::abs( dy ) <= height ) {
			float32 crossWidth = width * dy;
			float32 crossHeight = height * dx;

			if ( crossWidth > crossHeight ) {
				if ( crossWidth > -crossHeight ) {
					return BB_COLLISION_SIDE_BOTTOM;
				} else {
					return BB_COLLISION_SIDE_RIGHT;
				}
			} else {
				if ( crossWidth > -crossHeight ) {
					return BB_COLLISION_SIDE_LEFT;
				} else {
					return BB_COLLISION_SIDE_TOP;
				}
			}
		}
	}

	return BB_COLLISION_SIDE_NONE;
}