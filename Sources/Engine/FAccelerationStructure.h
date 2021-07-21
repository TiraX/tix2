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

		virtual void Build() = 0;

		void MarkDirty()
		{
			Dirty = true;
		}

		bool IsDirty() const
		{
			return Dirty;
		}
	protected:

	protected:
		bool Dirty;
	};

	/////////////////////////////////////////////////////////////

	class FBottomLevelAccelerationStructure : public FAccelerationStructure
	{
	public:
		FBottomLevelAccelerationStructure();
		virtual ~FBottomLevelAccelerationStructure();

		virtual void AddMeshBuffer(FMeshBufferPtr InMeshBuffer) = 0;
	protected:
	};

	/////////////////////////////////////////////////////////////

	class FTopLevelAccelerationStructure : public FAccelerationStructure
	{
	public:
		FTopLevelAccelerationStructure();
		virtual ~FTopLevelAccelerationStructure();

		virtual void ClearAllInstances() = 0;
		virtual void AddBLASInstance(FBottomLevelAccelerationStructurePtr BLAS, const FMatrix3x4& Transform) = 0;

	protected:
	};

}