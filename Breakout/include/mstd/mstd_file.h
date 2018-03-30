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
#ifndef __MSTD_FILE_H__
#define __MSTD_FILE_H__

typedef void*			file_t;

						/// \brief Opens the file for reading and writing.
						///
						/// Allows externally shared file access while the file is open in ystd.
						/// If the file can be opened then returns the file handle as a pointer, otherwise returns nullptr.
inline file_t			openFile( const char* filename );

						/// \brief If the file exists then opens it, otherwise creates it and then opens it.
						///
						/// If the file can be opened or created then returns the file handle as a pointer, otherwise returns nullptr.
inline file_t			openOrCreateFile( const char* filename );

						/// \brief Closes the file.
						///
						/// Returns true if the file closed successfully, otherwise returns false.
inline bool32			closeFile( file_t fileHandle );

						/// \brief Reads the entire contents of the file into the specified buffer.
						///
						/// If successful returns the number of bytes read, otherwise returns 0.
						/// There MUST NOT be an open file handle to the specified file because this function does that internally.
						/// The specified buffer MUST be null, as it gets new'd. Garbage collection MUST be handled by you.
inline size_t			readEntireFile( const char* filename, char** buffer );

						/// \brief Reads the next specified size of the file.
						///
						/// If successful, returns the number of bytes read, otherwise returns 0.
						/// The specified file handle MUST be valid and open when you call this function.
inline size_t			readFile( file_t fileHandle, const void* data, const size_t size );

						/// \brief Writes the specified buffer into the file, overwriting all previous content.
						///
						/// Returns true if the write was successful, otherwise returns false.
inline bool32			writeEntireFile( const char* filename, const void* data, const size_t size );

						/// \brief Appends the specified buffer into the file, adding it on to the end.
						///
						/// Returns true if the write was successful, otherwise returns false.
						/// The specified file handle MUST be valid and open when you call this function.
inline bool32			appendFile( file_t fileHandle, const void* data, const size_t size );

/*
========================
openFile
========================
*/
file_t openFile( const char* filename ) {
	assertf( filename, "Null file name was specified!" );

#if MSTD_OS_WINDOWS
	DWORD genericFlags = GENERIC_READ | GENERIC_WRITE;
	DWORD fileShareFlags = FILE_SHARE_READ | FILE_SHARE_WRITE;

	HANDLE handle = CreateFile( filename, genericFlags, fileShareFlags, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

	if ( handle == INVALID_HANDLE_VALUE ) {
		return nullptr;
	}

	return reinterpret_cast<file_t>( handle );
#endif
}

/*
========================
openOrCreateFile
========================
*/
file_t openOrCreateFile( const char* filename ) {
	assertf( filename, "Null file name was specified!" );

#if MSTD_OS_WINDOWS
	file_t fileHandle = openFile( filename );

	if ( fileHandle ) {
		return fileHandle;
	} else {
		DWORD genericFlags = GENERIC_READ | GENERIC_WRITE;
		DWORD fileShareFlags = FILE_SHARE_READ | FILE_SHARE_WRITE;

		HANDLE handle = CreateFile( filename, genericFlags, fileShareFlags, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

		if ( handle == INVALID_HANDLE_VALUE ) {
			return nullptr;
		}

		return reinterpret_cast<file_t>( handle );
	}
#endif
}

/*
========================
closeFile
========================
*/
bool32 closeFile( file_t fileHandle ) {
	assertf( fileHandle, "Null file handle was specified!" );

#if MSTD_OS_WINDOWS
	bool32 result = CloseHandle( reinterpret_cast<HANDLE>( fileHandle ) );

	if ( !result ) {
		return false;
	}
#endif

	fileHandle = nullptr;

	return true;
}

/*
========================
readEntireFile
========================
*/
size_t readEntireFile( const char* filename, char** buffer ) {
	assertf( filename, "Specified file name to read from was null!" );
	assertf( !*buffer, "Buffer must be null because this function news it!" );

	char* temp = nullptr;
	size_t size = 0;

	file_t fileHandle = openFile( filename );

	if ( !fileHandle ) {
		return 0;
	}

#if MSTD_OS_WINDOWS
	HANDLE handle = reinterpret_cast<HANDLE>( fileHandle );

	LARGE_INTEGER largeInt = {};
	GetFileSizeEx( handle, &largeInt );
	size = largeInt.QuadPart;

	temp = new char[size + 1];
	*buffer = temp;

	size_t bytesRead = readFile( fileHandle, temp, size );

	if ( bytesRead == 0 ) {
		return 0;
	}
#endif

	closeFile( fileHandle );

	temp[size] = 0;

	return bytesRead;
}

/*
========================
readFile
========================
*/
size_t readFile( file_t fileHandle, const void* data, const size_t size ) {
	assertf( fileHandle, "Null file handle was specified!" );
	assertf( data, "Null data was specified!" );

	if ( size == 0 ) {
		return 0;
	}

#if MSTD_OS_WINDOWS
	DWORD bytesRead = 0;
	DWORD bytesToRead = static_cast<DWORD>( size );

	bool32 result = ReadFile( reinterpret_cast<HANDLE>( fileHandle ), const_cast<void*>( data ), bytesToRead, &bytesRead, nullptr );

	if ( !result || bytesRead != bytesToRead ) {
		return 0;
	}

	return size;
#endif
}

/*
========================
writeEntireFile
========================
*/
bool32 writeEntireFile( const char* filename, const void* data, const size_t size ) {
	assertf( filename, "Specified file name to write to was null!" );
	assertf( data, "Specified data to write was null!" );
	assertf( size > 0, "Specified write size was <= 0!" );

	file_t fileHandle = openFile( filename );

	if ( !fileHandle ) {
		return false;
	}

#if MSTD_OS_WINDOWS
	HANDLE handle = reinterpret_cast<HANDLE>( fileHandle );

	bool32 result = appendFile( fileHandle, data, size );

	if ( !result ) {
		return false;
	}

	CloseHandle( handle );

	return true;
#endif
}

/*
========================
appendFile
========================
*/
bool32 appendFile( file_t fileHandle, const void* data, const size_t size ) {
	assertf( fileHandle, "Null file handle was specified!" );
	assertf( data, "Null data was specified!" );
	
	if ( size == 0 ) {
		return false;
	}

#if MSTD_OS_WINDOWS
	DWORD bytesWritten = 0;
	DWORD bytesToWrite = static_cast<DWORD>( size );

	// c style cast here because "reinterpret_const_cast<T*>( ptr )" doesnt exist
	bool32 result = WriteFile( reinterpret_cast<HANDLE>( fileHandle ), (char*) data, bytesToWrite, &bytesWritten, nullptr );

	if ( !result || bytesWritten != bytesToWrite ) {
		return false;
	}
#endif

	return true;
}

#endif