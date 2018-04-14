/*
	TiX Engine v2.0 Copyright (C) 2018
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

		virtual int  Read(void* buffer, int buffer_size, int read_size) const;
		virtual int  Write(const void* buffer, int size);
		virtual void Seek(int offset, bool relative = false);

		virtual int  Tell() const;
		virtual bool IsEnd() const;
		virtual int  BytesLeft() const;

		virtual char*	GetMemoryPointer(int size);

		int  GetSize() const
		{
			return Size;
		}
		const TString& GetFileName() const
		{
			return Filename;
		}
	protected:
		TString	Filename;
		FILE*		File;
		int			Size;
	};

	class TI_API TiFileBuffer : public TFile
	{
	public:
		TiFileBuffer(bool delete_buffer = true);
		virtual ~TiFileBuffer();

		virtual bool	Open(const TString& filename, char* file_buffer, int size);
		virtual int		Read(void* buffer, int buffer_size, int read_size) const;
		virtual void	Seek(int offset, bool relative = false);

		virtual int		Tell() const;
		virtual char*	GetMemoryPointer(int size);

		virtual bool	IsEnd() const;
		virtual int		BytesLeft() const;
	protected:
		char*			FileBuffer;
		bool			DeleteBuffer;
		mutable int		ReadPos;
	};
}