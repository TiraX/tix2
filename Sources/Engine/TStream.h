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
		TStream(int buf_size = 1024);
		TStream(void* buf, int buf_size);
		virtual ~TStream();

		void Put(const void* buf, int size);
		void Reset();

				char* GetBuffer()
				{
					return Buffer;
				}

				int GetLength()
				{
					return Pos;
				}

	protected:
		void	ReallocBuffer(int size);

	protected:
		char*	Buffer;
		int		Pos;
		int		BufferSize;
	};

	typedef TI_INTRUSIVE_PTR(TStream)	TStreamPtr;
}