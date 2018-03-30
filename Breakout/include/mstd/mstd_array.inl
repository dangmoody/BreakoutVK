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

/*
========================
array<T>::array
========================
*/
template<class T>
array<T>::array() {
	init( 0 );
}

/*
========================
array<T>::array
========================
*/
template<class T>
array<T>::array( const size_t length ) {
	init( length );
}

/*
========================
array<T>::array
========================
*/
template<class T>
array<T>::array( const array<T>& other ) {
	init( 0 );
	set( other );
}

/*
========================
array<T>::array
========================
*/
template<class T>
array<T>::array( const T* ptr, const size_t length ) {
	init( 0 );
	set( ptr, length );
}

/*
========================
array<T>::array
========================
*/
template<class T>
array<T>::array( const std::initializer_list<T>& list ) {
	// Ugh, I hate supporting STL shit.
	// So fucking stupid.
	init( 0 );
	addRange( list.begin(), list.size() );
}

/*
========================
array<T>::~array
========================
*/
template<class T>
array<T>::~array() {
	if ( mPtr ) {
		delete[] mPtr;
		mPtr = nullptr;
	}

	mLength = mAlloc = 0;
}

/*
========================
array<T>::length
========================
*/
template<class T>
size_t array<T>::length() const {
	return mLength;
}

/*
========================
array<T>::allocatedBytes
========================
*/
template<class T>
size_t array<T>::allocatedBytes() const {
	return mAlloc;
}

/*
========================
array<T>::stride
========================
*/
template<class T>
u32 array<T>::stride() const {
	return sizeof( T );
}

/*
========================
array<T>::data
========================
*/
template<class T>
T* array<T>::data() {
	return mPtr;
}

/*
========================
array<T>::data
========================
*/
template<class T>
const T* array<T>::data() const {
	return mPtr;
}

/*
========================
array<T>::first
========================
*/
template<class T>
T& array<T>::first() {
	return mPtr[0];
}

/*
========================
array<T>::first
========================
*/
template<class T>
const T& array<T>::first() const {
	return mPtr[0];
}

/*
========================
array<T>::last
========================
*/
template<class T>
T& array<T>::last() {
	return mPtr[mLength - 1];
}

/*
========================
array<T>::last
========================
*/
template<class T>
const T& array<T>::last() const {
	return mPtr[mLength - 1];
}

/*
========================
array<T>::begin
========================
*/
template<class T>
T* array<T>::begin() {
	return &mPtr[0];
}

/*
========================
array<T>::begin
========================
*/
template<class T>
const T* array<T>::begin() const {
	return &mPtr[0];
}

/*
========================
array<T>::end
========================
*/
template<class T>
T* array<T>::end() {
	return &mPtr[mLength];
}

/*
========================
array<T>::end
========================
*/
template<class T>
const T* array<T>::end() const {
	return &mPtr[mLength];
}

/*
========================
array<T>::getRange
========================
*/
template<class T>
array<T> array<T>::getRange( const size_t start, const size_t end ) const {
	assertf( start >= 0, "You tried to get part of an array where the start index was less than 0!\n" );
	assertf( end > start, "You tried to get part of an array where the end point was less than the start point!\n" );
	assertf( end < mLength, "You tried to get part of an array where the end point was bigger than the length of the array!\n" );
	size_t range = end - start;
	array<T> result( range );
	copy( result.mPtr, mPtr, range );
	return result;
}

/*
========================
array<T>::add
========================
*/
template<class T>
T& array<T>::add( const T& element ) {
	size_t oldSize = mLength;
	resize( oldSize + 1 );
	T* newSlot = mPtr + oldSize;
	*newSlot = element;
	return *newSlot;
}

/*
========================
array<T>::addRange
========================
*/
template<class T>
T& array<T>::addRange( const T* ptr, const size_t length ) {
	assertf( ptr != nullptr, "Cannot append an array with a null ptr!\n" );
	size_t oldSize = mLength;
	resize( oldSize + length );
	copy( mPtr + oldSize, ptr, length );
	T* newSlot = mPtr + mLength;
	return *newSlot;
}

/*
========================
array<T>::addRange
========================
*/
template<class T>
T& array<T>::addRange( const array<T>& other ) {
	return addRange( other.mPtr, other.mLength );
}

