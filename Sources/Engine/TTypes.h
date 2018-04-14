/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

typedef unsigned char	uint8;
typedef char			int8;

typedef unsigned short	uint16;
typedef short			int16;

typedef unsigned int	uint32;
typedef int				int32;

typedef unsigned long long	uint64;
typedef long long			int64;

typedef float	f32;
typedef double	f64;

#define TVector		std::vector
#define TList		std::list
#define TMap		std::map

#define TString		std::string
#define TWString	std::wstring

#define tix_find	std::find

#include "TInputEventType.h"
#include "TTypeCull.h"
#include "TTypeRenderer.h"
#include "TTypeMesh.h"
#include "TTypeNode.h"
#include "TTypePixelFormat.h"

#include "SColor.h"
//#include <TiUString.h>

