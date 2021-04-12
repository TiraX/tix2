/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

template < typename T >
class TArray3
{
public:
	TArray3() {};

	void Resize(const vector3di& InSize)
	{
		Size = InSize;
		int32 Total = Size.X * Size.Y * Size.Z;
		Data.resize(Total);
	}

	int32 GetAccessIndex(int32 x, int32 y, int32 z) const
	{
		return z * (Size.X * Size.Y) + y * Size.X + x;
	}

	void ResetZero()
	{
		memset(Data.data(), 0, Data.size() * sizeof(T));
	}

	const T& operator[] (int32 Index) const
	{
		return Data[Index];
	}

	T& operator[] (int32 Index)
	{
		return Data[Index];
	}

	const TVector<T>& GetData() const
	{
		return Data;
	}

	TVector<T>& GetData()
	{
		return Data;
	}

	const vector3di& GetSize() const
	{
		return Size;
	}

private:
	vector3di Size;

	TVector<T> Data;
};