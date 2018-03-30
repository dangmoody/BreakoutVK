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
#ifndef __MSTD_ARRAY_H__
#define __MSTD_ARRAY_H__

#include <initializer_list>

template<class T>
class array {
public:
						/// Initialises the array with a size of 0.
	inline				array();

						/// Initialises the array to be empty with the specified size.
	inline				array( const size_t length );

						/// Initialises the array to contain exactly the same content as the other array.
	inline				array( const array<T>& other );

						/// Initialises the array to contain the contents of the C-style array specified.
	inline				array( const T* ptr, const size_t length );

						/// Initialises the array to contain the contents of the initialiser list.
	inline				array( const std::initializer_list<T>& list );

	inline				~array();

						/// Returns the number of elements in the array.
	inline size_t		length() const;

						/// Returns the allocated size of the array in bytes.
	inline size_t		allocatedBytes() const;

						/// Returns the size of the object that the array contains, in bytes.
	inline u32			stride() const;

						/// Returns a pointer to the array.
	inline T*			data();
	inline const T*		data() const;

						/// Returns the first element of the array.
	inline T&			first();
	inline const T&		first() const;

						/// Returns the last element of the array.
	inline T&			last();
	inline const T&		last() const;

						/// DO NOT USE ME! This function exists only to support range-based for loops. Use "first()" instead.
	inline T*			begin();
	inline const T*		begin() const;

						/// DO NOT USE ME! This function exists only to support range-based for loops. Use "last()" instead.
	inline T*			end();
	inline const T*		end() const;

						/// Returns another array that contains the elements of this array between the start and end elements.
	inline array<T>		getRange( const size_t start, const size_t end ) const;

						/// Adds an element to the array.
	inline T&			add( const T& element );

						/// Adds a C-style array onto the end of the array.
	inline T&			addRange( const T* ptr, const size_t length );

						/// Adds another array onto the end of the array.
	inline T&			addRange( const array<T>& other );

						/// Inserts the element at the specified index of the array and pushes all the other elements up one, if any.
	inline T&			insert( const T& element, const size_t index );

						/// Inserts a C-style array into the specified index of the array and pushes all the other elements up, if any.
	inline T&			insertRange( const T* ptr, const size_t length, const size_t startIndex );

						/// Inserts another array into the specified index of the array and pushes all the other elements up, if any.
	inline T&			insertRange( const array<T>& other, const size_t startIndex );

						/// Removes the element at the specified index.
	inline T&			removeAt( const size_t index );

						/// Attempts to remove the element specified and returns true if successful, otherwise returns false.
	inline bool32		removeElement( const T& element );

						/// Removes all elements between the specified start and end indices.
	inline T&			removeRange( const size_t start, const size_t end );

						/// Removes the first element of the array.
	inline T&			removeFirst();

						/// Removes the last element of the array.
	inline T&			removeLast();

						/// Sets the number of elements the array can hold to the specified size.
	inline void			resize( const size_t newLength );

						/// Sets the allocated size in bytes of the array to the specified size.
	inline void			reserve( const size_t newSizeBytes );

						/// Empties the array.
	inline void			clear();

						/// Empties the array and resizes it to the specified size.
	inline void			reset( const size_t newLength );

						/// Reverses the array.
	inline void			reverse();

						/// Returns the index of the specified element if it's in the array, otherwise returns -1.
	inline ptrdiff_t	indexOf( const T& element ) const;

						/// Returns true if the array contains the specified element, otherwise returns false.
	inline bool32		contains( const T& element ) const;

						/// Returns true if the array is size 0.
	inline bool32		empty() const;

						/// Sets the array to contain the same elements and have the same number of elements and allocated size of the other array.
	inline array<T>&	operator=( const array<T>& other );

						/// Sets the array to contain the content from the specified initialiser list.
	inline array<T>&	operator=( const std::initializer_list<T>& list );

						/// Returns the array with the elements from the specified array appended onto it.
	inline array<T>		operator+( const array<T>& other ) const;

						/// Appends the elements from the specified array onto the array.
	inline array<T>		operator+=( const array<T>& other );

						/// Appends the elements from the initialiser list onto the array.
	inline array<T>		operator+=( const std::initializer_list<T>& list );

						/// Returns the element at the specified index.
	inline T&			operator[]( const size_t index );
	inline const T&		operator[]( const size_t index ) const;

						/// Returns true if the array has the same elements, number of elements and allocated size as the other array.
	inline bool32		operator==( const array<T>& other ) const;

						/// Returns true if the array DOES NOT have the same elements, number of elements and allocated size as the other array.
	inline bool32		operator!=( const array<T>& other ) const;

private:
	T*					mPtr;
	size_t				mLength;
	size_t				mAlloc;

private:
	inline void			init( const size_t length );

	inline T&			get( const size_t index );
	inline const T&		get( const size_t index ) const;

	inline void			set( const array<T>& other );
	inline void			set( const T* ptr, const size_t length );

	inline void			realloc( const size_t size );
	inline void			copy( T* dest, const T* source, const size_t length );
};

#include "mstd_array.inl"
#include "mstd_array_copy.inl"

#endif