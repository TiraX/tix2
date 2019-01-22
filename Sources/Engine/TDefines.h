/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once


#if defined (DEBUG) || defined (_DEBUG)
#	define TIX_DEBUG
#endif

#if defined (TI_PLATFORM_WIN32)
// overload operator new to debug.
#   define DEBUG_OPERATOR_NEW 0
#   if DEBUG_OPERATOR_NEW
void * operator new (std::size_t count);
#   endif
#endif

#ifdef TIX_DEBUG
#   if defined (TI_PLATFORM_WIN32)
#	    define ti_new new( _NORMAL_BLOCK, __FILE__, __LINE__)
#   else
#       define ti_new new
#   endif
// Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the
//allocations to be of _CLIENT_BLOCK type
#	define ti_delete delete
#else
#	define ti_new   new
#	define ti_delete delete
#endif // _DEBUG

#ifndef TI_ASSERT
#	ifdef TIX_DEBUG 
#		define TI_ASSERT(x) assert(x)
#	else
#		define TI_ASSERT(cond)
#	endif
#endif

#ifndef TI_BREAK
#	define	TI_BREAK	TI_ASSERT(0)
#endif

#define DO_STRINGIZE(x) #x
#define STRINGIZE(x) DO_STRINGIZE(x)
#define TODO_MESSAGE_STRING(msg) __FILE__ "(" STRINGIZE( __LINE__ ) ") : TODO - " ##msg " - [ " __FUNCTION__ " ]"
#if defined TI_PLATFORM_WIN32
#	define TI_TODO(msg) __pragma( message( TODO_MESSAGE_STRING(msg) ) )
#else
#	define TI_TODO(msg) 
#endif

#ifndef SAFE_DELETE
#	define SAFE_DELETE(x) if(x) { ti_delete x; x = nullptr; }
#endif
#ifndef SAFE_DELETE_ARRAY
#	define SAFE_DELETE_ARRAY(x) if(x) { ti_delete[] x; x = nullptr; }
#endif


#define TI_INTRUSIVE_PTR(T) IInstrusivePtr< T >

#define TI_MAKE_IDENTIFIER(c1, c2, c3, c4) ((c4 << 24) | (c3 << 16) | (c2 << 8) | (c1))

#define TI_CIRCLE_POINTS	(21)

#if defined (TI_PLATFORM_WIN32)
#	if defined (TI_LINK_STATIC_LIBRARY)
#		define TI_API 
#	else
#		ifdef TIX_EXPORTS
#			define TI_API __declspec(dllexport)
#		else
#			define TI_API __declspec(dllimport)
#		endif
#	endif
#else
#	define TI_API
#endif

// Define DEBUG System
#ifdef TIX_DEBUG
#	define TIX_DEBUG_RENDER_TASK_NAME 1
#else
#	define TIX_DEBUG_RENDER_TASK_NAME 0
#endif

