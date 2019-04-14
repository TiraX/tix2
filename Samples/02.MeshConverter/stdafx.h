// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>


// TODO: reference additional headers your program requires here
#include "TiX.h"
#include "TJSON.h"
#include "ResSettings.h"

#define ReturnEnumValue(Str, EnumValue) if (Str == #EnumValue) {return EnumValue;}

namespace tix
{
	class TJSON;

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

	inline E_INSTANCE_STREAM_SEGMENT GetInstanceSegment(const TString& name)
	{
		ReturnEnumValue(name, EINSSEG_POSITION);
		ReturnEnumValue(name, EINSSEG_TRANSFORM);

		return EINSSEG_TRANSFORM;
	}

	inline E_BLEND_MODE GetBlendMode(const TString& name)
	{
		ReturnEnumValue(name, BLEND_MODE_OPAQUE);
		ReturnEnumValue(name, BLEND_MODE_TRANSLUCENT);
		ReturnEnumValue(name, BLEND_MODE_MASK);
		ReturnEnumValue(name, BLEND_MODE_ADDITIVE);

		return BLEND_MODE_OPAQUE;
	}

	inline E_TEXTURE_TYPE GetTextureType(const TString& name)
	{
		ReturnEnumValue(name, ETT_TEXTURE_1D);
		ReturnEnumValue(name, ETT_TEXTURE_2D);
		ReturnEnumValue(name, ETT_TEXTURE_3D);
		ReturnEnumValue(name, ETT_TEXTURE_CUBE);

		return ETT_TEXTURE_2D;
	}

	inline E_TEXTURE_ADDRESS_MODE GetAddressMode(const TString& name)
	{
		ReturnEnumValue(name, ETC_REPEAT);
		ReturnEnumValue(name, ETC_CLAMP_TO_EDGE);
		ReturnEnumValue(name, ETC_MIRROR);

		return ETC_CLAMP_TO_EDGE;
	}

	inline E_PIXEL_FORMAT GetPixelFormat(const TString& name)
	{
		ReturnEnumValue(name, EPF_UNKNOWN);
		ReturnEnumValue(name, EPF_A8);
		ReturnEnumValue(name, EPF_RGBA8);
		ReturnEnumValue(name, EPF_RGBA8_SRGB);
		ReturnEnumValue(name, EPF_BGRA8);
		ReturnEnumValue(name, EPF_BGRA8_SRGB);
		ReturnEnumValue(name, EPF_R16F);
		ReturnEnumValue(name, EPF_RG16F);
		ReturnEnumValue(name, EPF_RGBA16F);
		ReturnEnumValue(name, EPF_R32F);
		ReturnEnumValue(name, EPF_RG32F);
		ReturnEnumValue(name, EPF_RGB32F);
		ReturnEnumValue(name, EPF_RGBA32F);
		ReturnEnumValue(name, EPF_DEPTH16);
		ReturnEnumValue(name, EPF_DEPTH32);
		ReturnEnumValue(name, EPF_DEPTH24_STENCIL8);
		ReturnEnumValue(name, EPF_STENCIL8);
		ReturnEnumValue(name, EPF_DDS_DXT1);
		ReturnEnumValue(name, EPF_DDS_DXT1_SRGB);
		ReturnEnumValue(name, EPF_DDS_DXT3);
		ReturnEnumValue(name, EPF_DDS_DXT3_SRGB);
		ReturnEnumValue(name, EPF_DDS_DXT5);
		ReturnEnumValue(name, EPF_DDS_DXT5_SRGB);
		ReturnEnumValue(name, EPF_DDS_BC5);
		ReturnEnumValue(name, EPF_ASTC4x4);
		ReturnEnumValue(name, EPF_ASTC4x4_SRGB);
		ReturnEnumValue(name, EPF_ASTC6x6);
		ReturnEnumValue(name, EPF_ASTC6x6_SRGB);
		ReturnEnumValue(name, EPF_ASTC8x8);
		ReturnEnumValue(name, EPF_ASTC8x8_SRGB);

		return EPF_UNKNOWN;
	}

	inline E_SHADER_STAGE GetShaderStage(const TString& name)
	{
		ReturnEnumValue(name, ESS_VERTEX_SHADER);
		ReturnEnumValue(name, ESS_PIXEL_SHADER);
		ReturnEnumValue(name, ESS_DOMAIN_SHADER);
		ReturnEnumValue(name, ESS_HULL_SHADER);
		ReturnEnumValue(name, ESS_GEOMETRY_SHADER);

		return ESS_VERTEX_SHADER;
	}

	inline E_BINDING_TYPE GetBindingType(const TString& name)
	{
		ReturnEnumValue(name, BINDING_UNIFORMBUFFER);
		ReturnEnumValue(name, BINDING_UNIFORMBUFFER_TABLE);
		ReturnEnumValue(name, BINDING_TEXTURE_TABLE);

		return BINDING_TYPE_INVALID;
	}

	inline E_STENCIL_OP GetStencilOp(const TString& name)
	{
		ReturnEnumValue(name, ESO_KEEP);
		ReturnEnumValue(name, ESO_ZERO);
		ReturnEnumValue(name, ESO_REPLACE);
		ReturnEnumValue(name, ESO_INCR_SAT);
		ReturnEnumValue(name, ESO_DECR_SAT);
		ReturnEnumValue(name, ESO_INVERT);
		ReturnEnumValue(name, ESO_INCR);
		ReturnEnumValue(name, ESO_DECR);

		return ESO_KEEP;
	}

	inline E_COMPARISON_FUNC GetComparisonFunc(const TString& name)
	{
		ReturnEnumValue(name, ECF_NEVER);
		ReturnEnumValue(name, ECF_LESS);
		ReturnEnumValue(name, ECF_LESS_EQUAL);
		ReturnEnumValue(name, ECF_EQUAL);
		ReturnEnumValue(name, ECF_GREATER);
		ReturnEnumValue(name, ECF_NOT_EQUAL);
		ReturnEnumValue(name, ECF_GREATER_EQUAL);
		ReturnEnumValue(name, ECF_ALWAYS);

		return ECF_ALWAYS;
	}
}
