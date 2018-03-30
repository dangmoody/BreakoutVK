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
#ifndef __MSTD_DEBUG_H__
#define __MSTD_DEBUG_H__

#include <stdio.h>
#include <signal.h>

#ifdef _WIN32
#include <Windows.h>
#include <crtdbg.h>
#endif

#ifdef _DEBUG
#ifdef _WIN32
#define MSTD_FATAL_ERROR_WINDOW( fmt )	_CrtDbgReport( _CRT_ASSERT, __FILE__, __LINE__, NULL, fmt );
#else
#define MSTD_FATAL_ERROR_WINDOW( fmt )
#endif
#else
#define MSTD_FATAL_ERROR_WINDOW( fmt )
#endif

										/// Prints the text "WARNING:" in red text followed by the specified warning message in yellow text.
inline void								warning( const char* fmt, ... );

										/// Prints the text "ERROR:" in red text followed by the specified error message in yellow text.
inline void								error( const char* fmt, ... );

										/// Prints the text "FATAL ERROR: " in red text followed by the specified error message in yellow text, then exits the program.
inline void								fatalError( const char* fmt, ... );

/*
========================
warning
========================
*/
void warning( const char* fmt, ... ) {
#ifdef _WIN32
	HANDLE handle = GetStdHandle( STD_OUTPUT_HANDLE );

	va_list args;
	va_start( args, fmt );

	SetConsoleTextAttribute( handle, 0x0C );

	printf( "WARNING: " );

	SetConsoleTextAttribute( handle, 0x0E );

	vprintf( fmt, args );
	va_end( args );

	SetConsoleTextAttribute( handle, 0x07 );
#endif
}

/*
========================
error
========================
*/
void error( const char* fmt, ... ) {
#ifdef _WIN32
	HANDLE handle = GetStdHandle( STD_OUTPUT_HANDLE );

	va_list args;
	va_start( args, fmt );

	SetConsoleTextAttribute( handle, 0x0C );

	printf( "ERROR: " );

	SetConsoleTextAttribute( handle, 0x0E );

	vprintf( fmt, args );
	va_end( args );

	SetConsoleTextAttribute( handle, 0x07 );
#endif
}

/*
========================
fatalError
========================
*/
void fatalError( const char* fmt, ... ) {
#ifdef _WIN32
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_WNDW );

	va_list args;
	va_start( args, fmt );

	u32 length = vsnprintf( nullptr, 0, fmt, args );
	char* errorMsg = new char[length + 1];
	vsprintf( errorMsg, fmt, args );
	errorMsg[length] = 0;

	MSTD_FATAL_ERROR_WINDOW( errorMsg );
#endif
}

#endif