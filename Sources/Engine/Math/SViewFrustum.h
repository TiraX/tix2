//-*-c++-*-
// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#pragma once
#ifndef __S_VIEW_FRUSTUM_H_INCLUDED__
#define __S_VIEW_FRUSTUM_H_INCLUDED__

namespace tix
{

	//! Defines the view frustum. That's the space visible by the camera.
	/** The view frustum is enclosed by 6 planes. These six planes share
		four points. A bounding box around these four points is also stored in
		this structure.
	*/
	struct SViewFrustum
	{
		enum VFPLANES
		{
			//! Far plane of the frustum. That is the plane farest away from the eye.
			VF_FAR_PLANE = 0,
			//! Near plane of the frustum. That is the plane nearest to the eye.
			VF_NEAR_PLANE,
			//! Left plane of the frustum.
			VF_LEFT_PLANE,
			//! Right plane of the frustum.
			VF_RIGHT_PLANE,
			//! Bottom plane of the frustum.
			VF_BOTTOM_PLANE,
			//! Top plane of the frustum.
			VF_TOP_PLANE,

			//! Amount of planes enclosing the view frustum. Should be 6.
			VF_PLANE_COUNT
		};

		//! Hold a copy of important transform matrices
		enum E_TRANSFORMATION_STATE_3
		{
			ETS_VIEW_PROJECTION_3 = ETS_COUNT,
			//ETS_VIEW_MODEL_INVERSE_3,
			//ETS_CURRENT_3,
			ETS_COUNT_3
		};

		//! Default Constructor
		SViewFrustum() {}

		//! This constructor creates a view frustum based on a projection and/or
		//! view matrix.
		SViewFrustum(const matrix4& mat);

		//! This constructor creates a view frustum based on a projection and/or
		//! view matrix.
		inline void setFrom(const matrix4& mat);

		//! transforms the frustum by the matrix
		/** \param mat Matrix by which the view frustum is transformed.*/
		void transform(const matrix4& mat);

		//! returns the point which is on the far left upper corner inside the the
		//! view frustum.
		vector3df getFarLeftUp() const;

		//! returns the point which is on the far left bottom corner inside the the
		//! view frustum.
		vector3df getFarLeftDown() const;

		//! returns the point which is on the far right top corner inside the the
		//! view frustum.
		vector3df getFarRightUp() const;

		//! returns the point which is on the far right bottom corner inside the the
		//! view frustum.
		vector3df getFarRightDown() const;

		//! returns the point which is on the far left upper corner inside the the
		//! view frustum.
		vector3df getNearLeftUp() const;

		//! returns the point which is on the far left bottom corner inside the the
		//! view frustum.
		vector3df getNearLeftDown() const;

		//! returns the point which is on the far right top corner inside the the
		//! view frustum.
		vector3df getNearRightUp() const;

		//! returns the point which is on the far right bottom corner inside the the
		//! view frustum.
		vector3df getNearRightDown() const;

		//! returns a bounding box enclosing the whole view frustum
		const aabbox3d<float32> &getBoundingBox() const;

		//! recalculates the bounding box member based on the planes
		inline void recalculateBoundingBox();

		//! update the given state's matrix
		void setTransformState( E_TRANSFORMATION_STATE state);

		//!
		bool testPlane(uint32 i, const aabbox3df& bbox) const;

		//!
		bool intersectsWithoutBoxTest(const aabbox3df& bbox) const;

		//!
		bool intersects3(const aabbox3df& bbox) const;

		//!
		bool intersectsWithoutBoxTest3(const aabbox3df& bbox) const;

		//!
		E_CULLING_RESULT intersectsExWithoutBoxTest3(const aabbox3df& bbox) const;

		//!
		bool intersects(const aabbox3df& bbox) const;

		//!
		bool testInsidePlane(uint32 i, const aabbox3df& bbox) const;

		//!
		bool isFullInsideWithoutBoxTest(const aabbox3df& bbox) const;

		//!
		bool isFullInside(const aabbox3df& bbox) const;

		//!
		E_CULLING_RESULT intersectsExWithoutBoxTest(const aabbox3df& bbox) const;

		//!
		E_CULLING_RESULT intersectsEx(const aabbox3df& bbox) const;

		//! the position of the camera
		vector3df CameraPosition;

		//! all planes enclosing the view frustum.
		plane3d<float32> Planes[VF_PLANE_COUNT];

		//! bounding box around the view frustum
		aabbox3d<float32> BoundingBox;

		//! Hold a copy of important transform matrices
		matrix4 Matrices[ETS_COUNT_3];
	};


