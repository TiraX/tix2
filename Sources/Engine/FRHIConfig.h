/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_UNIFORMBUFFER_SECTION
	{
		UB_SECTION_NORMAL,
		UB_SECTION_LIGHTS,

		UB_SECTION_COUNT,
	};
	enum E_RHI_FEATURE
	{
		RHI_FEATURE_RAYTRACING = 1 << 0,
	};
	class FRHIConfig
	{
	public: 
		static const int32 FrameBufferNum = 3;	// Use triple buffers

		static const E_PIXEL_FORMAT DefaultBackBufferFormat = EPF_BGRA8;
#if COMPILE_WITH_RHI_DX12
		static const E_PIXEL_FORMAT DefaultDepthBufferFormat = EPF_DEPTH24_STENCIL8;
		static const E_PIXEL_FORMAT DefaultStencilBufferFormat = EPF_UNKNOWN;
#elif COMPILE_WITH_RHI_METAL
		static const E_PIXEL_FORMAT DefaultDepthBufferFormat = EPF_DEPTH24_STENCIL8;
		static const E_PIXEL_FORMAT DefaultStencilBufferFormat = EPF_UNKNOWN;
#else
	#error("unknown platform.")
#endif

		static const int32 StaticSamplerNum = 1;

		void EnableFeature(E_RHI_FEATURE InFeature, bool enable)
		{
			if (enable && IsFeatureSupported(InFeature))
				FeatureEnabledFlag |= InFeature;
			else
				FeatureEnabledFlag &= ~InFeature;
		}

		bool IsFeatureSupported(E_RHI_FEATURE InFeature) const
		{
			return (SupportedFeatures & InFeature) != 0;
		}

		bool IsRaytracingEnabled() const
		{
			return (FeatureEnabledFlag & RHI_FEATURE_RAYTRACING) != 0;
		}

	private:

	private:
		uint64 SupportedFeatures = 0;
		// Features On/Off flag
		uint64 FeatureEnabledFlag = 0;

		friend class FRHI;
	};
}
