/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if defined (TI_PLATFORM_IOS)
#   define VT_TEXTURE_BAKER_CONST const
#else
#   define VT_TEXTURE_BAKER_CONST
#endif
int32 DoBake(int32 argc, VT_TEXTURE_BAKER_CONST int8* argv[]);
