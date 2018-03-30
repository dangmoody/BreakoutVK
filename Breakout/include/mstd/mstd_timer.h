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
#ifndef __MSTD_TIMER_H__
#define __MSTD_TIMER_H__

/// Returns a timestamp of the current time.
inline timestamp_t	timeNow( void );

/// Returns the number of seconds passed between start and end.
inline float64		deltaSeconds( const timestamp_t start, const timestamp_t end );

/// Returns the number of milliseconds passed between start and end.
inline float64		deltaMilliseconds( const timestamp_t start, const timestamp_t end );

/// Returns the number of microseconds passed between start and end.
inline float64		deltaMicroseconds( const timestamp_t start, const timestamp_t end );

/// Returns the number of nanoseconds passed between start and end.
inline float64		deltaNanoseconds( const timestamp_t start, const timestamp_t end );

inline u64			clockFrequency( void );

/*
========================
timeNow
========================
*/
timestamp_t timeNow( void ) {
#if MSTD_OS_WINDOWS
	static LARGE_INTEGER now;
	QueryPerformanceCounter( &now );
	return now.QuadPart;
#endif
}

/*
========================
deltaMilliseconds
========================
*/
float64 deltaMilliseconds( const timestamp_t start, const timestamp_t end ) {
#if MSTD_OS_WINDOWS
	timestamp_t delta = ( ( end - start ) * 1000000 ) / clockFrequency();
	return static_cast<float64>( delta / 1000.0 );
#endif
}

/*
========================
deltaMicroseconds
========================
*/
float64 deltaMicroseconds( const timestamp_t start, const timestamp_t end ) {
#if MSTD_OS_WINDOWS
	return static_cast<float64>( ( ( end - start ) * 1000000 ) / clockFrequency() );
#endif
}

/*
========================
deltaNanoseconds
========================
*/
float64 deltaNanoseconds( const timestamp_t start, const timestamp_t end ) {
#if MSTD_OS_WINDOWS
	timestamp_t delta = ( ( end - start ) * 1000000 ) / clockFrequency();
	return static_cast<float64>( delta * 1000.0 );
#endif
}

/*
========================
deltaSeconds
========================
*/
float64 deltaSeconds( const timestamp_t start, const timestamp_t end ) {
#if MSTD_OS_WINDOWS
	return static_cast<float64>( ( ( end - start ) ) / clockFrequency() );
#endif
}

/*
========================
clockFrequency
========================
*/
u64 clockFrequency() {
#if MSTD_OS_WINDOWS
	static LARGE_INTEGER frequency = {};
	if ( frequency.QuadPart == 0 ) {
		QueryPerformanceFrequency( &frequency );
	}
	return frequency.QuadPart;
#endif
}

#endif