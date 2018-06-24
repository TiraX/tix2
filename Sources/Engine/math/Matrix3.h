// gameswf.h	-- Thatcher Ulrich <tu@tulrich.com> 2003

// This source code has been donated to the Public Domain.  Do
// whatever you want with it.

// SWF (Shockwave Flash) player library.  The info for this came from
// http://www.openswf.org, the flashsource project, and swfparse.cpp


#ifndef GAMESWF_H
#define GAMESWF_H

namespace tix
{
	//! 3x2 matrix. Mostly used as transformation matrix for 2d ui calculations.
	template <class T>
	class CMatrix3
	{
	public:
		CMatrix3();
		void	set_identity();
		void	concatenate(const CMatrix3<T>& m);
		void	concatenate_translation(T tx, T ty);
		void	concatenate_scale(T x, T y);
		void	set_lerp(const CMatrix3<T>& m1, const CMatrix3<T>& m2, float t);
		void	set_scale_rotation(T x_scale, T y_scale, T rotation);
		void	transform(vector2d<T>& result, const vector2d<T>& p) const;
		void	transform(rect<T>& bound) const;
		void	transform_vector(vector2d<T>& result, const vector2d<T>& p) const;
		void	transform_by_inverse(vector2d<T>& result, const vector2d<T>& p) const;
		void	transform_by_inverse(rect<T>& bound) const;
		void	set_inverse(const CMatrix3<T>& m);
		bool	does_flip() const;	// return true if we flip handedness
		T		get_determinant() const;	// determinant of the 2x2 rotation/scale part only
		T		get_max_scale() const;	// return the maximum scale factor that this transform applies
		T		get_x_scale() const;	// return the magnitude scale of our x coord output
		T		get_y_scale() const;	// return the magnitude scale of our y coord output
		T		get_rotation() const;	// return our rotation component (in radians)

		bool operator==(const CMatrix3<T>& m) const
		{
			return 
				m_[0][0] == m.m_[0][0] &&
				m_[0][1] == m.m_[0][1] &&
				m_[0][2] == m.m_[0][2] &&
				m_[1][0] == m.m_[1][0] &&
				m_[1][1] == m.m_[1][1] &&
				m_[1][2] == m.m_[1][2];
		}

		bool operator!=(const CMatrix3<T>& m) const
		{
			return ! (*this == m);
		}

	private:
		T		m_[2][3];
	};

	//! Typedef for float32 matrix
	typedef CMatrix3<float32> matrix3;

	template <class T>
	inline CMatrix3<T>::CMatrix3()
	{
		// Default to identity.
		set_identity();
	}


	template <class T>
	inline void CMatrix3<T>::set_identity()
		// Set the matrix to identity.
	{
		memset(&m_[0], 0, sizeof(m_));
		m_[0][0] = 1;
		m_[1][1] = 1;
	}

	template <class T>
	inline void CMatrix3<T>::concatenate(const CMatrix3<T>& m)
		// Concatenate m's transform onto ours.  When
		// transforming points, m happens first, then our
		// original xform.
	{
		CMatrix3<T>	t;
		t.m_[0][0] = m_[0][0] * m.m_[0][0] + m_[0][1] * m.m_[1][0];
		t.m_[1][0] = m_[1][0] * m.m_[0][0] + m_[1][1] * m.m_[1][0];
		t.m_[0][1] = m_[0][0] * m.m_[0][1] + m_[0][1] * m.m_[1][1];
		t.m_[1][1] = m_[1][0] * m.m_[0][1] + m_[1][1] * m.m_[1][1];
		t.m_[0][2] = m_[0][0] * m.m_[0][2] + m_[0][1] * m.m_[1][2] + m_[0][2];
		t.m_[1][2] = m_[1][0] * m.m_[0][2] + m_[1][1] * m.m_[1][2] + m_[1][2];

		*this = t;
	}

