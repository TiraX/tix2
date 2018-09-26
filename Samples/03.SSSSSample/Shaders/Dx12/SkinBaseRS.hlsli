//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#define SkinBase_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT), " \
    "DescriptorTable(CBV(b0, numDescriptors = 1), visibility = SHADER_VISIBILITY_VERTEX), " \
    "DescriptorTable(CBV(b1, numDescriptors = 1), visibility = SHADER_VISIBILITY_PIXEL), " \
    "DescriptorTable(SRV(t0, numDescriptors = 5), visibility = SHADER_VISIBILITY_PIXEL)," \
    "StaticSampler(s0, visibility = SHADER_VISIBILITY_PIXEL)," 
