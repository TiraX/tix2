/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once


#if defined (DEBUG) || defined (_DEBUG)
#	define TIX_DEBUG
#endif

#define ti_new new
#define ti_delete delete

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

#ifndef TI_DEBUG
#	ifdef TIX_DEBUG
#		define TI_DEBUG(cond) {int r = cond; TI_ASSERT(r == 0);}
#	else
#		define TI_DEBUG(cond) cond
#	endif
#endif

#define DO_STRINGIZE(x) #x
#define STRINGIZE(x) DO_STRINGIZE(x)
#define TODO_MESSAGE_STRING(msg) __FILE__ "(" STRINGIZE( __LINE__ ) ") : TODO - " ##msg " - [ " __FUNCTION__ " ]"
#if defined TI_PLATFORM_WIN32
#	define TI_TODO(msg) __pragma( message( TODO_MESSAGE_STRING(msg) ) )
#else
#	define TI_TODO
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

#ifdef TI_PLATFORM_WIN32
#ifdef TIX_EXPORTS
#	define TI_API __declspec(dllexport)
#else
#	define TI_API __declspec(dllimport)
#endif
#else
#	define TI_API
#endif