	template <class T>
	inline void CMatrix3<T>::concatenate_translation(T tx, T ty)
		// Concatenate a translation onto the front of our
		// matrix.  When transforming points, the translation
		// happens first, then our original xform.
	{
		m_[0][2] += m_[0][0] * tx + m_[0][1] * ty;
		m_[1][2] += m_[1][0] * tx + m_[1][1] * ty;
	}

	template <class T>
	inline void CMatrix3<T>::concatenate_scale(T x, T y)
		// Concatenate a uniform scale onto the front of our
		// matrix.  When transforming points, the scale
		// happens first, then our original xform.
	{
		//m_[0][0] *= scale;
		//m_[0][1] *= scale;
		//m_[1][0] *= scale;
		//m_[1][1] *= scale;		
		m_[0][0] *= x;
		m_[1][0] *= x;
		m_[0][1] *= y;
		m_[1][1] *= y;
	}

	template <class T>
	inline void CMatrix3<T>::set_lerp(const CMatrix3<T>& m1, const CMatrix3<T>& m2, float t)
		// Set this matrix to a blend of m1 and m2, parameterized by t.
	{
		m_[0][0] = TI_INTERPOLATE(m1.m_[0][0], m2.m_[0][0], t);
		m_[1][0] = TI_INTERPOLATE(m1.m_[1][0], m2.m_[1][0], t);
		m_[0][1] = TI_INTERPOLATE(m1.m_[0][1], m2.m_[0][1], t);
		m_[1][1] = TI_INTERPOLATE(m1.m_[1][1], m2.m_[1][1], t);
		m_[0][2] = TI_INTERPOLATE(m1.m_[0][2], m2.m_[0][2], t);
		m_[1][2] = TI_INTERPOLATE(m1.m_[1][2], m2.m_[1][2], t);
	}

	template <class T>
	inline void CMatrix3<T>::set_scale_rotation(T x_scale, T y_scale, T angle)
		// Set the scale & rotation part of the matrix.
		// angle in radians.
	{
		float	cos_angle = cosf(angle);
		float	sin_angle = sinf(angle);
		m_[0][0] = x_scale * cos_angle;
		m_[0][1] = y_scale * -sin_angle;
		m_[1][0] = x_scale * sin_angle;
		m_[1][1] = y_scale * cos_angle;
	}

	template <class T>
	inline void CMatrix3<T>::transform(vector2d<T>& result, const vector2d<T>& p) const
		// Transform point 'p' by our matrix.  Put the result in
		// *result.
	{
		assert(result);
		assert(&p != result);

		result.X	= m_[0][0] * p.X + m_[0][1] * p.Y + m_[0][2];
		result.Y	= m_[1][0] * p.X + m_[1][1] * p.Y + m_[1][2];
	}

	template <class T>
	inline void CMatrix3<T>::transform(rect<T>& bound) const
		// Transform bound our matrix.
	{
		// get corners of transformed bound
		vector2d<T> p[4];
		transform(p[0], vector2d<T>(bound.Left, bound.Upper));
		transform(p[1], vector2d<T>(bound.Right, bound.Upper));
		transform(p[2], vector2d<T>(bound.Right, bound.Lower));
		transform(p[3], vector2d<T>(bound.Left, bound.Lower));

		// Build bound that covers transformed bound
		bound.reset(p[0]);
		bound.addInternalPoint(p[1]);
		bound.addInternalPoint(p[2]);
		bound.addInternalPoint(p[3]);
	}

	template <class T>
	inline void CMatrix3<T>::transform_vector(vector2d<T>& result, const vector2d<T>& v) const
		// Transform vector 'v' by our matrix. Doesn't apply translation.
		// Put the result in *result.
	{
		assert(result);
		assert(&v != result);

		result.X	= m_[0][0] * v.X + m_[0][1] * v.Y;
		result.Y	= m_[1][0] * v.X + m_[1][1] * v.Y;
	}

