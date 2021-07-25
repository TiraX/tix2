/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

BEGIN_UNIFORM_BUFFER_STRUCT(FSplatParam)
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, SplatInfo0)		// xy = mouse_pos in uv space; zw = mouse move dir
	DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, SplatInfo1)		// xyz = color; z = radius scale
END_UNIFORM_BUFFER_STRUCT(FSplatParam)