/*
	TiX Engine v2.0 Copyright (C) 2018~2021
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	class TStream : public IReferenceCounted
	{
	public:
		TStream(uint32 buf_size = 0)
			: Buffer(nullptr)
			, BufferSize(buf_size)
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

		TStream(void* buf, uint32 buf_size)
			: Buffer(nullptr)
			, BufferSize(buf_size)
			, Pos(buf_size)
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

		TStream(const TStream& Other)
			: Buffer(nullptr)
			, BufferSize(0)
			, Pos(0)
		{
			*this = Other;
		}

		TStream& operator = (const TStream& Other)
		{
			if (Buffer != nullptr)
			{
				ti_delete[] Buffer;
				Buffer = nullptr;
			}
			Pos = Other.Pos;
			BufferSize = Other.BufferSize;
			if (BufferSize > 0)
			{
				Buffer = ti_new char[BufferSize];
				memcpy(Buffer, Other.Buffer, Pos);
			}

			return *this;
		}

		// Put data in this stream, increase 'Pos'
		void Put(const void* buf, uint32 size)
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

		// Set data in this stream, keep 'Pos' not changed.
		void Set(const void* buf, uint32 size)
		{
			if (size == 0)
				return;
			TI_ASSERT(Pos + size <= BufferSize);
			memcpy(Buffer + Pos, buf, size);
		}

		// Fill data with 0, keep 'Pos' not changed
		void FillWithZero(uint32 size)
		{
			if (size == 0)
				return;
			TI_ASSERT(Pos + size <= BufferSize);
			memset(Buffer + Pos, 0, size);
		}
		
		// Seek 'Pos' to position in buffer
		void Seek(uint32 NewPos)
		{
			TI_ASSERT(NewPos < BufferSize);
			Pos = NewPos;
		}

		void Reset()
		{
			Pos = 0;
		}

		void ReserveAndClear(uint32 InSize)
		{
			TI_ASSERT(InSize > 0);
			if (InSize != BufferSize)
			{
				if (Buffer != nullptr)
				{
					ti_delete[] Buffer;
					Buffer = nullptr;
				}
				Pos = 0;
				BufferSize = InSize;
				if (BufferSize > 0)
				{
					Buffer = ti_new char[InSize];
				}
			}
			if (InSize > 0)
				memset(Buffer, 0, InSize);
		}

		void ReserveAndFill(uint32 InSize)
		{
			TI_ASSERT(InSize > 0);
			if (InSize != BufferSize)
			{
				if (Buffer != nullptr)
				{
					ti_delete[] Buffer;
					Buffer = nullptr;
				}
				Pos = InSize;
				BufferSize = InSize;
				if (BufferSize > 0)
				{
					Buffer = ti_new char[InSize];
				}
			}
			if (InSize > 0)
				memset(Buffer, 0, InSize);
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

		uint32 GetLength() const
		{
			return Pos;
		}

		uint32 GetBufferSize() const
		{
			return BufferSize;
		}

	protected:
		void IncreaseBuffer(uint32 size)
		{
			if (size < BufferSize * 2)
			{
				size = BufferSize * 2;
			}

			ReallocBuffer(size);
		}

		void ReallocBuffer(uint32 size)
		{
			size = TMath::Align4(size);
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
		int8* Buffer;
		uint32 Pos;
		uint32 BufferSize;
	};
}