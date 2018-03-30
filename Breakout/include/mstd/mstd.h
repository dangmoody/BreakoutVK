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
#ifndef __MSTD_H__
#define __MSTD_H__

#ifdef _WIN32
// _CRT_SECURE_NO_WARNINGS
#pragma warning( disable : 4996 )
#endif

#include "mstd_types.h"
#include "mstd_debug.h"
#include "mstd_defines.h"
#include "mstd_math.h"
#include "mstd_random.h"
#include "mstd_array.h"
#include "mstd_string.h"
#include "mstd_file.h"
#include "mstd_stack.h"
#include "mstd_timer.h"

#ifdef _WIN32
// _CRT_SECURE_NO_WARNINGS
#pragma warning( default : 4996 )
#endif

#endif