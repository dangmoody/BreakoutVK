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
#ifndef __MSTD_MATH_H__
#define __MSTD_MATH_H__

#undef min
#undef max

#define PI						3.1415926535f
#define TAU						PI * 2.0f
#define EPSILON					0.0001f

								/// Returns the smaller of the two numbers.
template<class T> inline T		min( const T& x, const T& y );

								/// Returns the larger of the two numbers.
template<class T> inline T		max( const T& x, const T& y );

								/// If x goes above max then returns min. If x goes below min then returns max. Otherwise just returns x.
template<class T> inline T		wrap( const T& x, const T& min, const T& max );

								/// Makes the value at variable x the value of variable y and vice versa.
template<class T> inline void	swap( const T& x, const T& y );

/*
========================
min
========================
*/
template<class T>
T min( const T& x, const T& y ) {
	return x < y ? x : y;
}

/*
========================
max
========================
*/
template<class T>
T max( const T& x, const T& y ) {
	return x > y ? x : y;
}

/*
========================
wrap
========================
*/
template<class T>
T wrap( const T& x, const T& min, const T& max ) {
	return x > max ? min : x < min ? max : x;
}

/*
========================
swap
========================
*/
template<class T>
void swap( const T& x, const T& y ) {
	T& temp = y;
	y = x;
	x = temp;
}

#endif