/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// Define in a single file for reference in different project.
	const int32 TMeshBuffer::SemanticSize[ESSI_TOTAL] =
	{
		12,	// ESSI_POSITION,
		4,	// ESSI_NORMAL,
		4,	// ESSI_COLOR,
		4,	// ESSI_TEXCOORD0,
		4,	// ESSI_TEXCOORD1,
		4,	// ESSI_TANGENT,
		4,	// ESSI_BLENDINDEX,
		4,	// ESSI_BLENDWEIGHT,
	};

	const int8* TMeshBuffer::SemanticName[ESSI_TOTAL] =
	{
		"POSITION",		// ESSI_POSITION,
		"NORMAL",		// ESSI_NORMAL,
		"COLOR",		// ESSI_COLOR,
		"TEXCOORD",		// ESSI_TEXCOORD0,
		"TEXCOORD",		// ESSI_TEXCOORD1,
		"TANGENT",		// ESSI_TANGENT,
		"BLENDINDEX",	// ESSI_BLENDINDEX,
		"BLENDWEIGHT",	// ESSI_BLENDWEIGHT,
	};

	const int32 TMeshBuffer::SemanticIndex[ESSI_TOTAL] =
	{
		0,		// ESSI_POSITION,
		0,		// ESSI_NORMAL,
		0,		// ESSI_COLOR,
		0,		// ESSI_TEXCOORD0,
		0,		// ESSI_TEXCOORD1,
		0,		// ESSI_TANGENT,
		0,		// ESSI_BLENDINDEX,
		0,		// ESSI_BLENDWEIGHT,
	};

	///////////////////////////////////////////////////////////
	const int32 TInstanceBuffer::SemanticSize[EISI_TOTAL] =
	{
		16,	// EISI_TRANSITION,
#if USE_HALF_FOR_INSTANCE_ROTATION
		8,	// EISI_ROT_SCALE_MAT0,
		8,	// EISI_ROT_SCALE_MAT1,
		8,	// EISI_ROT_SCALE_MAT2,
#else
		16,	// EISI_ROT_SCALE_MAT0,
		16,	// EISI_ROT_SCALE_MAT1,
		16,	// EISI_ROT_SCALE_MAT2,
#endif
	};

	const int8* TInstanceBuffer::SemanticName[EISI_TOTAL] =
	{
		"INS_TRANSITION",		// EISI_TRANSITION,
		"INS_TRANSFORM",	// EISI_ROT_SCALE_MAT0,
		"INS_TRANSFORM",	// EISI_ROT_SCALE_MAT1,
		"INS_TRANSFORM",	// EISI_ROT_SCALE_MAT2,
	};

	const int32 TInstanceBuffer::SemanticIndex[EISI_TOTAL] =
	{
		0,		// EISI_TRANSITION,
		0,		// EISI_ROT_SCALE_MAT0,
		1,		// EISI_ROT_SCALE_MAT1,
		2,		// EISI_ROT_SCALE_MAT2
	};
}
