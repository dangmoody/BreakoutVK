/*
===========================================================================

mock-std.
Copyright (c) Dan Moody 2018 - Present.

This file is part of mock-std.

mock-std is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

mock-std is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with mock-std.  If not, see <http://www.gnu.org/licenses/>.

===========================================================================
*/
#ifndef __MSTD_RANDOM_H__
#define __MSTD_RANDOM_H__

#include <stdlib.h>
#include <time.h>

					/// Generates a random seed based off UNIX time.
inline void			seedRandom( void );

					/// Generates a random number between the desired range.
template<class T> T	random( const T min, const T max );

/*
========================
seedRandom
========================
*/
void seedRandom( void ) {
	srand( static_cast<u32>( time( nullptr ) ) );
}

/*
========================
random
========================
*/
template<class T>
T random<T>( const T min, const T max ) {
	return min + static_cast<T>( rand() ) / ( static_cast<T>( RAND_MAX ) / ( max - min + 1 ) + 1 );
}

#endif