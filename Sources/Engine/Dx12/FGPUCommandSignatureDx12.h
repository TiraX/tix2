/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

#if COMPILE_WITH_RHI_DX12
namespace tix
{
	// Indirect drawing GPU command buffer
	class FGPUCommandSignatureDx12 : public FGPUCommandSignature
	{
	public:
		static const uint32 GPU_COMMAND_STRIDE[GPU_COMMAND_TYPE_COUNT];

		FGPUCommandSignatureDx12(FPipelinePtr InPipeline, const TVector<E_GPU_COMMAND_TYPE>& CommandStructure);
		virtual ~FGPUCommandSignatureDx12();

		virtual uint32 GetCommandStrideInBytes() const
		{
			return CommandStrideInBytes;
		}

		uint32 GetArgumentStrideOffset(uint32 Index) const
		{
			return ArgumentStrideOffset[Index];
		}
	private:

	private:
		uint32 CommandStrideInBytes;
		TVector<uint32> ArgumentStrideOffset;

		ComPtr<ID3D12CommandSignature> CommandSignature;
		friend class FRHIDx12;
	};
} // end namespace tix
#endif