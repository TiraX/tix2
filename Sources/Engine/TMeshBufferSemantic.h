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
}
