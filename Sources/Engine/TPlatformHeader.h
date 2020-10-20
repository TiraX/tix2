/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if defined (TI_PLATFORM_WIN32)
// undef NOMINMAX for compile
#	ifndef NOMINMAX
#	define NOMINMAX
#	endif

#	include <Windows.h>
#endif

// dll export
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