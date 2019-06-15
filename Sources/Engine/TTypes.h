/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

typedef unsigned char uint8;
typedef char int8;

typedef unsigned short uint16;
typedef short int16;

typedef unsigned int uint32;
typedef int int32;

#if defined (TI_PLATFORM_WIN32)
typedef __int64 int64;
typedef unsigned __int64 uint64;
#else
typedef unsigned long long uint64;
typedef long long int64;
#endif

typedef float float32;
typedef double float64;

#define TVector std::vector
#define TList std::list
#define TMap std::map
#define THMap std::unordered_map
#define TSort std::sort

typedef std::string TString;
typedef std::wstring TWString;
typedef std::stringstream TStringStream;

typedef std::mutex TMutex;
typedef std::condition_variable TCond;
typedef std::thread::id TThreadId;

#define tix_find std::find

#include "TInputEventType.h"
#include "TTypeCull.h"
#include "TTypeRenderer.h"
#include "TTypeRenderResource.h"
#include "TTypeNode.h"
#include "TTypePixelFormat.h"
#include "TTypeScene.h"

#include "SColor.h"
//#include <TiUString.h>