/*
========================
array<T>::insert
========================
*/
template<class T>
T& array<T>::insert( const T& element, const size_t index ) {
	assertf( index < mLength, "You can't insert an element into an array at an index which is greater than it's size!\n" );
	size_t oldSize = mLength;
	resize( oldSize + 1 );
	T* moveSlot = mPtr + index;
	copy( moveSlot + 1, moveSlot, oldSize - index );
	*moveSlot = element;
	return *moveSlot;
}

/*
========================
array<T>::insertRange
========================
*/
template<class T>
T& array<T>::insertRange( const T* ptr, const size_t length, const size_t startIndex ) {
	assertf( ptr != nullptr, "You tried to insert a null pointer into an array!" );
	assertf( startIndex < mLength, "You can't insert an array into another array at an index which is greater than it's size!\n" );
	size_t oldSize = mLength;
	resize( oldSize + length );
	T* moveSlot = mPtr + startIndex;
	copy( moveSlot + length, moveSlot, oldSize - startIndex );
	copy( moveSlot, ptr, length );
	return *moveSlot;
}

/*
========================
array<T>::insertRange
========================
*/
template<class T>
T& array<T>::insertRange( const array<T>& other, const size_t startIndex ) {
	return insertRange( other.mPtr, other.mLength, startIndex );
}

/*
========================
array<T>::removeAt
========================
*/
template<class T>
T& array<T>::removeAt( const size_t index ) {
	assertf( mLength > 0, "You tried to remove an element from an array that was empty!\n" );
	assertf( index < mLength, "You tried to remove an element from an array but the index was bigger than the size of it!\n" );
	size_t oldSize = mLength;
	T* moveSlot = mPtr + index;
	copy( moveSlot, moveSlot + 1, oldSize - index );
	resize( mLength - 1 );
	return *moveSlot;
}

/*
========================
array<T>::removeElement
========================
*/
template<class T>
bool32 array<T>::removeElement( const T& element ) {
	size_t index = indexOf( element );
	if ( index == -1 ) {
		return false;
	} else {
		removeAt( index );
		return true;
	}
}

/*
========================
array<T>::remove
========================
*/
template<class T>
T& array<T>::removeRange( const size_t start, const size_t end ) {
	assertf( start >= 0, "You tried to remove a section of an array where the start index is less than 0!\n" );
	assertf( end > start, "You tried to remove a section of an array where the end index is less than the start index!\n" );
	size_t oldSize = mLength;
	T* moveSlot = mPtr + start;
	copy( moveSlot, moveSlot + end, oldSize - index );
	resize( mSize - 1 );
	return *moveSlot;
}

/*
========================
array<T>::removeFirst
========================
*/
template<class T>
T& array<T>::removeFirst() {
	removeAt( 0 );
	return first();
}

/*
========================
array<T>::removeLast
========================
*/
template<class T>
T& array<T>::removeLast() {
	removeAt( mLength - 1 );
	return last();
}

/*
========================
array<T>::resize
========================
*/
template<class T>
void array<T>::resize( const size_t newSize ) {
	if ( newSize > mAlloc ) {
		realloc( newSize );
	}

	mLength = newSize;
}

/*
========================
array<T>::reserve
========================
*/
template<class T>
void array<T>::reserve( const size_t newSizeBytes ) {
	if ( newSizeBytes > mAlloc ) {
		realloc( newSizeBytes );
	}
}

/*
========================
array<T>::clear
========================
*/
template<class T>
void array<T>::clear() {
	reset( 0 );
}

/*
========================
array<T>::reset
========================
*/
template<class T>
void array<T>::reset( const size_t newLength ) {
	if ( mAlloc != newLength ) {
		realloc( max( newLength, mAlloc ) );
	}
	mLength = newLength;
}

/*
========================
array<T>::reverse
========================
*/
template<class T>
void array<T>::reverse() {
	if ( !mPtr ) {
		return;
	}

	size_t start = 0;
	size_t end = mLength - 1;
	T temp = mPtr[end];
	while ( start < end ) {
		temp = mPtr[start];
		mPtr[start] = mPtr[end];
		mPtr[end] = temp;

		start++;
		end--;
	}
}

