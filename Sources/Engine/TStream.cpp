/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"

#include <stdarg.h>
#include "TStream.h"

namespace tix
{
	TStream::TStream(int buf_size)
		: BufferSize(buf_size)
		, Pos(0)
	{
		Buffer = ti_new char[BufferSize];
	}

	TStream::TStream(void* buf, int buf_size)
		: Pos(buf_size)
	{
		BufferSize	= (buf_size + 3) & (~3);
		Buffer		= ti_new char[BufferSize];

		memcpy(Buffer, buf, buf_size);
	}

	TStream::~TStream()
	{
		ti_delete[] Buffer;
	}

	void TStream::Put(const void* buf, int size)
	{
		if (size == 0)
			return;

		if (Pos + size > BufferSize)
		{
			ReallocBuffer(Pos + size);
		}

		memcpy(Buffer + Pos, buf, size);
		Pos				+= size;
	}

	void TStream::ReallocBuffer(int size)
	{
		if (size < BufferSize * 2)
		{
			size = BufferSize * 2;
		}
		else
		{
			size = (size + 4) & (~3);
		}
		char* newBuffer	= ti_new char[size];
		memcpy(newBuffer, Buffer, Pos);

		char* oldBuffer = Buffer;
		Buffer			= newBuffer;
		ti_delete[] oldBuffer;

		BufferSize		= size;
	}

	void TStream::Reset()
	{
		Pos				= 0;
	}
}