/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	BEGIN_UNIFORM_BUFFER_STRUCT(FDynamicLightUniformBuffer)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, LightPosition)
		DECLARE_UNIFORM_BUFFER_STRUCT_MEMBER(FFloat4, LightColor)
	END_UNIFORM_BUFFER_STRUCT(FDynamicLightUniformBuffer)

	class FLight : public IReferenceCounted
	{
	public:
		FLight(TNodeLight * Light);
		~FLight();

		FDynamicLightUniformBufferPtr DynamicLightBuffer;
		void SetLightIndex(uint32 Index)
		{
			LightIndex = Index;
		}
		uint32 GetLightIndex() const
		{
			return LightIndex;
		}
	protected:
		void InitFromLightNode(TNodeLight * Light);
		void InitRenderResource_RenderThread();

	protected:
		uint32 LightIndex;	// The index light allocated in FSceneLights
	};
} // end namespace tix