	inline SViewFrustum::SViewFrustum(const matrix4& mat)
	{
		setFrom ( mat );
	}


	inline void SViewFrustum::transform(const matrix4& mat)
	{
		for (uint32 i=0; i<VF_PLANE_COUNT; ++i)
			mat.transformPlane(Planes[i]);

		mat.transformVect(CameraPosition);
		recalculateBoundingBox();
	}


	inline vector3df SViewFrustum::getFarLeftUp() const
	{
		vector3df p;
		Planes[SViewFrustum::VF_FAR_PLANE].getIntersectionWithPlanes(
			Planes[SViewFrustum::VF_TOP_PLANE],
			Planes[SViewFrustum::VF_LEFT_PLANE], p);

		return p;
	}

	inline vector3df SViewFrustum::getFarLeftDown() const
	{
		vector3df p;
		Planes[SViewFrustum::VF_FAR_PLANE].getIntersectionWithPlanes(
			Planes[SViewFrustum::VF_BOTTOM_PLANE],
			Planes[SViewFrustum::VF_LEFT_PLANE], p);

		return p;
	}

	inline vector3df SViewFrustum::getFarRightUp() const
	{
		vector3df p;
		Planes[SViewFrustum::VF_FAR_PLANE].getIntersectionWithPlanes(
			Planes[SViewFrustum::VF_TOP_PLANE],
			Planes[SViewFrustum::VF_RIGHT_PLANE], p);

		return p;
	}

	inline vector3df SViewFrustum::getFarRightDown() const
	{
		vector3df p;
		Planes[SViewFrustum::VF_FAR_PLANE].getIntersectionWithPlanes(
			Planes[SViewFrustum::VF_BOTTOM_PLANE],
			Planes[SViewFrustum::VF_RIGHT_PLANE], p);

		return p;
	}

	inline vector3df SViewFrustum::getNearLeftUp() const
	{
		vector3df p;
		Planes[SViewFrustum::VF_NEAR_PLANE].getIntersectionWithPlanes(
			Planes[SViewFrustum::VF_TOP_PLANE],
			Planes[SViewFrustum::VF_LEFT_PLANE], p);

		return p;
	}

	inline vector3df SViewFrustum::getNearLeftDown() const
	{
		vector3df p;
		Planes[SViewFrustum::VF_NEAR_PLANE].getIntersectionWithPlanes(
			Planes[SViewFrustum::VF_BOTTOM_PLANE],
			Planes[SViewFrustum::VF_LEFT_PLANE], p);

		return p;
	}

	inline vector3df SViewFrustum::getNearRightUp() const
	{
		vector3df p;
		Planes[SViewFrustum::VF_NEAR_PLANE].getIntersectionWithPlanes(
			Planes[SViewFrustum::VF_TOP_PLANE],
			Planes[SViewFrustum::VF_RIGHT_PLANE], p);

		return p;
	}

	inline vector3df SViewFrustum::getNearRightDown() const
	{
		vector3df p;
		Planes[SViewFrustum::VF_NEAR_PLANE].getIntersectionWithPlanes(
			Planes[SViewFrustum::VF_BOTTOM_PLANE],
			Planes[SViewFrustum::VF_RIGHT_PLANE], p);

		return p;
	}



	inline const aabbox3d<float32> &SViewFrustum::getBoundingBox() const
	{
		return BoundingBox;
	}

	inline void SViewFrustum::recalculateBoundingBox()
	{
		BoundingBox.reset ( CameraPosition );

		BoundingBox.addInternalPoint(getFarLeftUp());
		BoundingBox.addInternalPoint(getFarRightUp());
		BoundingBox.addInternalPoint(getFarLeftDown());
		BoundingBox.addInternalPoint(getFarRightDown());
	}

