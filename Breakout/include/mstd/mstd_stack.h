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
#ifndef __MSTD_STACK_H__
#define __MSTD_STACK_H__

template<class T>
class Stack {
public:
						/// Initialises an empty stack.
	inline				Stack() {}

						/// Initialises the Stack to contain exactly the same content as the other Stack.
	inline				Stack( const Stack<T>& stack );

						/// Initialises a Stack with the specified length.
	inline				Stack( const size_t length );

						/// Initialises the array to contain the contents of the initialiser list.
	inline				Stack( const std::initializer_list<T>& list );

	inline				~Stack() {}

						/// Adds the specified item to the stack.
	inline void			push( const T& element );

						/// Returns the item at the top of the stack and removes it.
	inline T			pop();

						/// Returns the item at the top of the stack.
	inline T&			top() { return mData.last(); }
	inline const T&		top() const { return mData.last(); }

						/// Returns how many items are on the stack.
	inline size_t		length() const { return mData.length(); }

						/// Returns true if the stack does not contain any items.
	inline bool32		empty() const { return mData.empty(); }

						/// Sets the Stack to contain the same data as the other Stack.
	inline Stack<T>&	operator=( const Stack<T>& stack );

						/// Sets the Stack to contain the same data as the specified std::initializer_list.
	inline Stack<T>&	operator=( const std::initializer_list<T>& list );

						/// Returns true if the data in the stack matches the one specified.
	inline bool32		operator==( const Stack<T>& stack ) const { return mData == stack.mData; }

						/// Returns true if the data in the stack DOES NOT match the one specified.
	inline bool32		operator!=( const Stack<T>& stack ) const { return mData == stack.mData; }

private:
	array<T>			mData;

private:
	void				set( const Stack<T>& stack ) { mData = stack.mData; }
	void				set( const std::initializer_list<T>& list );
};

/*
========================
Stack<T>::Stack
========================
*/
template<class T>
Stack<T>::Stack( const size_t length ) {
	mData.reset( length );
}

/*
========================
Stack<T>::Stack
========================
*/
template<class T>
Stack<T>::Stack( const Stack<T>& stack ) {
	set( stack );
}

/*
========================
Stack<T>::Stack
========================
*/
template<class T>
Stack<T>::Stack( const std::initializer_list<T>& list ) {
	set( list );
}

/*
========================
Stack<T>::push
========================
*/
template<class T>
void Stack<T>::push( const T& element ) {
	mData.add( element );
}

/*
========================
Stack<T>::pop
========================
*/
template<class T>
T Stack<T>::pop() {
	assertf( mData.length() > 0, "Stack underflow!\n" );
	
	T result = mData.last();
	mData.removeLast();
	return result;
}

/*
========================
Stack<T>::operator=
========================
*/
template<class T>
Stack<T>& Stack<T>::operator=( const Stack<T>& stack ) {
	set( stack );
	return *this;
}

/*
========================
Stack<T>::operator=
========================
*/
template<class T>
Stack<T>& Stack<T>::operator=( const std::initializer_list<T>& list ) {
	set( list );
	return *this;
}

/*
========================
Stack<T>::set
========================
*/
template<class T>
void Stack<T>::set( const std::initializer_list<T>& list ) {
	size_t size = list.size();
	mData.resize( size );
	memcpy( mData.data(), list.begin(), size * mData.stride() );
}

#endif