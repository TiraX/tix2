/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#define Grass_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
	"CBV(b0, visibility=SHADER_VISIBILITY_VERTEX), " \
	"CBV(b1, visibility=SHADER_VISIBILITY_VERTEX), " \
	"CBV(b4, visibility=SHADER_VISIBILITY_PIXEL), " \
	"CBV(b5, visibility=SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t0, numDescriptors = 1), visibility = SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s0, addressU = TEXTURE_ADDRESS_WRAP, " \
                      "addressV = TEXTURE_ADDRESS_WRAP, " \
                      "addressW = TEXTURE_ADDRESS_WRAP, " \
                        "filter = FILTER_MIN_MAG_MIP_LINEAR, "\
                    "visibility = SHADER_VISIBILITY_PIXEL)"
