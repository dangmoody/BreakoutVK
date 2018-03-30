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
#ifndef __MSTD_DEFINES_H__
#define __MSTD_DEFINES_H__

#if defined( _WIN32 )
#define MSTD_OS_WINDOWS		1
#elif defined( __APPLE__ )
#define MSTD_OS_MAC_OS		1
#elif defined( __linux__ )
#define MSTD_OS_LINUX		1
#endif

#if defined( _M_IX86 )
#define MSTD_X86			1
#define MSTD_X64			0
#elif defined( _M_IA64 ) || defined( _AMD64_ )
#define MSTD_X86			0
#define MSTD_X64			1
#endif

#define BIT( x )			1ULL << ( x )
#define UNUSED( x )			( x ) = ( x )

#ifdef _DEBUG
#define MSTD_DEBUG			1
#define MSTD_RELEASE		0

#if MSTD_OS_WINDOWS
#define MSTD_DEBUG_BREAK()	if ( IsDebuggerPresent() ) __debugbreak()
#elif MSTD_OS_MAC_OS || MSTD_OS_LINUX
#define MSTD_DEBUG_BREAK()	raise( SIGTRAP )
#endif

#define assertf( x, msg )	if ( x == false ) { fatalError( "Assertion: %s:%d \"%s\", \"%s\"", __FUNCTION__, __LINE__, ( #x ), msg ); MSTD_DEBUG_BREAK(); }
#else
#define MSTD_DEBUG			0
#define MSTD_RELEASE		1

#define assertf( x, msg )
#endif

#endif