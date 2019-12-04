/*
	TiX Engine v2.0 Copyright (C) 2018~2020
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_GPU_COMMAND_TYPE
	{
		GPU_COMMAND_SET_VERTEX_BUFFER,
		GPU_COMMAND_SET_INSTANCE_BUFFER,
		GPU_COMMAND_SET_INDEX_BUFFER,
		GPU_COMMAND_DRAW_INDEXED,
		GPU_COMMAND_DISPATCH,
		GPU_COMMAND_CONSTANT,
		GPU_COMMAND_CONSTANT_BUFFER,
		GPU_COMMAND_SHADER_RESOURCE,

		GPU_COMMAND_TYPE_COUNT
	};

	// Indirect drawing GPU command signature, contain command structure in GPU command buffer
	class FGPUCommandSignature : public FRenderResource
	{
	public:
		FGPUCommandSignature(FPipelinePtr InPipeline, const TVector<E_GPU_COMMAND_TYPE>& InCommandStructure);
		virtual ~FGPUCommandSignature();

		TI_API virtual uint32 GetCommandStrideInBytes() const
		{
			return 0;
		}
		const TVector<E_GPU_COMMAND_TYPE>& GetCommandStructure() const
		{
			return CommandStructure;
		}

		FPipelinePtr GetPipeline()
		{
			return Pipeline;
		}

	private:

	protected:
		FPipelinePtr Pipeline;
		TVector<E_GPU_COMMAND_TYPE> CommandStructure;
	};
} // end namespace tix