	/*
	//! This constructor creates a view frustum based on a projection
	//! and/or view matrix.
	inline void SViewFrustum::setFrom(const matrix4& mat)
	{
		// left clipping plane
		Planes[SViewFrustum::VF_LEFT_PLANE].Normal.X = -(mat(0,3) + mat(0,0));
		Planes[SViewFrustum::VF_LEFT_PLANE].Normal.Y = -(mat(1,3) + mat(1,0));
		Planes[SViewFrustum::VF_LEFT_PLANE].Normal.Z = -(mat(2,3) + mat(2,0));
		Planes[SViewFrustum::VF_LEFT_PLANE].D = -(mat(3,3) + mat(3,0));
		
		// right clipping plane
		Planes[SViewFrustum::VF_RIGHT_PLANE].Normal.X = -(mat(0,3) - mat(0,0));
		Planes[SViewFrustum::VF_RIGHT_PLANE].Normal.Y = -(mat(1,3) - mat(1,0));
		Planes[SViewFrustum::VF_RIGHT_PLANE].Normal.Z = -(mat(2,3) - mat(2,0));
		Planes[SViewFrustum::VF_RIGHT_PLANE].D =        -(mat(3,3) - mat(3,0));

		// top clipping plane
		Planes[SViewFrustum::VF_TOP_PLANE].Normal.X = -(mat(0,3) - mat(0,1));
		Planes[SViewFrustum::VF_TOP_PLANE].Normal.Y = -(mat(1,3) - mat(1,1));
		Planes[SViewFrustum::VF_TOP_PLANE].Normal.Z = -(mat(2,3) - mat(2,1));
		Planes[SViewFrustum::VF_TOP_PLANE].D =        -(mat(3,3) - mat(3,1));

		// bottom clipping plane
		Planes[SViewFrustum::VF_BOTTOM_PLANE].Normal.X = -(mat(0,3) + mat(0,1));
		Planes[SViewFrustum::VF_BOTTOM_PLANE].Normal.Y = -(mat(1,3) + mat(1,1));
		Planes[SViewFrustum::VF_BOTTOM_PLANE].Normal.Z = -(mat(2,3) + mat(2,1));
		Planes[SViewFrustum::VF_BOTTOM_PLANE].D =        -(mat(3,3) + mat(3,1));

		// near clipping plane
		Planes[SViewFrustum::VF_NEAR_PLANE].Normal.X = -mat(0,2);
		Planes[SViewFrustum::VF_NEAR_PLANE].Normal.Y = -mat(1,2);
		Planes[SViewFrustum::VF_NEAR_PLANE].Normal.Z = -mat(2,2);
		Planes[SViewFrustum::VF_NEAR_PLANE].D =        -mat(3,2);

		// far clipping plane
		Planes[SViewFrustum::VF_FAR_PLANE].Normal.X = -(mat(0,3) - mat(0,2));
		Planes[SViewFrustum::VF_FAR_PLANE].Normal.Y = -(mat(1,3) - mat(1,2));
		Planes[SViewFrustum::VF_FAR_PLANE].Normal.Z = -(mat(2,3) - mat(2,2));
		Planes[SViewFrustum::VF_FAR_PLANE].D =        -(mat(3,3) - mat(3,2));
		// normalize normals

		for (s32 i=0; i<6; ++i)
		{
			const float32 len = reciprocal_squareroot(
				Planes[i].Normal.getLengthSQ() );
			Planes[i].Normal *= len;
			Planes[i].D *= len;
		}

		// make bounding box
		recalculateBoundingBox();
	}
	*/

	inline void SViewFrustum::setFrom(const matrix4& mat)
	{
		// left clipping plane
		Planes[VF_LEFT_PLANE].Normal.setX(mat[3 ] + mat[0]);
		Planes[VF_LEFT_PLANE].Normal.setY(mat[7 ] + mat[4]);
		Planes[VF_LEFT_PLANE].Normal.setZ(mat[11] + mat[8]);
		Planes[VF_LEFT_PLANE].D =        mat[15] + mat[12];

		// right clipping plane
		Planes[VF_RIGHT_PLANE].Normal.setX(mat[3 ] - mat[0]);
		Planes[VF_RIGHT_PLANE].Normal.setY(mat[7 ] - mat[4]);
		Planes[VF_RIGHT_PLANE].Normal.setZ(mat[11] - mat[8]);
		Planes[VF_RIGHT_PLANE].D =        mat[15] - mat[12];

		// top clipping plane
		Planes[VF_TOP_PLANE].Normal.setX(mat[3 ] - mat[1]);
		Planes[VF_TOP_PLANE].Normal.setY(mat[7 ] - mat[5]);
		Planes[VF_TOP_PLANE].Normal.setZ(mat[11] - mat[9]);
		Planes[VF_TOP_PLANE].D =        mat[15] - mat[13];

		// bottom clipping plane
		Planes[VF_BOTTOM_PLANE].Normal.setX(mat[3 ] + mat[1]);
		Planes[VF_BOTTOM_PLANE].Normal.setY(mat[7 ] + mat[5]);
		Planes[VF_BOTTOM_PLANE].Normal.setZ(mat[11] + mat[9]);
		Planes[VF_BOTTOM_PLANE].D =        mat[15] + mat[13];

		// far clipping plane
		Planes[VF_FAR_PLANE].Normal.setX(mat[3 ] - mat[2]);
		Planes[VF_FAR_PLANE].Normal.setY(mat[7 ] - mat[6]);
		Planes[VF_FAR_PLANE].Normal.setZ(mat[11] - mat[10]);
		Planes[VF_FAR_PLANE].D =        mat[15] - mat[14];

		// near clipping plane
		Planes[VF_NEAR_PLANE].Normal.setX(mat[2]);
		Planes[VF_NEAR_PLANE].Normal.setY(mat[6]);
		Planes[VF_NEAR_PLANE].Normal.setZ(mat[10]);
		Planes[VF_NEAR_PLANE].D =        mat[14];

		// normalize normals
		uint32 i;
		for ( i=0; i != VF_PLANE_COUNT; ++i)
		{
			const float32 len = -TMath::ReciprocalSquareroot(
				Planes[i].Normal.getLengthSQ());
			Planes[i].Normal *= len;
			Planes[i].D *= len;
		}

		// make bounding box
		recalculateBoundingBox();
	}