/*
========================
array<T>::indexOf
========================
*/
template<class T>
ptrdiff_t array<T>::indexOf( const T& element ) const {
	for ( size_t i = 0; i < mLength; i++ ) {
		if ( mPtr[i] == element ) {
			return i;
		}
	}

	return -1;
}

/*
========================
array<T>::contains
========================
*/
template<class T>
bool32 array<T>::contains( const T& element ) const {
	return indexOf( element ) != -1;
}

/*
========================
array<T>::contains
========================
*/
template<class T>
bool32 array<T>::empty() const {
	return mLength == 0;
}

/*
========================
array<T>::operator=
========================
*/
template<class T>
array<T>& array<T>::operator=( const array<T>& other ) {
	set( other );
	return *this;
}

/*
========================
array<T>::operator=
========================
*/
template<class T>
array<T>& array<T>::operator=( const std::initializer_list<T>& list ) {
	clear();
	addRange( list.begin(), list.size() );
	return *this;
}

/*
========================
array<T>::operator+
========================
*/
template<class T>
array<T> array<T>::operator+( const array<T>& other ) const {
	array<T> result = *this;
	result.addRange( other );
	return result;
}

/*
========================
array<T>::operator+=
========================
*/
template<class T>
array<T> array<T>::operator+=( const array<T>& other ) {
	addRange( other );
	return *this;
}

/*
========================
array<T>::operator+=
========================
*/
template<class T>
array<T> array<T>::operator+=( const std::initializer_list<T>& list ) {
	T* start = (T*) list.begin();
	start = start;
	addRange( list.begin(), list.size() );
	return *this;
}

/*
========================
array<T>::operator[]
========================
*/
template<class T>
T& array<T>::operator[]( const size_t index ) {
	return get( index );
}

/*
========================
array<T>::operator[]
========================
*/
template<class T>
const T& array<T>::operator[]( const size_t index ) const {
	return get( index );
}

/*
========================
array<T>::operator==
========================
*/
template<class T>
bool32 array<T>::operator==( const array<T>& other ) const {
	if ( mLength != other.mLength ) {
		return false;
	}

	return memcmp( mPtr, other.mPtr, mLength * sizeof( T ) ) == 0;
}

/*
========================
array<T>::operator!=
========================
*/
template<class T>
bool32 array<T>::operator!=( const array<T>& other ) const {
	return !( operator==( other ) );
}

/*
========================
array<T>::init
========================
*/
template<class T>
void array<T>::init( const size_t length ) {
	mPtr = length > 0 ? new T[length] : nullptr;
	mLength = mAlloc = length;
	memset( mPtr, 0, mLength * sizeof( T ) );
}

/*
========================
array<T>::get
========================
*/
template<class T>
T& array<T>::get( const size_t index ) {
	assertf( index >= 0, "You tried to access an array element at an index that was less than 0!\n" );
	assertf( index < mLength, "You tried to access an array element that was greater than the size of the array!\n" );
	return mPtr[index];
}

/*
========================
array<T>::get
========================
*/
template<class T>
const T& array<T>::get( const size_t index ) const {
	assertf( index >= 0, "You tried to access an array element at an index that was less than 0!\n" );
	assertf( index < mLength, "You tried to access an array element at an index that was greater than the size of the array!\n" );
	return mPtr[index];
}

/*
========================
array<T>::set
========================
*/
template<class T>
void array<T>::set( const array<T>& other ) {
	if ( *this == other ) {
		return;
	}

	set( other.mPtr, other.mLength );
}

/*
========================
array<T>::set
========================
*/
template<class T>
void array<T>::set( const T* ptr, const size_t length ) {
	if ( !ptr ) {
		return;
	}

	reset( length );
	copy( mPtr, ptr, length );
}

/*
========================
array<T>::realloc
========================
*/
template<class T>
void array<T>::realloc( const size_t newSize ) {
	size_t rounded = 1;
	while ( newSize > rounded ) {
		rounded <<= 1;
	}

	T* newPtr = nullptr;

	if ( rounded > 0 ) {
		newPtr = new T[rounded];
		if ( mPtr ) {
			copy( newPtr, mPtr, min( mLength, rounded ) );
		}
	}

	delete[] mPtr;
	mPtr = newPtr;
	mAlloc = rounded;
}