	template <class T>
	inline void CMatrix3<T>::transform_by_inverse(vector2d<T>& result, const vector2d<T>& p) const
		// Transform point 'p' by the inverse of our matrix.  Put result in *result.
	{
		// @@ TODO optimize this!
		CMatrix3<T>	m;
		m.set_inverse(*this);
		m.transform(result, p);
	}

	template <class T>
	inline void CMatrix3<T>::transform_by_inverse(rect<T>& bound) const
		// Transform point 'p' by the inverse of our matrix.  Put result in *result.
	{
		// @@ TODO optimize this!
		CMatrix3<T>	m;
		m.set_inverse(*this);
		m.transform(bound);
	}

	template <class T>
	inline void CMatrix3<T>::set_inverse(const CMatrix3<T>& m)
		// Set this matrix to the inverse of the given matrix.
	{
		assert(this != &m);

		// Invert the rotation part.
		T	det = m.m_[0][0] * m.m_[1][1] - m.m_[0][1] * m.m_[1][0];
		if (det == 0.f)
		{
			// Not invertible.
			//assert(0);	// castano: this happens sometimes! (ie. sample6.swf)

			// Arbitrary fallback.
			set_identity();
			m_[0][2] = -m.m_[0][2];
			m_[1][2] = -m.m_[1][2];
		}
		else
		{
			float	inv_det = 1.0f / det;
			m_[0][0] = m.m_[1][1] * inv_det;
			m_[1][1] = m.m_[0][0] * inv_det;
			m_[0][1] = -m.m_[0][1] * inv_det;
			m_[1][0] = -m.m_[1][0] * inv_det;

			m_[0][2] = -(m_[0][0] * m.m_[0][2] + m_[0][1] * m.m_[1][2]);
			m_[1][2] = -(m_[1][0] * m.m_[0][2] + m_[1][1] * m.m_[1][2]);
		}
	}

	template <class T>
	inline bool CMatrix3<T>::does_flip() const
		// Return true if this matrix reverses handedness.
	{
		T	det = m_[0][0] * m_[1][1] - m_[0][1] * m_[1][0];

		return det < 0.f;
	}

	template <class T>
	inline T CMatrix3<T>::get_determinant() const
		// Return the determinant of the 2x2 rotation/scale part only.
	{
		return m_[0][0] * m_[1][1] - m_[1][0] * m_[0][1];
	}

	template <class T>
	inline T CMatrix3<T>::get_max_scale() const
		// Return the maximum scale factor that this transform
		// applies.  For assessing scale, when determining acceptable
		// errors in tesselation.
	{
		// @@ not 100% sure what the heck I'm doing here.  I
		// think this is roughly what I want; take the max
		// length of the two basis vectors.
		T	basis0_length2 = m_[0][0] * m_[0][0] + m_[0][1] * m_[0][1];
		T	basis1_length2 = m_[1][0] * m_[1][0] + m_[1][1] * m_[1][1];
		float	max_length2 = max(basis0_length2, basis1_length2);
		return sqrtf(max_length2);
	}

	template <class T>
	inline T CMatrix3<T>::get_x_scale() const
	{
		float scale = sqrtf(m_[0][0] * m_[0][0] + m_[1][0] * m_[1][0]);

		// Are we turned inside out?
		if (get_determinant() < 0.f)
		{
			scale = -scale;
		}

		return scale;
	}

	template <class T>
	inline T CMatrix3<T>::get_y_scale() const
	{
		return sqrtf(m_[1][1] * m_[1][1] + m_[0][1] * m_[0][1]);
	}

	template <class T>
	inline T CMatrix3<T>::get_rotation() const
	{
		if (get_determinant() < 0.f)
		{
			// We're turned inside out; negate the
			// x-component used to compute rotation.
			//
			// Matches get_x_scale().
			//
			// @@ this may not be how Macromedia does it!  Test this!
			return atan2f(m_[1][0], -m_[0][0]);
		}
		else
		{
			return atan2f(m_[1][0], m_[0][0]);
		}
	}

} // end namespace ti

#endif