	inline void SViewFrustum::setTransformState(E_TRANSFORMATION_STATE state)
	{
		switch ( state )
		{
			case ETS_VIEW:
				Matrices[ETS_VIEW_PROJECTION_3].setbyproduct_nocheck(
					Matrices[ETS_PROJECTION],
					Matrices[ETS_VIEW]);
				//Matrices[ETS_VIEW_MODEL_INVERSE_3] = Matrices[ETS_VIEW];
				//Matrices[ETS_VIEW_MODEL_INVERSE_3].makeInverse();
				break;

			case ETS_WORLD:
				//Matrices[ETS_CURRENT_3].setbyproduct(
				//	Matrices[ETS_VIEW_PROJECTION_3 ],
				//	Matrices[ETS_WORLD]);
				break;
			default:
				break;
		}
	}

	inline bool SViewFrustum::testPlane(uint32 i, const aabbox3df& bbox) const
	{
		//http://www.lighthouse3d.com/tutorials/view-frustum-culling/geometric-approach-testing-boxes-ii/
		// get the "nearest" corner to the frustum along Planes[i]'s normal
		vector3df
			p(Planes[i].Normal.getX() >= 0 ? bbox.MinEdge.getX() : bbox.MaxEdge.getX(),
			  Planes[i].Normal.getY() >= 0 ? bbox.MinEdge.getY() : bbox.MaxEdge.getY(),
			  Planes[i].Normal.getZ() >= 0 ? bbox.MinEdge.getZ() : bbox.MaxEdge.getZ());

		// if the nearest corner to the frustum is outside, then the bbox is
		// outside.
		if (Planes[i].getDistanceTo(p) > 0)
		{
			return false;
		}
		return true;
	}

	inline bool SViewFrustum::intersectsWithoutBoxTest(const aabbox3df& bbox) const
	{
		for (uint32 i = 0; i < VF_PLANE_COUNT; ++i)
		{
			if (!testPlane(i, bbox))
			{
				return false;
			}
		}
		return true;
	}

	inline bool SViewFrustum::intersects(const aabbox3df& bbox) const
	{
		if (BoundingBox.intersectsWithBox(bbox))
		{
			return intersectsWithoutBoxTest(bbox);
		}
		return false;
	}

	inline bool SViewFrustum::testInsidePlane(uint32 i, const aabbox3df& bbox) const
	{
		// get the "farthest" corner to the frustum along Planes[i]'s normal
		vector3df
			p(Planes[i].Normal.getX() >= 0 ? bbox.MaxEdge.getX() : bbox.MinEdge.getX(),
			  Planes[i].Normal.getY() >= 0 ? bbox.MaxEdge.getY() : bbox.MinEdge.getY(),
			  Planes[i].Normal.getZ() >= 0 ? bbox.MaxEdge.getZ() : bbox.MinEdge.getZ());

		// if the nearest corner to the frustum is outside, then the bbox is
		// outside.
		if (Planes[i].getDistanceTo(p) > 0)
		{
			return false;
		}
		return true;
	}

	inline bool SViewFrustum::isFullInsideWithoutBoxTest(const aabbox3df& bbox) const
	{
		for (uint32 i = 0; i < VF_PLANE_COUNT; ++i)
		{
			if (!testInsidePlane(i, bbox))
			{
				return false;
			}
		}
		return true;
	}

	inline bool SViewFrustum::isFullInside(const aabbox3df& bbox) const
	{
		if (bbox.isFullInside(BoundingBox))
		{
			return isFullInsideWithoutBoxTest(bbox);
		}
		return false;
	}

