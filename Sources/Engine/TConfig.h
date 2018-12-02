/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

// Platform
#if defined (_WINDOWS)
#	define TI_PLATFORM_WIN32
#elif defined (IPHONEOS)
#	define TI_PLATFORM_IOS
#elif defined (TI_ANDROID)
#	define TI_PLATFORM_ANDROID
#else
#error("do not support other platforms yet.")
#endif

// Renderers
#ifdef TI_PLATFORM_WIN32
#	define COMPILE_WITH_RHI_DX12	1
#elif defined (TI_PLATFORM_IOS)
#	define COMPILE_WITH_RHI_METAL 1
#endif

// We use right hand coordinate
#define TI_USE_RH 1

#if defined (TI_PLATFORM_WIN32)
// undef NOMINMAX for compile
#ifndef NOMINMAX
#	define NOMINMAX
#endif
#	include <Windows.h>
#endif