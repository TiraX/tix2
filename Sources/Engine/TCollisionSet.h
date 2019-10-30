/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	// TMeshBuffer, hold mesh vertex and index data memory in game thread
	class TI_API TCollisionSet : public TResource
	{
	public:
		struct TSphere
		{
			TSphere()
				: Radius(1.f)
			{}

			vector3df Center;
			float Radius;
		};
		struct TBox
		{
			vector3df Center;
			quaternion Rotation;
			vector3df Edge;
		};
		struct TCapsule
		{
			TCapsule()
				: Radius(1.f)
				, Length(1.f)
			{}

			vector3df Center;
			quaternion Rotation;
			float Radius;
			float Length;
		};
		struct TConvex
		{
			TVector<vector3df> VertexData;
			TVector<uint16> IndexData;
		};

		TCollisionSet();
		~TCollisionSet();

		TMeshBufferPtr ConvertToMesh() const;

	public:
		virtual void InitRenderThreadResource() override;
		virtual void DestroyRenderThreadResource() override;

	protected:

	protected:
		TVector<TCollisionSet::TSphere> Spheres;
		TVector<TCollisionSet::TBox> Boxes;
		TVector<TCollisionSet::TCapsule> Capsules;
		TVector<TCollisionSet::TConvex> Convexes;

		friend class TAssetFile;
	};
}
