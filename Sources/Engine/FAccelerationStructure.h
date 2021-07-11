/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// AccelerationStructure
	// A base class for bottom-level and top-level AS.
	class FAccelerationStructure : public FRenderResource
	{
	public:
		FAccelerationStructure();
		virtual ~FAccelerationStructure();

	protected:

	protected:
		bool IsDirty;
	};

	/////////////////////////////////////////////////////////////

	class FBottomLevelAccelerationStructure : public FAccelerationStructure
	{
	public:
		FBottomLevelAccelerationStructure();
		virtual ~FBottomLevelAccelerationStructure();

		virtual void AddMeshBuffer(FMeshBufferPtr InMeshBuffer) = 0;
		virtual void Build() = 0;
	protected:
	};

	/////////////////////////////////////////////////////////////

	class FTopLevelAccelerationStructure : public FAccelerationStructure
	{
	public:
		FTopLevelAccelerationStructure();
		virtual ~FTopLevelAccelerationStructure();

	protected:
	};

}