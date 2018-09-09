/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TStream : public IReferenceCounted
	{
	public:
		TStream(int32 buf_size = 1024)
			: BufferSize(buf_size)
			, Pos(0)
		{
			if (buf_size > 0)
			{
				Buffer = ti_new char[BufferSize];
			}
			else
			{
				Buffer = nullptr;
			}
		}

		TStream(void* buf, int32 buf_size)
			: Pos(buf_size)
		{
			TI_ASSERT(buf_size != 0);
			BufferSize = (buf_size + 3) & (~3);
			Buffer = ti_new char[BufferSize];

			memcpy(Buffer, buf, buf_size);
		}

		virtual ~TStream()
		{
			Destroy();
		}

		void Put(const void* buf, int32 size)
		{
			if (size == 0)
				return;

			if (Pos + size > BufferSize)
			{
				IncreaseBuffer(Pos + size);
			}

			memcpy(Buffer + Pos, buf, size);
			Pos += size;
		}

		void Put(TFile& File)
		{
			File.Seek(0);
			ReallocBuffer(File.GetSize());
			Pos += File.Read(Buffer, BufferSize, File.GetSize());
		}

		void Reset()
		{
			Pos = 0;
		}

		void Destroy()
		{
			if (Buffer != nullptr)
			{
				ti_delete[] Buffer;
				Buffer = nullptr;
			}
			Pos = 0;
			BufferSize = 0;
		}

		char* GetBuffer()
		{
			return Buffer;
		}

		const char* GetBuffer() const
		{
			return Buffer;
		}

		int32 GetLength() const
		{
			return Pos;
		}

	protected:
		void IncreaseBuffer(int32 size)
		{
			if (size < BufferSize * 2)
			{
				size = BufferSize * 2;
			}

			ReallocBuffer(size);
		}

		void ReallocBuffer(int32 size)
		{
			size = ti_align4(size);
			if (size < BufferSize)
				return;

			char* newBuffer = ti_new char[size];
			memcpy(newBuffer, Buffer, Pos);

			if (Buffer != nullptr)
			{
				ti_delete[] Buffer;
			}
			Buffer = newBuffer;

			BufferSize = size;
		}

	protected:
		char* Buffer;
		int32 Pos;
		int32 BufferSize;
	};
}