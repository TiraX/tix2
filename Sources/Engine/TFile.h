/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	enum E_FILE_ACCESS
	{
		EFA_READ,
		EFA_WRITE,
		EFA_CREATEWRITE,
		EFA_READWRITE,
	};

	class TI_API TFile
	{
	public:
		TFile();
		virtual ~TFile();

		virtual bool Open(const TString& filename, E_FILE_ACCESS access);
		virtual void Close();

		virtual int32 Read(void* buffer, int32 buffer_size, int32 read_size) const;
		virtual int32 Write(const void* buffer, int32 size);
		virtual void Seek(int32 offset, bool relative = false);

		virtual int32 Tell() const;
		virtual bool IsEnd() const;
		virtual int32 BytesLeft() const;

		virtual char* GetMemoryPointer(int32 size);

		int32  GetSize() const
		{
			return Size;
		}
		const TString& GetFileName() const
		{
			return Filename;
		}
	protected:
		TString	Filename;
		FILE* File;
		int32 Size;
	};

	class TI_API TiFileBuffer : public TFile
	{
	public:
		TiFileBuffer(bool delete_buffer = true);
		virtual ~TiFileBuffer();

		virtual bool Open(const TString& filename, char* file_buffer, int32 size);
		virtual int32 Read(void* buffer, int32 buffer_size, int32 read_size) const;
		virtual void Seek(int32 offset, bool relative = false);

		virtual int32 Tell() const;
		virtual char* GetMemoryPointer(int32 size);

		virtual bool IsEnd() const;
		virtual int32 BytesLeft() const;
	protected:
		char* FileBuffer;
		bool DeleteBuffer;
		mutable int32 ReadPos;
	};
}