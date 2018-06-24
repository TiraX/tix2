/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TI_API TStream : public IReferenceCounted
	{
	public:
		TStream(int buf_size = 1024)
			: BufferSize(buf_size)
			, Pos(0)
		{
			Buffer = ti_new char[BufferSize];
		}

		TStream(void* buf, int buf_size)
			: Pos(buf_size)
		{
			BufferSize = (buf_size + 3) & (~3);
			Buffer = ti_new char[BufferSize];

			memcpy(Buffer, buf, buf_size);
		}

		virtual ~TStream()
		{
			ti_delete[] Buffer;
		}

		void Put(const void* buf, int size)
		{
			if (size == 0)
				return;

			if (Pos + size > BufferSize)
			{
				ReallocBuffer(Pos + size);
			}

			memcpy(Buffer + Pos, buf, size);
			Pos += size;
		}

		void Reset()
		{
			Pos = 0;
		}

		char* GetBuffer()
		{
			return Buffer;
		}

		int32 GetLength()
		{
			return Pos;
		}

	protected:
		void ReallocBuffer(int size)
		{
			if (size < BufferSize * 2)
			{
				size = BufferSize * 2;
			}
			else
			{
				size = (size + 4) & (~3);
			}
			char* newBuffer = ti_new char[size];
			memcpy(newBuffer, Buffer, Pos);

			char* oldBuffer = Buffer;
			Buffer = newBuffer;
			ti_delete[] oldBuffer;

			BufferSize = size;
		}

	protected:
		char*	Buffer;
		int		Pos;
		int		BufferSize;
	};

	typedef TI_INTRUSIVE_PTR(TStream)	TStreamPtr;
}