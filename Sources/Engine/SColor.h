/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class SColorf;
	class SColor
	{
	public:

		//! Constructor of the Color. Does nothing.
		/** The color value is not initialized to save time. */
		SColor()
			: R(0)
			, G(0)
			, B(0)
			, A(255)
		{
		}

		//! Constructs the color from 4 values representing the alpha, red, green and blue component.
		/** Must be values between 0 and 255. */
		SColor (uint8 a, uint8 r, uint8 g, uint8 b)
			: R(r)
			, G(g)
			, B(b)
			, A(a)
		{

		}

		//! Returns the luminance of the color.
		float32 getLuminance() const
		{
			return 0.3f*R + 0.59f*G + 0.11f*B;
		}

		//! Returns the average intensity of the color.
		uint8 getAverage() const
		{
			return uint8((uint32(R) + G + B) / 3);
		}

		void set(uint8 a, uint8 r, uint8 g, uint8 b)
		{
			A = a;
			R = r;
			G = g;
			B = b;
		}

		//! Compares the color to another color.
		/** \return True if the colors are the same, and false if not. */
		bool operator==(const SColor& other) const 
		{ 
			return (*(uint32*)this) == (*(uint32*)&other); 
		}

		//! Compares the color to another color.
		/** \return True if the colors are different, and false if they are the same. */
		bool operator!=(const SColor& other) const 
		{ 
			return (*(uint32*)this) != (*(uint32*)&other); 
		}

		inline SColor operator * (float d)
		{
			SColor c;
			c.A	= (uint8)(A * d);
			c.R	= (uint8)(R * d);
			c.G	= (uint8)(G * d);
			c.B	= (uint8)(B * d);
			return c;
		}

		//!
		uint8& operator [] (uint32 i)
		{
			TI_ASSERT(i < 4);
			return GetDataPtr()[i];
		}

		//!
		const uint8& operator [] (uint32 i) const
		{
			TI_ASSERT(i < 4);
			return GetDataPtr()[i];
		}
		inline uint8* GetDataPtr()
		{
			return reinterpret_cast<uint8*>(this);
		}

		inline const uint8* GetDataPtr() const
		{
			return reinterpret_cast<const uint8*>(this);
		}

		uint8 R, G, B, A;
	};

	const float k_color_inv = 1.0f / 255.0f;
	class SColorf
	{
	public:
		SColorf()
			: A(1.0f)
			, R(0.0f)
			, G(0.0f)
			, B(0.0f)
		{}
		SColorf(const SColor& c)
			: A(c.A * k_color_inv)
			, R(c.R * k_color_inv)
			, G(c.G * k_color_inv)
			, B(c.B * k_color_inv)
		{}

		SColorf(float a, float r, float g, float b)
			: A(a), R(r), G(g), B(b)
		{}

		SColor	GetColoruint8() const
		{
			SColor c;

			c.A	= (uint8)(A * 255);
			c.R	= (uint8)(R * 255);
			c.G	= (uint8)(G * 255);
			c.B	= (uint8)(B * 255);

			return c;
		}
		//!
		float32& operator [] (uint32 i)
		{
			TI_ASSERT(i < 4);
			return GetDataPtr()[i];
		}

		//!
		const float32& operator [] (uint32 i) const
		{
			TI_ASSERT(i < 4);
			return GetDataPtr()[i];
		}

		inline float* GetDataPtr()
		{
			return reinterpret_cast<float*>(this);
		}
		inline const float* GetDataPtr() const
		{
			return reinterpret_cast<const float*>(this);
		}

		inline SColorf operator * (float d) const
		{
			SColorf c;
			c.A	= A * d;
			c.R	= R * d;
			c.G	= G * d;
			c.B	= B * d;
			return c;
		}

		inline SColorf operator - (const SColorf& other) const
		{
			SColorf c;
			c.A		= A - other.A;
			c.R		= R - other.R;
			c.G		= G	- other.G;
			c.B		= B - other.B;
			return c;
		}

		inline SColorf operator + (const SColorf& other) const
		{
			SColorf c;
			c.A		= A + other.A;
			c.R		= R + other.R;
			c.G		= G	+ other.G;
			c.B		= B + other.B;
			return c;
		}

		inline SColorf& operator*=(float d) 
		{
			A	*= d;
			R	*= d;
			G	*= d;
			B	*= d;
			return *this; 
		}
		inline SColorf& operator*=(const SColorf& other)
		{
			A	*= other.A;
			R	*= other.R;
			G	*= other.G;
			B	*= other.B;
			return *this;
		}
		inline SColorf& operator+=(const SColorf& other)
		{
			R	+= other.R;
			G	+= other.G;
			B	+= other.B;
			A	+= other.A;

			return *this;
		}
		const bool operator != (const SColorf& other) const
		{
			return  A != other.A ||
					R != other.R ||
					G != other.G ||
					B != other.B ;
		}

		float R, G, B, A;
	};
}