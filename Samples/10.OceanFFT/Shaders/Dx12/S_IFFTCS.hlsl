//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#define IFFT_RootSig \
	"CBV(b0) ," \
    "DescriptorTable(SRV(t0, numDescriptors=2), UAV(u0, numDescriptors=1))" 

static const float PI = 3.14159f;
static const float GRAVITY = 9.8f;

cbuffer FInfoUniform : register(b0)
{
    float4 Info;    // x = Dir(0=Hor;1=Ver); y = Stage;
};

Texture2D<float2> SourceTexture : register(t0);
Texture2D<float4> ButterFlyTexture : register(t1);

RWTexture2D<float2> ResultTexture : register(u0);

float2 complex_mul(in float2 c0, in float2 c1)
{
    float2 c;
    c.x = c0.x * c1.x - c0.y * c1.y;
    c.y = c0.x * c1.y + c0.y * c1.x;
    return c;
}

void IFFTHorizontal(in uint2 ThreadId)
{
    int Stage = (int)(Info.y);

    float4 Data = ButterFlyTexture.Load(int3(Stage, ThreadId.x, 0));
    float2 P_ = SourceTexture.Load(int3(Data.z, ThreadId.y, 0)).xy;
    float2 Q_ = SourceTexture.Load(int3(Data.w, ThreadId.y, 0)).xy;
    float2 W = Data.xy;

    // Do ButterFly
    float2 H = P_ + complex_mul(W, Q_);
    ResultTexture[ThreadId] = H;
}

void IFFTVertical(in uint2 ThreadId)
{
    int Stage = (int)(Info.y);

    float4 Data = ButterFlyTexture.Load(int3(Stage, ThreadId.y, 0));
    float2 P_ = SourceTexture.Load(int3(ThreadId.x, Data.z, 0)).xy;
    float2 Q_ = SourceTexture.Load(int3(ThreadId.x, Data.w, 0)).xy;
    float2 W = Data.xy;

    // Do ButterFly
    float2 H = P_ + complex_mul(W, Q_);
    ResultTexture[ThreadId] = H;
}

[RootSignature(IFFT_RootSig)]
[numthreads(32, 32, 1)]
void main(uint3 groupId : SV_GroupID, uint3 threadIDInGroup : SV_GroupThreadID, uint3 dispatchThreadId : SV_DispatchThreadID)
{
    const float Dir = Info.x;
    
    if (Dir == 0)
        IFFTHorizontal(dispatchThreadId.xy);
    else
        IFFTVertical(dispatchThreadId.xy);
}