	inline bool SViewFrustum::intersectsWithoutBoxTest3(const aabbox3df& bbox) const
	{
		if (testPlane(VF_LEFT_PLANE, bbox)
			&& testPlane(VF_RIGHT_PLANE, bbox)
			&& testPlane(VF_FAR_PLANE, bbox))
		{
			return true;
		}
		return false;
	}

	inline bool SViewFrustum::intersects3(const aabbox3df& bbox) const
	{
		if (BoundingBox.intersectsWithBox(bbox))
		{
			return intersectsWithoutBoxTest3(bbox);
		}
		return false;
	}

	inline E_CULLING_RESULT SViewFrustum::intersectsExWithoutBoxTest3(const aabbox3df& bbox) const
	{
		E_CULLING_RESULT result = ECR_INSIDE;
		// p: "nearest" corner to the frustum along Planes[i]'s normal
		// n: farthest
		vector3df p, n;
		static const VFPLANES planes[] = {VF_FAR_PLANE, VF_LEFT_PLANE, VF_RIGHT_PLANE};
		for (int j = 0; j < 3; ++j)
		{
			const VFPLANES i = planes[j];
			if (Planes[i].Normal.getX() >= 0)
			{
				p.setX(bbox.MinEdge.getX());
				n.setX(bbox.MaxEdge.getX());
			}
			else
			{
				p.setX(bbox.MaxEdge.getX());
				n.setX(bbox.MinEdge.getX());
			}

			if (Planes[i].Normal.getY() >= 0)
			{
				p.setY(bbox.MinEdge.getY());
				n.setY(bbox.MaxEdge.getY());
			}
			else
			{
				p.setY(bbox.MaxEdge.getY());
				n.setY(bbox.MinEdge.getY());
			}

			if (Planes[i].Normal.getZ() >= 0)
			{
				p.setZ(bbox.MinEdge.getZ());
				n.setZ(bbox.MaxEdge.getZ());
			}
			else
			{
				p.setZ(bbox.MaxEdge.getZ());
				n.setZ(bbox.MinEdge.getZ());
			}

			// if the nearest point to the frustum is outside, then the bbox is
			// outside.
			if (Planes[i].getDistanceTo(p) > 0)
			{
				return ECR_OUTSIDE;
			}
			// here, p is inside, thus if n is outside, we intersect
			if (Planes[i].getDistanceTo(n) > 0)
			{
				result = ECR_INTERSECT;
			}
		}
		return result;
	}

	inline E_CULLING_RESULT SViewFrustum::intersectsExWithoutBoxTest(const aabbox3df& bbox) const
	{
		E_CULLING_RESULT result = ECR_INSIDE;
		// p: "nearest" corner to the frustum along Planes[i]'s normal
		// n: farthest
		vector3df p, n;
		for (uint32 i = 0; i < VF_PLANE_COUNT; ++i)
		{
			if (Planes[i].Normal.getX() >= 0)
			{
				p.setX(bbox.MinEdge.getX());
				n.setX(bbox.MaxEdge.getX());
			}
			else
			{
				p.setX(bbox.MaxEdge.getX());
				n.setX(bbox.MinEdge.getX());
			}

			if (Planes[i].Normal.getY() >= 0)
			{
				p.setY(bbox.MinEdge.getY());
				n.setY(bbox.MaxEdge.getY());
			}
			else
			{
				p.setY(bbox.MaxEdge.getY());
				n.setY(bbox.MinEdge.getY());
			}

			if (Planes[i].Normal.getZ() >= 0)
			{
				p.setZ(bbox.MinEdge.getZ());
				n.setZ(bbox.MaxEdge.getZ());
			}
			else
			{
				p.setZ(bbox.MaxEdge.getZ());
				n.setZ(bbox.MinEdge.getZ());
			}

			// if the nearest point to the frustum is outside, then the bbox is
			// outside.
			if (Planes[i].getDistanceTo(p) > 0)
			{
				return ECR_OUTSIDE;
			}
			// here, p is inside, thus if n is outside, we intersect
			if (Planes[i].getDistanceTo(n) > 0)
			{
				result = ECR_INTERSECT;
			}
		}
		return result;
	}

	inline E_CULLING_RESULT SViewFrustum::intersectsEx(const aabbox3df& bbox) const
	{
		if (BoundingBox.intersectsWithBox(bbox))
		{
			return intersectsExWithoutBoxTest(bbox);
		}
		return ECR_OUTSIDE;
	}

} // end namespace ti

#endif

