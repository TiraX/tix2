// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: reference additional headers your program requires here
#include "TiX.h"
#include "rapidjson/document.h"
using namespace rapidjson;

#define ReturnEnumValue(Str, EnumValue) if (Str == #EnumValue) {return EnumValue;}

namespace tix
{
	inline E_VERTEX_STREAM_SEGMENT GetVertexSegment(const TString& name)
	{
		ReturnEnumValue(name, EVSSEG_POSITION);
		ReturnEnumValue(name, EVSSEG_NORMAL);
		ReturnEnumValue(name, EVSSEG_COLOR);
		ReturnEnumValue(name, EVSSEG_TEXCOORD0);
		ReturnEnumValue(name, EVSSEG_TEXCOORD1);
		ReturnEnumValue(name, EVSSEG_TANGENT);
		ReturnEnumValue(name, EVSSEG_BLENDINDEX);
		ReturnEnumValue(name, EVSSEG_BLENDWEIGHT);

		return EVSSEG_POSITION;
	}

	inline E_BLEND_MODE GetMode(const TString& name)
	{
		ReturnEnumValue(name, BLEND_MODE_OPAQUE);
		ReturnEnumValue(name, BLEND_MODE_TRANSLUCENT);
		ReturnEnumValue(name, BLEND_MODE_MASK);
		ReturnEnumValue(name, BLEND_MODE_ADDITIVE);

		return BLEND_MODE_OPAQUE;
	}
}