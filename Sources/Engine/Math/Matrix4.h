//-*-c++-*-
// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#pragma once
#ifndef __IRR_MATRIX_H_INCLUDED__
#define __IRR_MATRIX_H_INCLUDED__

namespace tix
{


	//all matrices types must have operator[]
	template<typename MT1, typename MT2, typename MT3>
	inline void rowMatrixProduct(MT1& out, const MT2& m1, const MT3& m2)
	{
		out[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
		out[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
		out[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
		out[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];

		out[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
		out[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
		out[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
		out[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];

		out[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
		out[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
		out[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
		out[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];

		out[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
		out[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
		out[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
		out[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];
	}

	//all matrices types must have operator[]
	template<typename MT1, typename MT2, typename MT3>
	static inline void rowMatrixProduct34(MT1& out, const MT2& m1, const MT3& m2)
	{
		out[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2];
		out[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2];
		out[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2];
		out[3] = 0;

		out[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6];
		out[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6];
		out[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6];
		out[7] = 0;

		out[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10];
		out[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10];
		out[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10];
		out[11] = 0;

		out[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12];
		out[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13];
		out[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14];
		out[15] = 1;
	}



	//! 4x4 matrix. Mostly used as transformation matrix for 3d calculations.
	/** The matrix is a D3D style matrix, row major with translations in the 4th row. */
	template <class T>
	class CMatrix4
	{
	public:

		//! Constructor Flags
		enum eConstructor
		{
			EM4CONST_NOTHING = 0,
			EM4CONST_COPY,
			EM4CONST_IDENTITY,
			EM4CONST_TRANSPOSED,
			EM4CONST_INVERSE,
			EM4CONST_INVERSE_TRANSPOSED
		};

		//! Default constructor
		/** \param constructor Choose the initialization style */
		CMatrix4( eConstructor constructor = EM4CONST_IDENTITY );

		//! Contructor with data
		CMatrix4( T _00, T _01, T _02, T _03,
				  T _10, T _11, T _12, T _13,
				  T _20, T _21, T _22, T _23,
				  T _30, T _31, T _32, T _33);

		//! Copy constructor
		/** \param other Other matrix to copy from
			\param constructor Choose the initialization style */
	#if 1
		CMatrix4( const CMatrix4<T>& other,eConstructor constructor = EM4CONST_COPY);
	#else
		CMatrix4( const CMatrix4<T>& other,eConstructor constructor);
	#endif
		//! Simple operator for directly accessing every element of the matrix.
		T& operator()(const int row, const int col) { definitelyIdentityMatrix=false; return M[ row * 4 + col ]; }

		//! Simple operator for directly accessing every element of the matrix.
		const T& operator()(const int row, const int col) const { return M[row * 4 + col]; }

		//! Simple operator for linearly accessing every element of the matrix.
		T& operator[](uint32 index) { definitelyIdentityMatrix=false; return M[index]; }

		//! Simple operator for linearly accessing every element of the matrix.
		const T& operator[](uint32 index) const { return M[index]; }

		//! Sets this matrix equal to the other matrix.
		inline CMatrix4<T>& operator=(const CMatrix4<T> &other);

		//! Sets all elements of this matrix to the value.
		inline CMatrix4<T>& operator=(const T& scalar);

		//! Returns pointer to internal array
		const T* pointer() const { return M; }
		T* pointer() { definitelyIdentityMatrix=false; return M; }

		//! Returns true if other matrix is equal to this matrix.
		bool operator==(const CMatrix4<T> &other) const;

		//! Returns true if other matrix is not equal to this matrix.
		bool operator!=(const CMatrix4<T> &other) const;

		//! Add another matrix.
		CMatrix4<T> operator+(const CMatrix4<T>& other) const;

		//! Add another matrix.
		CMatrix4<T>& operator+=(const CMatrix4<T>& other);

		//! Subtract another matrix.
		CMatrix4<T> operator-(const CMatrix4<T>& other) const;

		//! Subtract another matrix.
		CMatrix4<T>& operator-=(const CMatrix4<T>& other);

		//! set this matrix to the product of two matrices
		inline CMatrix4<T>& setbyproduct(const CMatrix4<T>& other_a,const CMatrix4<T>& other_b );

		//! Set this matrix to the product of two matrices
		/** no optimization used,
			use it if you know you never have a identity matrix */
		CMatrix4<T>& setbyproduct_nocheck(const CMatrix4<T>& other_a,const CMatrix4<T>& other_b );

		//! Multiply by another matrix.
		CMatrix4<T> operator*(const CMatrix4<T>& other) const;

		//! Multiply by another matrix.
		CMatrix4<T>& operator*=(const CMatrix4<T>& other);

		//! Multiply by another matrix as if both matrices where 3x4.
		CMatrix4<T> mult34(const CMatrix4<T>& m2) const;

		//! Multiply by another matrix as if both matrices where 3x4.
		CMatrix4<T>& mult34(const CMatrix4<T>& m2, CMatrix4<T>& out) const;

		//! Multiply by scalar.
		CMatrix4<T> operator*(const T& scalar) const;

		//! Multiply by scalar.
		CMatrix4<T>& operator*=(const T& scalar);

		//! Set matrix to identity.
		inline CMatrix4<T>& makeIdentity();

		//! Returns the c'th column of the matrix, without the lowest row.
		vector3d<T> getColumn(uint32 c) const;

		//! Returns the c'th column of the matrix.
		//vector4d<T> getFullColumn(uint32 c) const;

		//! Sets the c'th column of the matrix, without the lowest row.
		CMatrix4<T>& setColumn(uint32 c, const vector3d<T>& v);

		//! Sets the c'th column of the matrix.
		//CMatrix4<T>& setFullColumn(uint32 c, const vector4d<T>& v);

		//! Gets the current translation
		vector3d<T> getTranslation() const;

		//! Set the translation of the current matrix. Will erase any previous values.
		CMatrix4<T>& setTranslation( const vector3d<T>& translation );

		//! Set the inverse translation of the current matrix. Will erase any previous values.
		CMatrix4<T>& setInverseTranslation( const vector3d<T>& translation );

		//! Make a rotation matrix from Euler angles. The 4th row and column are unmodified.
		inline CMatrix4<T>& setRotationRadians( const vector3d<T>& rotation );

		//! Make a rotation matrix from Euler angles. The 4th row and column are unmodified.
		CMatrix4<T>& setRotationDegrees( const vector3d<T>& rotation );

		//! Returns the rotation, as set by setRotation().
		/** This code was orginally written by by Chev. */
		vector3d<T> getRotationDegrees() const;

		//! Make an inverted rotation matrix from Euler angles.
		/** The 4th row and column are unmodified. */
		inline CMatrix4<T>& setInverseRotationRadians( const vector3d<T>& rotation );

		//! Make an inverted rotation matrix from Euler angles.
		/** The 4th row and column are unmodified. */
		CMatrix4<T>& setInverseRotationDegrees( const vector3d<T>& rotation );

		//! Set Scale
		CMatrix4<T>& setScale( const vector3d<T>& scale );

		//! Set Scale
		CMatrix4<T>& setScale( const T scale ) { return setScale(vector3d<T>(scale,scale,scale)); }

		//! Apply scale to this matrix as if multiplication was on the left.
		CMatrix4<T>& preScale( const vector3d<T>& scale );

		//! Apply scale to this matrix as if multiplication was on the right.
		CMatrix4<T>& postScale( const vector3d<T>& scale );

		//! Get Scale
		vector3d<T> getScale() const;

		//! Translate a vector by the inverse of the translation part of this matrix.
		void inverseTranslateVect( vector3d<T>& vect ) const;

		//! Rotate a vector by the inverse of the rotation part of this matrix.
		void inverseRotateVect( vector3d<T>& vect ) const;

		//! Rotate a vector by the rotation part of this matrix.
		void rotateVect( vector3d<T>& vect ) const;

		//! An alternate transform vector method, writing into a second vector
		void rotateVect(vector3d<T>& out, const vector3d<T>& in) const;

		//! An alternate transform vector method, writing into an array of 3 floats
		void rotateVect(T *out,const vector3d<T> &in) const;

		//! Transforms the vector by this matrix
		void transformVect( vector3d<T>& vect) const;

		//! Transforms input vector by this matrix and stores result in output vector
		void transformVect( vector3d<T>& out, const vector3d<T>& in ) const;
		void transformVect( vector3d<T>& out, const T* in ) const;

		//! Transforms the vector by this matrix as though it was in 2D (Z ignored).
		void transformVect2D( vector3d<T>& vect) const;

		//! Transforms input vector by this matrix and stores result in output vector as though it was in 2D (Z ignored).
		void transformVect2D( vector3d<T>& out, const vector3d<T>& in ) const;

		//! An alternate transform vector method, writing into an array of 4 floats
		void transformVect(T *out,const vector3d<T> &in) const;

		//! Translate a vector by the translation part of this matrix.
		void translateVect( vector3d<T>& vect ) const;

		//! Transforms a plane by this matrix
		void transformPlane( plane3d<T> &plane) const;

		//! Transforms a plane by this matrix ( some problems to solve..)
		void transformPlane_new( plane3d<T> &plane) const;

		//! Transforms a plane by this matrix
		void transformPlane( const plane3d<T> &in, plane3d<T> &out) const;

		//! Transforms a axis aligned bounding box
		/** The result box of this operation may not be accurate at all. For
			correct results, use transformBoxEx() */
		void transformBox(aabbox3d<T>& box) const;

		//! Transforms a axis aligned bounding box
		/** The result box of this operation should by accurate, but this operation
			is slower than transformBox(). */
		void transformBoxEx(aabbox3d<T>& box) const;

		//! Multiplies this matrix by a 1x4 matrix
		void multiplyWith1x4Matrix(T* matrix) const;

		//! Calculates inverse of matrix. Slow.
		/** \return Returns false if there is no inverse matrix.*/
		bool makeInverse();

		//! Computes the determinant of the matrix.
		T getDeterminant() const;

		//! Inverts a primitive matrix which only contains a translation and a rotation
		/** \param out where result matrix is written to. */
		bool getInversePrimitive(CMatrix4<T>& out) const;

		//! Gets the inversed matrix of this one
		/** \param out where result matrix is written to.
			\return Returns false if there is no inverse matrix. */
		bool getInverse(CMatrix4<T>& out) const;

		//! Creates a newly matrix as interpolated matrix from two other ones.
		/** \param b other matrix to interpolate with
			\param time Must be a value between 0 and 1. */
		CMatrix4<T> interpolate(const CMatrix4<T>& b, T time) const;

		//! Gets transposed matrix
		CMatrix4<T> getTransposed() const;

		//! Gets transposed matrix
		inline void getTransposed( CMatrix4<T>& dest ) const;

		//! Set texture transformation rotation
		/** Rotate about z axis, recenter at (0.5,0.5).
			Doesn't clear other elements than those affected
			\param radAngle Angle in radians
			\return Altered matrix */
		CMatrix4<T>& setTextureRotationCenter( T radAngle );

		//! Set texture transformation translation
		/** Doesn't clear other elements than those affected.
			\param x Offset on x axis
			\param y Offset on y axis
			\return Altered matrix */
		CMatrix4<T>& setTextureTranslate( T x, T y );

		//! Set texture transformation translation, using a transposed representation
		/** Doesn't clear other elements than those affected.
			\param x Offset on x axis
			\param y Offset on y axis
			\return Altered matrix */
		CMatrix4<T>& setTextureTranslateTransposed( T x, T y );

		//! Set texture transformation scale
		/** Doesn't clear other elements than those affected.
			\param sx Scale factor on x axis
			\param sy Scale factor on y axis
			\return Altered matrix. */
		CMatrix4<T>& setTextureScale( T sx, T sy );

		//! Set texture transformation scale, and recenter at (0.5,0.5)
		/** Doesn't clear other elements than those affected.
			\param sx Scale factor on x axis
			\param sy Scale factor on y axis
			\return Altered matrix. */
		CMatrix4<T>& setTextureScaleCenter( T sx, T sy );

		//! Applies a texture post scale.
		/**	\param sx Scale factor on x axis
			\param sy Scale factor on y axis
			\return Altered matrix. */
		CMatrix4<T>& postTextureScale ( T sx, T sy );

		//! Sets all matrix data members at once
		CMatrix4<T>& setM(const T* data);

		//! Gets all matrix data members at once
		/** \returns data */
		T* getM(T* data) const;

		//! Sets if the matrix is definitely identity matrix
		void setDefinitelyIdentityMatrix( bool isDefinitelyIdentityMatrix);

		//! Gets if the matrix is definitely identity matrix
		bool getDefinitelyIdentityMatrix() const;

	private:
		//! Matrix data, stored in row-major order
		T M[16];
		//! Flag is this matrix is identity matrix
        union
        {
            mutable bool definitelyIdentityMatrix;
            int          _4bytesAlign;
        };
	};

	// Default constructor
	template <class T>
	inline CMatrix4<T>::CMatrix4( eConstructor constructor ) : definitelyIdentityMatrix(false)
	{
		switch ( constructor )
		{
			case EM4CONST_NOTHING:
			case EM4CONST_COPY:
				break;
			case EM4CONST_IDENTITY:
			case EM4CONST_INVERSE:
			default:
				makeIdentity();
				break;
		}
	}

	//! Contructor with data
	template <class T>
	inline CMatrix4<T>::CMatrix4(T _00, T _01, T _02, T _03, T _10, T _11, T _12, T _13, T _20, T _21, T _22, T _23, T _30, T _31, T _32, T _33)
	{
		definitelyIdentityMatrix	= false;
		M[0]	= _00;
		M[1]	= _01;
		M[2]	= _02;
		M[3]	= _03;
		
		M[4]	= _10;
		M[5]	= _11;
		M[6]	= _12;
		M[7]	= _13;
		
		M[8]	= _20;
		M[9]	= _21;
		M[10]	= _22;
		M[11]	= _23;
		
		M[12]	= _30;
		M[13]	= _31;
		M[14]	= _32;
		M[15]	= _33;
	}

	// Copy constructor
	template <class T>
	inline CMatrix4<T>::CMatrix4( const CMatrix4<T>& other, eConstructor constructor) : definitelyIdentityMatrix(false)
	{
		switch ( constructor )
		{
			case EM4CONST_IDENTITY:
				makeIdentity();
				break;
			case EM4CONST_NOTHING:
				break;
			case EM4CONST_COPY:
				*this = other;
				break;
			case EM4CONST_TRANSPOSED:
				other.getTransposed(*this);
				break;
			case EM4CONST_INVERSE:
				if (!other.getInverse(*this))
					memset(M, 0, 16*sizeof(T));
				break;
			case EM4CONST_INVERSE_TRANSPOSED:
				if (!other.getInverse(*this))
					memset(M, 0, 16*sizeof(T));
				else
					*this=getTransposed();
				break;
		}
	}

	//! Add another matrix.
	template <class T>
	inline CMatrix4<T> CMatrix4<T>::operator+(const CMatrix4<T>& other) const
	{
		CMatrix4<T> temp ( EM4CONST_NOTHING );

		temp[0] = M[0]+other[0];
		temp[1] = M[1]+other[1];
		temp[2] = M[2]+other[2];
		temp[3] = M[3]+other[3];
		temp[4] = M[4]+other[4];
		temp[5] = M[5]+other[5];
		temp[6] = M[6]+other[6];
		temp[7] = M[7]+other[7];
		temp[8] = M[8]+other[8];
		temp[9] = M[9]+other[9];
		temp[10] = M[10]+other[10];
		temp[11] = M[11]+other[11];
		temp[12] = M[12]+other[12];
		temp[13] = M[13]+other[13];
		temp[14] = M[14]+other[14];
		temp[15] = M[15]+other[15];

		return temp;
	}

	//! Add another matrix.
	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::operator+=(const CMatrix4<T>& other)
	{
		M[0]+=other[0];
		M[1]+=other[1];
		M[2]+=other[2];
		M[3]+=other[3];
		M[4]+=other[4];
		M[5]+=other[5];
		M[6]+=other[6];
		M[7]+=other[7];
		M[8]+=other[8];
		M[9]+=other[9];
		M[10]+=other[10];
		M[11]+=other[11];
		M[12]+=other[12];
		M[13]+=other[13];
		M[14]+=other[14];
		M[15]+=other[15];

		return *this;
	}

	//! Subtract another matrix.
	template <class T>
	inline CMatrix4<T> CMatrix4<T>::operator-(const CMatrix4<T>& other) const
	{
		CMatrix4<T> temp ( EM4CONST_NOTHING );

		temp[0] = M[0]-other[0];
		temp[1] = M[1]-other[1];
		temp[2] = M[2]-other[2];
		temp[3] = M[3]-other[3];
		temp[4] = M[4]-other[4];
		temp[5] = M[5]-other[5];
		temp[6] = M[6]-other[6];
		temp[7] = M[7]-other[7];
		temp[8] = M[8]-other[8];
		temp[9] = M[9]-other[9];
		temp[10] = M[10]-other[10];
		temp[11] = M[11]-other[11];
		temp[12] = M[12]-other[12];
		temp[13] = M[13]-other[13];
		temp[14] = M[14]-other[14];
		temp[15] = M[15]-other[15];

		return temp;
	}

	//! Subtract another matrix.
	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::operator-=(const CMatrix4<T>& other)
	{
		M[0]-=other[0];
		M[1]-=other[1];
		M[2]-=other[2];
		M[3]-=other[3];
		M[4]-=other[4];
		M[5]-=other[5];
		M[6]-=other[6];
		M[7]-=other[7];
		M[8]-=other[8];
		M[9]-=other[9];
		M[10]-=other[10];
		M[11]-=other[11];
		M[12]-=other[12];
		M[13]-=other[13];
		M[14]-=other[14];
		M[15]-=other[15];

		return *this;
	}

	//! Multiply by scalar.
	template <class T>
	inline CMatrix4<T> CMatrix4<T>::operator*(const T& scalar) const
	{
		CMatrix4<T> temp ( EM4CONST_NOTHING );

		temp[0] = M[0]*scalar;
		temp[1] = M[1]*scalar;
		temp[2] = M[2]*scalar;
		temp[3] = M[3]*scalar;
		temp[4] = M[4]*scalar;
		temp[5] = M[5]*scalar;
		temp[6] = M[6]*scalar;
		temp[7] = M[7]*scalar;
		temp[8] = M[8]*scalar;
		temp[9] = M[9]*scalar;
		temp[10] = M[10]*scalar;
		temp[11] = M[11]*scalar;
		temp[12] = M[12]*scalar;
		temp[13] = M[13]*scalar;
		temp[14] = M[14]*scalar;
		temp[15] = M[15]*scalar;

		return temp;
	}

	//! Multiply by scalar.
	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::operator*=(const T& scalar)
	{
		M[0]*=scalar;
		M[1]*=scalar;
		M[2]*=scalar;
		M[3]*=scalar;
		M[4]*=scalar;
		M[5]*=scalar;
		M[6]*=scalar;
		M[7]*=scalar;
		M[8]*=scalar;
		M[9]*=scalar;
		M[10]*=scalar;
		M[11]*=scalar;
		M[12]*=scalar;
		M[13]*=scalar;
		M[14]*=scalar;
		M[15]*=scalar;

		return *this;
	}

	//! Multiply by another matrix.
	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::operator*=(const CMatrix4<T>& other)
	{
		// do checks on your own in order to avoid copy creation
		if ( !other.getDefinitelyIdentityMatrix() )
		{
			if ( this->getDefinitelyIdentityMatrix() )
			{
				return (*this = other);
			}
			else
			{
				CMatrix4<T> temp ( *this );
				return setbyproduct_nocheck( temp, other );
			}
		}
		return *this;
	}

	//! multiply by another matrix
	// set this matrix to the product of two other matrices
	// goal is to reduce stack use and copy
	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setbyproduct_nocheck(const CMatrix4<T>& other_a,const CMatrix4<T>& other_b )
	{
		const T *m1 = other_a.M;
		const T *m2 = other_b.M;

		rowMatrixProduct(M, m1, m2);
		/*M[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
		  M[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
		  M[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
		  M[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];

		  M[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
		  M[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
		  M[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
		  M[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];

		  M[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
		  M[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
		  M[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
		  M[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];

		  M[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
		  M[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
		  M[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
		  M[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];*/
		definitelyIdentityMatrix=false;
		return *this;
	}


	//! multiply by another matrix
	// set this matrix to the product of two other matrices
	// goal is to reduce stack use and copy
	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setbyproduct(const CMatrix4<T>& other_a, const CMatrix4<T>& other_b )
	{
		if (other_a.getDefinitelyIdentityMatrix())
			return (*this = other_b);
		else if (other_b.getDefinitelyIdentityMatrix())
			return (*this = other_a);
		else
			return setbyproduct_nocheck(other_a,other_b);
	}

	//! multiply by another matrix
	template <class T>
	inline CMatrix4<T> CMatrix4<T>::operator*(const CMatrix4<T>& m) const
	{
		// Testing purpose..
		if (this->getDefinitelyIdentityMatrix())
			return m;
		if (m.getDefinitelyIdentityMatrix())
			return *this;

		CMatrix4<T> m3 ( EM4CONST_NOTHING );

#if TI_USE_RH
		const T *m1 = M;
		const T *m2 = m.M;
#else
		const T *m2 = M;
		const T *m1 = m.M;
#endif

		m3[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2] + m1[12]*m2[3];
		m3[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2] + m1[13]*m2[3];
		m3[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2] + m1[14]*m2[3];
		m3[3] = m1[3]*m2[0] + m1[7]*m2[1] + m1[11]*m2[2] + m1[15]*m2[3];

		m3[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6] + m1[12]*m2[7];
		m3[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6] + m1[13]*m2[7];
		m3[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6] + m1[14]*m2[7];
		m3[7] = m1[3]*m2[4] + m1[7]*m2[5] + m1[11]*m2[6] + m1[15]*m2[7];

		m3[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10] + m1[12]*m2[11];
		m3[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10] + m1[13]*m2[11];
		m3[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10] + m1[14]*m2[11];
		m3[11] = m1[3]*m2[8] + m1[7]*m2[9] + m1[11]*m2[10] + m1[15]*m2[11];

		m3[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12]*m2[15];
		m3[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13]*m2[15];
		m3[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14]*m2[15];
		m3[15] = m1[3]*m2[12] + m1[7]*m2[13] + m1[11]*m2[14] + m1[15]*m2[15];
		return m3;
	}


	//! multiply by another matrix
	template <class T>
	inline CMatrix4<T> CMatrix4<T>::mult34(const CMatrix4<T>& m2) const
	{
		CMatrix4<T> out;
		this->mult34(m2, out);
		return out;
	}


	//! multiply by another matrix
	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::mult34(const CMatrix4<T>& m, CMatrix4<T>& out) const
	{
		// Testing purpose..
		if ( getDefinitelyIdentityMatrix() )
		{
			out = m;
			return out;
		}

		if ( m.getDefinitelyIdentityMatrix() )
		{
			out = *this;
			return out;
		}

#if TI_USE_RH
		const T *m1 = M;
		const T *m2 = m.M;
#else
		const T *m2 = M;
		const T *m1 = m.M;
#endif

		out.M[0] = m1[0]*m2[0] + m1[4]*m2[1] + m1[8]*m2[2];
		out.M[1] = m1[1]*m2[0] + m1[5]*m2[1] + m1[9]*m2[2];
		out.M[2] = m1[2]*m2[0] + m1[6]*m2[1] + m1[10]*m2[2];
		out.M[3] = 0.0f;

		out.M[4] = m1[0]*m2[4] + m1[4]*m2[5] + m1[8]*m2[6];
		out.M[5] = m1[1]*m2[4] + m1[5]*m2[5] + m1[9]*m2[6];
		out.M[6] = m1[2]*m2[4] + m1[6]*m2[5] + m1[10]*m2[6];
		out.M[7] = 0.0f;

		out.M[8] = m1[0]*m2[8] + m1[4]*m2[9] + m1[8]*m2[10];
		out.M[9] = m1[1]*m2[8] + m1[5]*m2[9] + m1[9]*m2[10];
		out.M[10] = m1[2]*m2[8] + m1[6]*m2[9] + m1[10]*m2[10];
		out.M[11] = 0.0f;

		out.M[12] = m1[0]*m2[12] + m1[4]*m2[13] + m1[8]*m2[14] + m1[12];
		out.M[13] = m1[1]*m2[12] + m1[5]*m2[13] + m1[9]*m2[14] + m1[13];
		out.M[14] = m1[2]*m2[12] + m1[6]*m2[13] + m1[10]*m2[14] + m1[14];
		out.M[15] = 1.0f;

		out.definitelyIdentityMatrix = false;//definitelyIdentityMatrix && m2.definitelyIdentityMatrix;
			
		return out;
	}

	template <class T>
	inline vector3d<T> CMatrix4<T>::getColumn(uint32 c) const
	{
		const T* v = &M[c * 4];
		return vector3d<T>(v[0], v[1], v[2]);
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setColumn(uint32 c, const vector3d<T>& v)
	{
		T* dst = &M[c * 4];
		dst[0] = v.getX();
		dst[1] = v.getY();
		dst[2] = v.getZ();
		return *this;
	}

	template <class T>
	inline vector3d<T> CMatrix4<T>::getTranslation() const
	{
		return vector3d<T>(M[12], M[13], M[14]);
	}


	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setTranslation( const vector3d<T>& translation )
	{
		M[12] = translation.getX();
		M[13] = translation.getY();
		M[14] = translation.getZ();
		definitelyIdentityMatrix=false;
		return *this;
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setInverseTranslation( const vector3d<T>& translation )
	{
		M[12] = -translation.getX();
		M[13] = -translation.getY();
		M[14] = -translation.getZ();
		definitelyIdentityMatrix=false;
		return *this;
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setScale( const vector3d<T>& scale )
	{
		M[0] = scale.getX();
		M[5] = scale.getY();
		M[10] = scale.getZ();
		definitelyIdentityMatrix=false;
		return *this;
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::preScale( const vector3d<T>& scale )
	{
		if (definitelyIdentityMatrix)
		{
			setScale(scale);
		}
		else
		{
			M[0] *= scale.getX();
			M[1] *= scale.getY();
			M[2] *= scale.getZ();

			M[4] *= scale.getX();
			M[5] *= scale.getY();
			M[6] *= scale.getZ();

			M[8] *= scale.getX();
			M[9] *= scale.getY();
			M[10] *= scale.getZ();

			M[12] *= scale.getX();
			M[13] *= scale.getY();
			M[14] *= scale.getZ();

			definitelyIdentityMatrix=false;
		}
		return *this;
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::postScale( const vector3d<T>& scale )
	{
		if (definitelyIdentityMatrix)
		{
			setScale(scale);
		}
		else
		{
			M[0] *= scale.getX();
			M[1] *= scale.getX();
			M[2] *= scale.getX();

			M[4] *= scale.getY();
			M[5] *= scale.getY();
			M[6] *= scale.getY();

			M[8] *= scale.getZ();
			M[9] *= scale.getZ();
			M[10] *= scale.getZ();

			definitelyIdentityMatrix=false;
		}
		return *this;
	}

	template <class T>
	inline vector3d<T> CMatrix4<T>::getScale() const
	{
		vector3d<T> vScale;
		vScale.setX(vector3d<T>(M[0],M[1],M[2]).getLength());
		vScale.setY(vector3d<T>(M[4],M[5],M[6]).getLength());
		vScale.setZ(vector3d<T>(M[8],M[9],M[10]).getLength());
		return vScale;
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setRotationDegrees( const vector3d<T>& rotation )
	{
		return setRotationRadians( rotation * DEGTORAD );
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setInverseRotationDegrees( const vector3d<T>& rotation )
	{
		return setInverseRotationRadians( rotation * DEGTORAD );
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setRotationRadians( const vector3d<T>& rotation )
	{
		const float64 cr = cos( rotation.getX() );
		const float64 sr = sin( rotation.getX() );
		const float64 cp = cos( rotation.getY() );
		const float64 sp = sin( rotation.getY() );
		const float64 cy = cos( rotation.getZ() );
		const float64 sy = sin( rotation.getZ() );

		M[0] = (T)( cp*cy );
		M[1] = (T)( cp*sy );
		M[2] = (T)( -sp );

		const float64 srsp = sr*sp;
		const float64 crsp = cr*sp;

		M[4] = (T)( srsp*cy-cr*sy );
		M[5] = (T)( srsp*sy+cr*cy );
		M[6] = (T)( sr*cp );

		M[8] = (T)( crsp*cy+sr*sy );
		M[9] = (T)( crsp*sy-sr*cy );
		M[10] = (T)( cr*cp );
		definitelyIdentityMatrix=false;
		return *this;
	}


	//! Returns the rotation, as set by setRotation(). This code was sent
	//! in by Chev.
	template <class T>
	inline vector3d<T> CMatrix4<T>::getRotationDegrees() const
	{
		const CMatrix4<T> &mat = *this;

		float64 Y = -asin(mat(0,2));
		const float64 C = cos(Y);
		Y *= RADTODEG64;

		float64 rotx, roty, X, Z;

		if (fabs(C)>ROUNDING_ERROR_64)
		{
			const T invC = (T)(1.0/C);
			rotx = mat(2,2) * invC;
			roty = mat(1,2) * invC;
			X = atan2( roty, rotx ) * RADTODEG64;
			rotx = mat(0,0) * invC;
			roty = mat(0,1) * invC;
			Z = atan2( roty, rotx ) * RADTODEG64;
		}
		else
		{
			X = 0.0;
			rotx = mat(1,1);
			roty = -mat(1,0);
			Z = atan2( roty, rotx ) * RADTODEG64;
		}

		// fix values that get below zero
		// before it would set (!) values to 360
		// that where above 360:
		if (X < 0.0) X += 360.0;
		if (Y < 0.0) Y += 360.0;
		if (Z < 0.0) Z += 360.0;

		return vector3d<T>((T)X,(T)Y,(T)Z);
	}


	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setInverseRotationRadians( const vector3d<T>& rotation )
	{
		float64 cr = cos( rotation.getX() );
		float64 sr = sin( rotation.getX() );
		float64 cp = cos( rotation.getY() );
		float64 sp = sin( rotation.getY() );
		float64 cy = cos( rotation.getZ() );
		float64 sy = sin( rotation.getZ() );

		M[0] = (T)( cp*cy );
		M[4] = (T)( cp*sy );
		M[8] = (T)( -sp );

		float64 srsp = sr*sp;
		float64 crsp = cr*sp;

		M[1] = (T)( srsp*cy-cr*sy );
		M[5] = (T)( srsp*sy+cr*cy );
		M[9] = (T)( sr*cp );

		M[2] = (T)( crsp*cy+sr*sy );
		M[6] = (T)( crsp*sy-sr*cy );
		M[10] = (T)( cr*cp );
		definitelyIdentityMatrix=false;
		return *this;
	}


	/*!
	 */
	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::makeIdentity()
	{
		memset(M, 0, 16*sizeof(T));
		M[0] = M[5] = M[10] = M[15] = (T)1;
		definitelyIdentityMatrix=true;
		return *this;
	}


	template <class T>
	inline void CMatrix4<T>::rotateVect( vector3d<T>& vect ) const
	{
		vector3d<T> tmp = vect;
		vect.setX(tmp.getX()*M[0] + tmp.getY()*M[4] + tmp.getZ()*M[8]);
		vect.setY(tmp.getX()*M[1] + tmp.getY()*M[5] + tmp.getZ()*M[9]);
		vect.setZ(tmp.getX()*M[2] + tmp.getY()*M[6] + tmp.getZ()*M[10]);
	}

	//! An alternate transform vector method, writing into a second vector
	template <class T>
	inline void CMatrix4<T>::rotateVect(vector3d<T>& out, const vector3d<T>& in) const
	{
		out.setX(in.getX()*M[0] + in.getY()*M[4] + in.getZ()*M[8]);
		out.setY(in.getX()*M[1] + in.getY()*M[5] + in.getZ()*M[9]);
		out.setZ(in.getX()*M[2] + in.getY()*M[6] + in.getZ()*M[10]);
	}

	//! An alternate transform vector method, writing into an array of 3 floats
	template <class T>
	inline void CMatrix4<T>::rotateVect(T *out, const vector3d<T>& in) const
	{
		out[0] = in.getX()*M[0] + in.getY()*M[4] + in.getZ()*M[8];
		out[1] = in.getX()*M[1] + in.getY()*M[5] + in.getZ()*M[9];
		out[2] = in.getX()*M[2] + in.getY()*M[6] + in.getZ()*M[10];
	}

	template <class T>
	inline void CMatrix4<T>::inverseRotateVect( vector3d<T>& vect ) const
	{
		vector3d<T> tmp = vect;
		vect.setX(tmp.getX()*M[0] + tmp.getY()*M[1] + tmp.getZ()*M[2]);
		vect.setY(tmp.getX()*M[4] + tmp.getY()*M[5] + tmp.getZ()*M[6]);
		vect.setZ(tmp.getX()*M[8] + tmp.getY()*M[9] + tmp.getZ()*M[10]);
	}

	template <class T>
	inline void CMatrix4<T>::transformVect( vector3d<T>& vect) const
	{
		T vector[3];

		vector[0] = vect.getX()*M[0] + vect.getY()*M[4] + vect.getZ()*M[8] + M[12];
		vector[1] = vect.getX()*M[1] + vect.getY()*M[5] + vect.getZ()*M[9] + M[13];
		vector[2] = vect.getX()*M[2] + vect.getY()*M[6] + vect.getZ()*M[10] + M[14];

		vect.setX(vector[0]);
		vect.setY(vector[1]);
		vect.setZ(vector[2]);
	}

	template <class T>
	inline void CMatrix4<T>::transformVect2D( vector3d<T>& vect) const
	{
		T vector[2];

		vector[0] = vect.getX()*M[0] + vect.getY()*M[4] + M[12];
		vector[1] = vect.getX()*M[1] + vect.getY()*M[5] + M[13];

		vect.setX(vector[0]);
		vect.setY(vector[1]);
	}

	template <class T>
	inline void CMatrix4<T>::transformVect2D( vector3d<T>& out, const vector3d<T>& in) const
	{
		out.setX(in.getX()*M[0] + in.getY()*M[4] + M[12]);
		out.setY(in.getX()*M[1] + in.getY()*M[5] + M[13]);
	}

	template <class T>
	inline void CMatrix4<T>::transformVect( vector3d<T>& out, const vector3d<T>& in) const
	{
		out.setX(in.getX()*M[0] + in.getY()*M[4] + in.getZ()*M[8] + M[12]);
		out.setY(in.getX()*M[1] + in.getY()*M[5] + in.getZ()*M[9] + M[13]);
		out.setZ(in.getX()*M[2] + in.getY()*M[6] + in.getZ()*M[10] + M[14]);
	}

	template <class T>
	inline void CMatrix4<T>::transformVect( vector3d<T>& out, const T* in) const
	{
		out.setX(in[0]*M[0] + in[1]*M[4] + in[2]*M[8] + M[12]);
		out.setY(in[0]*M[1] + in[1]*M[5] + in[2]*M[9] + M[13]);
		out.setZ(in[0]*M[2] + in[1]*M[6] + in[2]*M[10] + M[14]);
	}

	template <class T>
	inline void CMatrix4<T>::transformVect(T *out, const vector3d<T> &in) const
	{
		out[0] = in.getX()*M[0] + in.getY()*M[4] + in.getZ()*M[8] + M[12];
		out[1] = in.getX()*M[1] + in.getY()*M[5] + in.getZ()*M[9] + M[13];
		out[2] = in.getX()*M[2] + in.getY()*M[6] + in.getZ()*M[10] + M[14];
		out[3] = in.getX()*M[3] + in.getY()*M[7] + in.getZ()*M[11] + M[15];
	}

	//! Transforms a axis aligned bounding box
	template <class T>
	inline void CMatrix4<T>::transformBox(aabbox3d<T>& box) const
	{
		if (getDefinitelyIdentityMatrix())
			return;

		transformVect(box.MinEdge);
		transformVect(box.MaxEdge);
		box.repair();
	}

	//! Transforms a axis aligned bounding box more accurately than transformBox()
	template <class T>
	inline void CMatrix4<T>::transformBoxEx(aabbox3d<T>& box) const
	{
		const T Amin[3] = {box.MinEdge.getX(), box.MinEdge.getY(), box.MinEdge.getZ()};
		const T Amax[3] = {box.MaxEdge.getX(), box.MaxEdge.getY(), box.MaxEdge.getZ()};

		T Bmin[3];
		T Bmax[3];

		Bmin[0] = Bmax[0] = M[12];
		Bmin[1] = Bmax[1] = M[13];
		Bmin[2] = Bmax[2] = M[14];

		const CMatrix4<T> &m = *this;

		for (uint32 i = 0; i < 3; ++i)
		{
			for (uint32 j = 0; j < 3; ++j)
			{
				const T a = m(j,i) * Amin[j];
				const T b = m(j,i) * Amax[j];

				if (a < b)
				{
					Bmin[i] += a;
					Bmax[i] += b;
				}
				else
				{
					Bmin[i] += b;
					Bmax[i] += a;
				}
			}
		}

		box.MinEdge.setX(Bmin[0]);
		box.MinEdge.setY(Bmin[1]);
		box.MinEdge.setZ(Bmin[2]);

		box.MaxEdge.setX(Bmax[0]);
		box.MaxEdge.setY(Bmax[1]);
		box.MaxEdge.setZ(Bmax[2]);
	}


	//! Multiplies this matrix by a 1x4 matrix
	template <class T>
	inline void CMatrix4<T>::multiplyWith1x4Matrix(T* matrix) const
	{
		/*
		  0  1  2  3
		  4  5  6  7
		  8  9  10 11
		  12 13 14 15
		*/

		T mat[4];
		mat[0] = matrix[0];
		mat[1] = matrix[1];
		mat[2] = matrix[2];
		mat[3] = matrix[3];

		matrix[0] = M[0]*mat[0] + M[4]*mat[1] + M[8]*mat[2] + M[12]*mat[3];
		matrix[1] = M[1]*mat[0] + M[5]*mat[1] + M[9]*mat[2] + M[13]*mat[3];
		matrix[2] = M[2]*mat[0] + M[6]*mat[1] + M[10]*mat[2] + M[14]*mat[3];
		matrix[3] = M[3]*mat[0] + M[7]*mat[1] + M[11]*mat[2] + M[15]*mat[3];
	}

	template <class T>
	inline void CMatrix4<T>::inverseTranslateVect( vector3d<T>& vect ) const
	{
		vect.setX(vect.getX()-M[12]);
		vect.setY(vect.getY()-M[13]);
		vect.setZ(vect.getZ()-M[14]);
	}

	template <class T>
	inline void CMatrix4<T>::translateVect( vector3d<T>& vect ) const
	{
		vect.setX(vect.getX()+M[12]);
		vect.setY(vect.getY()+M[13]);
		vect.setZ(vect.getZ()+M[14]);
	}

	//! Transforms a plane by this matrix
	template <class T>
	inline void CMatrix4<T>::transformPlane( plane3d<T> &plane) const
	{
		vector3d<T> member;
		transformVect(member, plane.getMemberPoint());

		vector3d<T> origin(0,0,0);
		transformVect(plane.Normal);
		transformVect(origin);

		plane.Normal -= origin;
		plane.D = - member.dotProduct(plane.Normal);
	}

	//! Transforms a plane by this matrix
	template <class T>
	inline void CMatrix4<T>::transformPlane_new( plane3d<T> &plane) const
	{
		// rotate normal -> rotateVect ( plane.n );
		vector3d<T> n;
		n.setX(plane.Normal.getX()*M[0] + plane.Normal.getY()*M[4] + plane.Normal.getZ()*M[8]);
		n.setY(plane.Normal.getX()*M[1] + plane.Normal.getY()*M[5] + plane.Normal.getZ()*M[9]);
		n.setZ(plane.Normal.getX()*M[2] + plane.Normal.getY()*M[6] + plane.Normal.getZ()*M[10]);

		// compute newly d. -> getTranslation(). dotproduct ( plane.n )
		plane.D -= M[12] * n.getX() + M[13] * n.getY() + M[14] * n.getZ();
		plane.Normal.setX(n.getX());
		plane.Normal.setY(n.getY());
		plane.Normal.setZ(n.getZ());
	}

	//! Transforms a plane by this matrix
	template <class T>
	inline void CMatrix4<T>::transformPlane( const plane3d<T> &in, plane3d<T> &out) const
	{
		out = in;
		transformPlane( out );
	}

	template <class T>
	inline T CMatrix4<T>::getDeterminant() const
	{
		if (this->getDefinitelyIdentityMatrix())
		{
			return T(1);
		}

		T t0 = M[10] * M[15] - M[11] * M[14];
		T t1 = M[6] * M[15] - M[7] * M[14];
		T t2 = M[6] * M[11] - M[7] * M[10];
		T t3 = M[2] * M[15] - M[3] * M[14];
		T t4 = M[2] * M[11] - M[3] * M[10];
		T t5 = M[2] * M[7] - M[3] * M[6];

		T t6 = M[8] * M[13] - M[9] * M[12];
		T t7 = M[4] * M[13] - M[5] * M[12];
		T t8 = M[4] * M[9] - M[5] * M[8];
		T t9 = M[0] * M[13] - M[1] * M[12];
		T t10 = M[0] * M[9] - M[1] * M[8];
		T t11 = M[0] * M[5] - M[1] * M[4];

		return t0 * t11 - t1 * t10 + t2 * t9 + t3 * t8 - t4 * t7 + t5 * t6;
	}

	template <class T>
	inline bool CMatrix4<T>::getInverse(CMatrix4<T>& out) const
	{
		if (this->getDefinitelyIdentityMatrix())
		{
			out = *this;
			return true;
		}

		// Cramer's rule.
		T t0 = M[10] * M[15] - M[11] * M[14];
		T t1 = M[6] * M[15] - M[7] * M[14];
		T t2 = M[6] * M[11] - M[7] * M[10];
		T t3 = M[2] * M[15] - M[3] * M[14];
		T t4 = M[2] * M[11] - M[3] * M[10];
		T t5 = M[2] * M[7] - M[3] * M[6];

		T t6 = M[8] * M[13] - M[9] * M[12];
		T t7 = M[4] * M[13] - M[5] * M[12];
		T t8 = M[4] * M[9] - M[5] * M[8];
		T t9 = M[0] * M[13] - M[1] * M[12];
		T t10 = M[0] * M[9] - M[1] * M[8];
		T t11 = M[0] * M[5] - M[1] * M[4];

		T det = t0 * t11 - t1 * t10 + t2 * t9 + t3 * t8 - t4 * t7 + t5 * t6;
		if (iszero( det ))
		{
			return false;
		}

		out.M[0] = M[5] * t0 - M[9] * t1 + M[13] * t2;
		out.M[1] = M[9] * t3 - M[1] * t0 - M[13] * t4;
		out.M[2] = M[1] * t1 - M[5] * t3 + M[13] * t5;
		out.M[3] = M[5] * t4 - M[1] * t2 - M[9] * t5;

		out.M[4] = M[8] * t1 - M[4] * t0 - M[12] * t2;
		out.M[5] = M[0] * t0 - M[8] * t3 + M[12] * t4;
		out.M[6] = M[4] * t3 - M[0] * t1 - M[12] * t5;
		out.M[7] = M[0] * t2 - M[4] * t4 + M[8] * t5;

		out.M[8] = M[7] * t6 - M[11] * t7 + M[15] * t8;
		out.M[9] = M[11] * t9 - M[3] * t6 - M[15] * t10;
		out.M[10] = M[3] * t7 - M[7] * t9 + M[15] * t11;
		out.M[11] = M[7] * t10 - M[3] * t8 - M[11] * t11;

		out.M[12] = M[10] * t7 - M[6] * t6 - M[14] * t8;
		out.M[13] = M[2] * t6 - M[10] * t9 + M[14] * t10;
		out.M[14] = M[6] * t9 - M[2] * t7 - M[14] * t11;
		out.M[15] = M[2] * t8 - M[6] * t10 + M[10] * t11;

		//det = reciprocal(det);
		det = 1.0f / det;
		for ( int i = 0; i < 16; ++i )
		{
			out.M[i] *= det;
		}
		out.definitelyIdentityMatrix = definitelyIdentityMatrix;
		return true;
	}

	//! Inverts a primitive matrix which only contains a translation and a rotation
	//! \param out: where result matrix is written to.
	template <class T>
	inline bool CMatrix4<T>::getInversePrimitive(CMatrix4<T>& out) const
	{
		out.M[0 ] = M[0];
		out.M[1 ] = M[4];
		out.M[2 ] = M[8];
		out.M[3 ] = 0;

		out.M[4 ] = M[1];
		out.M[5 ] = M[5];
		out.M[6 ] = M[9];
		out.M[7 ] = 0;

		out.M[8 ] = M[2];
		out.M[9 ] = M[6];
		out.M[10] = M[10];
		out.M[11] = 0;

		out.M[12] = -(M[12] * M[0] + M[13] * M[1] + M[14] * M[2]);
		out.M[13] = -(M[12] * M[4] + M[13] * M[5] + M[14] * M[6]);
		out.M[14] = -(M[12] * M[8] + M[13] * M[9] + M[14] * M[10]);
		out.M[15] = T(1);
		out.definitelyIdentityMatrix = definitelyIdentityMatrix;
		return true;
	}

	/*!
	 */
	template <class T>
	inline bool CMatrix4<T>::makeInverse()
	{
		if (definitelyIdentityMatrix)
			return true;

		CMatrix4<T> temp ( EM4CONST_NOTHING );

		if (getInverse(temp))
		{
			*this = temp;
			return true;
		}

		return false;
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::operator=(const T& scalar)
	{
		for (int i = 0; i < 16; ++i)
			M[i]=scalar;
		definitelyIdentityMatrix=false;
		return *this;
	}


	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::operator=(const CMatrix4<T>& other)
	{
		for (int i = 0; i < 16; ++i)
			M[i]=other.M[i];
		definitelyIdentityMatrix=other.definitelyIdentityMatrix;
		return *this;
	}


	template <class T>
	inline bool CMatrix4<T>::operator==(const CMatrix4<T> &other) const
	{
		if (definitelyIdentityMatrix && other.definitelyIdentityMatrix)
			return true;
		for (int i = 0; i < 16; ++i)
			if (M[i] != other.M[i])
				return false;

		return true;
	}


	template <class T>
	inline bool CMatrix4<T>::operator!=(const CMatrix4<T> &other) const
	{
		return !(*this == other);
	}

	// creates a newly matrix as interpolated matrix from this and the passed one.
	template <class T>
	inline CMatrix4<T> CMatrix4<T>::interpolate(const CMatrix4<T>& b, T time) const
	{
		CMatrix4<T> mat ( EM4CONST_NOTHING );

		for (uint32 i=0; i < 16; i += 4)
		{
			mat.M[i+0] = (T)(M[i+0] + ( b.M[i+0] - M[i+0] ) * time);
			mat.M[i+1] = (T)(M[i+1] + ( b.M[i+1] - M[i+1] ) * time);
			mat.M[i+2] = (T)(M[i+2] + ( b.M[i+2] - M[i+2] ) * time);
			mat.M[i+3] = (T)(M[i+3] + ( b.M[i+3] - M[i+3] ) * time);
		}
		return mat;
	}


	// returns transposed matrix
	template <class T>
	inline CMatrix4<T> CMatrix4<T>::getTransposed() const
	{
		CMatrix4<T> t ( EM4CONST_NOTHING );
		getTransposed ( t );
		return t;
	}


	// returns transposed matrix
	template <class T>
	inline void CMatrix4<T>::getTransposed( CMatrix4<T>& o ) const
	{
		o.M[ 0] = M[ 0];
		o.M[ 1] = M[ 4];
		o.M[ 2] = M[ 8];
		o.M[ 3] = M[12];

		o.M[ 4] = M[ 1];
		o.M[ 5] = M[ 5];
		o.M[ 6] = M[ 9];
		o.M[ 7] = M[13];

		o.M[ 8] = M[ 2];
		o.M[ 9] = M[ 6];
		o.M[10] = M[10];
		o.M[11] = M[14];

		o.M[12] = M[ 3];
		o.M[13] = M[ 7];
		o.M[14] = M[11];
		o.M[15] = M[15];
		o.definitelyIdentityMatrix=definitelyIdentityMatrix;
	}

	// rotate about z axis, center ( 0.5, 0.5 )
	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setTextureRotationCenter( T rotateRad )
	{
		const T c = cosf(rotateRad);
		const T s = sinf(rotateRad);
		M[0] = (T)c;
		M[1] = (T)s;

		M[4] = (T)-s;
		M[5] = (T)c;

		M[8] = (T)(0.5f * ( s - c) + 0.5f);
		M[9] = (T)(-0.5f * ( s + c) + 0.5f);
		definitelyIdentityMatrix = definitelyIdentityMatrix && (rotateRad==0.0f);
		return *this;
	}


	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setTextureTranslate ( T x, T y )
	{
		M[8] = (T)x;
		M[9] = (T)y;
		definitelyIdentityMatrix = definitelyIdentityMatrix && (x==0.0f) && (y==0.0f);
		return *this;
	}


	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setTextureTranslateTransposed ( T x, T y )
	{
		M[2] = (T)x;
		M[6] = (T)y;
		definitelyIdentityMatrix = definitelyIdentityMatrix && (x==0.0f) && (y==0.0f) ;
		return *this;
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setTextureScale ( T sx, T sy )
	{
		M[0] = (T)sx;
		M[5] = (T)sy;
		definitelyIdentityMatrix = definitelyIdentityMatrix && (sx==1.0f) && (sy==1.0f);
		return *this;
	}

	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::postTextureScale ( T sx, T sy )
	{
		M[0] *= (T)sx;
		M[1] *= (T)sx;
		M[4] *= (T)sy;
		M[5] *= (T)sy;
		definitelyIdentityMatrix = definitelyIdentityMatrix && (sx==1.0f) && (sy==1.0f);
		return *this;
	}


	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setTextureScaleCenter( T sx, T sy )
	{
		M[0] = (T)sx;
		M[5] = (T)sy;
		M[8] = (T)(0.5f - 0.5f * sx);
		M[9] = (T)(0.5f - 0.5f * sy);
		definitelyIdentityMatrix = definitelyIdentityMatrix && (sx==1.0f) && (sy==1.0f);
		return *this;
	}


	// sets all matrix data members at once
	template <class T>
	inline CMatrix4<T>& CMatrix4<T>::setM(const T* data)
	{
		memcpy(M,data, 16*sizeof(T));

		definitelyIdentityMatrix = false;
		return *this;
	}

	// gets all matrix data members at once
	template <class T>
	inline T* CMatrix4<T>::getM(T* data) const
	{
		memcpy(data, M, 16 * sizeof(T));
		return data;
	}


	// sets if the matrix is definitely identity matrix
	template <class T>
	inline void CMatrix4<T>::setDefinitelyIdentityMatrix( bool isDefinitelyIdentityMatrix)
	{
		definitelyIdentityMatrix = isDefinitelyIdentityMatrix;
	}


	// gets if the matrix is definitely identity matrix
	template <class T>
	inline bool CMatrix4<T>::getDefinitelyIdentityMatrix() const
	{
		return definitelyIdentityMatrix;
	}


	// Multiply by scalar.
	template <class T>
	inline CMatrix4<T> operator*(const T scalar, const CMatrix4<T>& mat)
	{
		return mat*scalar;
	}


	//! Typedef for float32 matrix
	typedef CMatrix4<float32> matrix4;
	//! global const identity matrix
	extern const matrix4 IdentityMatrix;

	//MT must have operator[]
	template<class T, typename MT>
	inline CMatrix4<T> operator*(const CMatrix4<T>& mat, const MT& other)
	{
		CMatrix4<T> mat2(CMatrix4<T>::EM4CONST_NOTHING);
		
		if ( mat.getDefinitelyIdentityMatrix() )
		{
			for(uint32 i = 0; i < 16; ++i)
			{
				mat2[i] = other[i];
			}
		}
		else
		{
			rowMatrixProduct34(mat2, mat, other.M);
		}
		
		return mat2;
	}

	//MT must have operator[]
	template<typename T, typename MT>
	inline CMatrix4<T> operator*(const MT& other, const CMatrix4<T>& mat)
	{
		CMatrix4<T> mat2(CMatrix4<T>::EM4CONST_NOTHING);

		if ( mat.getDefinitelyIdentityMatrix() )
		{
			for(uint32 i = 0; i < 16; ++i)
			{
				mat2[i] = other[i];
			}
		}
		else
		{
			CMatrix4<T>::rowMatrixProduct34(mat2, other.M, mat);
		}
		
		return mat2;
	}


	//! Builds a right-handed perspective projection matrix based on a field of view
	template<typename T>
	CMatrix4<T> buildProjectionMatrixPerspectiveFov(T fieldOfViewRadians, T aspectRatio, T zNear, T zFar)
	{
		const float64 h = 1.0/tan(fieldOfViewRadians/2.0);
		const T w = (T)(h / aspectRatio);

		CMatrix4<T> m(CMatrix4<T>::EM4CONST_NOTHING);

		m(0,0) = w;
		m(0,1) = 0;
		m(0,2) = 0;
		m(0,3) = 0;

		m(1,0) = 0;
		m(1,1) = (T)h;
		m(1,2) = 0;
		m(1,3) = 0; 

		m(2,0) = 0;
		m(2,1) = 0;
		m(2,2) = (T)(zFar/(zFar-zNear));
		m(2,3) = 1;

		m(3,0) = 0;
		m(3,1) = 0;
		m(3,2) = (T)(-zNear*zFar/(zFar-zNear));
		m(3,3) = 0;

		return m;
	}

	//! Builds a left-handed perspective projection matrix.
	template<typename T>
	CMatrix4<T> buildProjectionMatrixPerspectiveFovInfinity(T fieldOfViewRadians, T aspectRatio, T zNear)
	{
		const float64 h = 1.0/tan(fieldOfViewRadians/2.0);
		const T w = (T)(h / aspectRatio);

		CMatrix4<T> m(CMatrix4<T>::EM4CONST_NOTHING);

		m(0,0) = w;
		m(0,1) = 0;
		m(0,2) = 0;
		m(0,3) = 0;

		m(1,0) = 0;
		m(1,1) = (T)h;
		m(1,2) = 0;
		m(1,3) = 0; 

		m(2,0) = 0;
		m(2,1) = 0;
		m(2,2) = 1;
		m(2,3) = 1;

		m(3,0) = 0;
		m(3,1) = 0;
		m(3,2) = (T)-zNear;
		m(3,3) = 0;

		return m;
	}

	//! Builds a left-handed perspective projection matrix.
	template<typename T>
	CMatrix4<T> buildProjectionMatrixPerspective(T widthOfViewVolume, T heightOfViewVolume, T zNear, T zFar)
	{
		CMatrix4<T> m(CMatrix4<T>::EM4CONST_NOTHING);

		m(0,0) = (T)(2*zNear/widthOfViewVolume);
		m(0,1) = 0;
		m(0,2) = 0;
		m(0,3) = 0;

		m(1,0) = 0;
		m(1,1) = (T)(2*zNear/heightOfViewVolume);
		m(1,2) = 0;
		m(1,3) = 0; 

		m(2,0) = 0;
		m(2,1) = 0;
		m(2,2) = (T)(zFar/(zFar-zNear));
		m(2,3) = 1;

		m(3,0) = 0;
		m(3,1) = 0;
		m(3,2) = (T)(zNear*zFar/(zNear-zFar));
		m(3,3) = 0;

		return m;
	}

	//! Builds a left-handed perspective projection matrix.
	template<typename T>
	CMatrix4<T> buildProjectionMatrixPerspectiveInfinity(T widthOfViewVolume, T heightOfViewVolume, T zNear)
	{
		CMatrix4<T> m(CMatrix4<T>::EM4CONST_NOTHING);

		m(0,0) = (T)(2*zNear/widthOfViewVolume);
		m(0,1) = 0;
		m(0,2) = 0;
		m(0,3) = 0;

		m(1,0) = 0;
		m(1,1) = (T)(2*zNear/heightOfViewVolume);
		m(1,2) = 0;
		m(1,3) = 0; 

		m(2,0) = 0;
		m(2,1) = 0;
		m(2,2) = 1;
		m(2,3) = 1;

		m(3,0) = 0;
		m(3,1) = 0;
		m(3,2) = (T)-zNear;
		m(3,3) = 0;

		return m;
	}

	//! Builds a centered right-handed orthogonal projection matrix.
	template<typename T>
	CMatrix4<T> buildProjectionMatrixOrtho(T widthOfViewVolume, T heightOfViewVolume, T zNear, T zFar)
	{

		CMatrix4<T> m(CMatrix4<T>::EM4CONST_NOTHING);

		m(0,0) = (T)(2/widthOfViewVolume);
		m(0,1) = 0;
		m(0,2) = 0;
		m(0,3) = 0;

		m(1,0) = 0;
		m(1,1) = (T)(2/heightOfViewVolume);
		m(1,2) = 0;
		m(1,3) = 0; 

		m(2,0) = 0;
		m(2,1) = 0;
		m(2,2) = (T)(1/(zFar-zNear));
		m(2,3) = 0;

		m(3,0) = 0;
		m(3,1) = 0;
		m(3,2) = (T)(zNear/(zNear-zFar));
		m(3,3) = 1;

		return m;
	}

	//! Builds a right-handed orthogonal projection matrix.
	template<typename T>
	CMatrix4<T> buildProjectionMatrixOrtho(T left, T right, T bottom, T top, T zNear, T zFar)
	{
		T w = right - left;
		T h = top - bottom;

		CMatrix4<T> m(CMatrix4<T>::EM4CONST_NOTHING);

		m(0,0) = (T)(2/w);
		m(0,1) = 0;
		m(0,2) = 0;
		m(0,3) = 0;

		m(1,0) = 0;
		m(1,1) = (T)(2/h);
		m(1,2) = 0;
		m(1,3) = 0; 

		m(2,0) = 0;
		m(2,1) = 0;
		m(2,2) = (T)(1/(zFar-zNear));
		m(2,3) = 0;

		m(3,0) = - (left + right) / w;
		m(3,1) = - (bottom + top) / h;
		m(3,2) = (T)(zNear/(zNear-zFar));
		m(3,3) = 1;

		return m;
	}

	//! Builds a right-handed look-at matrix.
	template <typename T>
	CMatrix4<T> buildCameraLookAtMatrix(
		const vector3d<T>& position,
		const vector3d<T>& target,
		const vector3d<T>& upVector)
	{
		vector3d<T> zaxis = target - position;
		zaxis.normalize();

		vector3d<T> xaxis = upVector.crossProduct(zaxis);
		xaxis.normalize();

		vector3d<T> yaxis = zaxis.crossProduct(xaxis);

		CMatrix4<T> m(CMatrix4<T>::EM4CONST_NOTHING);

		m(0,0) = (T)xaxis.getX();
		m(0,1) = (T)yaxis.getX();
		m(0,2) = (T)zaxis.getX();
		m(0,3) = 0;

		m(1,0) = (T)xaxis.getY();
		m(1,1) = (T)yaxis.getY();
		m(1,2) = (T)zaxis.getY();
		m(1,3) = 0;

		m(2,0) = (T)xaxis.getZ();
		m(2,1) = (T)yaxis.getZ();
		m(2,2) = (T)zaxis.getZ();
		m(2,3) = 0;

		m(3,0) = (T)-xaxis.dotProduct(position);
		m(3,1) = (T)-yaxis.dotProduct(position);
		m(3,2) = (T)-zaxis.dotProduct(position);
		m(3,3) = 1;

		return m;
	}

	////! Builds a matrix that flattens geometry into a plane.
	///** \param light light source
	//	\param plane: plane into which the geometry if flattened into
	//	\param point: value between 0 and 1, describing the light source.
	//	If this is 1, it is a point light, if it is 0, it is a directional light. */
	template<typename T>
	CMatrix4<T> buildShadowMatrix(const vector3d<T>& light, plane3d<T> plane, T point=1.0f)
	{
		CMatrix4<T> m(CMatrix4<T>::EM4CONST_NOTHING);

		plane.Normal.normalize();
		const T d = plane.Normal.dotProduct(light);

		m(0,0) = (T)(-plane.Normal.getX() * light.getX() + d);
		m(0,1) = (T)(-plane.Normal.getX() * light.getY());
		m(0,2) = (T)(-plane.Normal.getX() * light.getZ());
		m(0,3) = (T)(-plane.Normal.getX() * point);

		m(1,0) = (T)(-plane.Normal.getY() * light.getX());
		m(1,1) = (T)(-plane.Normal.getY() * light.getY() + d);
		m(1,2) = (T)(-plane.Normal.getY() * light.getZ());
		m(1,3) = (T)(-plane.Normal.getY() * point); 

		m(2,0) = (T)(-plane.Normal.getZ() * light.getX());
		m(2,1) = (T)(-plane.Normal.getZ() * light.getY());
		m(2,2) = (T)(-plane.Normal.getZ() * light.getZ() + d);
		m(2,3) = (T)(-plane.Normal.getZ() * point);

		m(3,0) = (T)(-plane.D * light.getX());
		m(3,1) = (T)(-plane.D * light.getY());
		m(3,2) = (T)(-plane.D * light.getZ());
		m(3,3) = (T)(-plane.D * point + d);

		return m;
	}

} // end namespace ti

#endif

