/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if defined (TI_PLATFORM_IOS)
#   define RES_CONVERTER_CONST const
#else
#   define RES_CONVERTER_CONST
#endif
int32 DoConvert(int32 argc, RES_CONVERTER_CONST int8* argv[]);
