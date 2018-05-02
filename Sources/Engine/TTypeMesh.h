/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_MB_TYPES
	{
		EMBT_UNKNOWN,
		EMBT_Dx12,
		EMBT_Metal,
		EMBT_Vulkan
	};

	enum E_INDEX_TYPE 
	{
		EIT_16BIT,
		EIT_32BIT,
	};

	enum E_VERTEX_STREAM_SEGMENT
	{
		EVSSEG_POSITION		= 1,
		EVSSEG_NORMAL		= EVSSEG_POSITION << 1,
		EVSSEG_COLOR		= EVSSEG_NORMAL << 1,
		EVSSEG_TEXCOORD0	= EVSSEG_COLOR << 1,
		EVSSEG_TEXCOORD1	= EVSSEG_TEXCOORD0 << 1,
		EVSSEG_TANGENT		= EVSSEG_TEXCOORD1 << 1,
		EVSSEG_BLENDINDEX	= EVSSEG_TANGENT << 1,
		EVSSEG_BLENDWEIGHT	= EVSSEG_BLENDINDEX << 1,

		EVSSEG_TOTAL		= EVSSEG_BLENDWEIGHT,
	};

	enum E_SHADER_STREAM_INDEX
	{
		ESSI_POSITION		= 0,
		ESSI_NORMAL,
		ESSI_COLOR,
		ESSI_TEXCOORD0,
		ESSI_TEXCOORD1,
		ESSI_TANGENT,
		ESSI_BLENDINDEX,
		ESSI_BLENDWEIGHT,

		ESSI_TOTAL,
	};


//#define GL_POINTS                         0x0000
//#define GL_LINES                          0x0001
//#define GL_LINE_LOOP                      0x0002
//#define GL_LINE_STRIP                     0x0003
//#define GL_TRIANGLES                      0x0004
//#define GL_TRIANGLE_STRIP                 0x0005
//#define GL_TRIANGLE_FAN                   0x0006
//#define GL_QUADS                          0x0007
//#define GL_QUAD_STRIP                     0x0008
//#define GL_POLYGON                        0x0009
	enum E_PRIMITIVE_TYPE
	{
		EPT_POINTLIST,
		EPT_LINES,
		EPT_LINE_LOOP,
		EPT_LINESTRIP,
		EPT_TRIANGLELIST,
		EPT_TRIANGLESTRIP,
		EPT_TRIANGLE_FAN,
		EPT_QUADS,
		EPT_QUAD_STRIP,
		EPT_POLYGON,
	};
}