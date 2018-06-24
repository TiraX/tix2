//-*-c++-*-
// Copyright (C) 2002-2008 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h
#pragma once
#ifndef __IRR_POINT_4D_H_INCLUDED__
#define __IRR_POINT_4D_H_INCLUDED__

namespace tix
{

	//! 3d vector template class with lots of operators and methods.
	template < typename T >
	class vector4d
	{
	public:

		//!
		typedef T SValueType;

		//! Default constructor (null vector).
		vector4d() : X(0), Y(0), Z(0), W(0) {}
		//! Constructor with three different values
		vector4d(T nx, T ny, T nz, T nw) : X(nx), Y(ny), Z(nz), W(nw) {}

		//! Copy constructor
		vector4d(const vector4d<T>& other) : X(other.X), Y(other.Y), Z(other.Z), W(other.W) {}
		// operators

		T X, Y, Z, W;
	};

	//! Typedef for a f32 4d vector.
	typedef vector4d<float32> vector4df;
	//! Typedef for an integer 4d vector.
	typedef vector4d<int> vector4di;

} // end namespace ti

#endif

