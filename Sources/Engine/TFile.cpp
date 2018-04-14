/*
	TiX Engine v2.0 Copyright (C) 2018
	By ZhaoShuai tirax.cn@gmail.com
*/

#include "stdafx.h"
#include "TFile.h"

namespace tix
{
	const char* k_file_access[] =
	{
		"rb",
		"ab",
		"wb",
		"rwb",
	};

	TFile::TFile()
		: File(nullptr)
		, Size(0)
	{
	}

	TFile::~TFile()
	{
		Close();
	}

//#include "direct.h"

	bool TFile::Open(const TString& filename, E_FILE_ACCESS access)
	{

		//char curr_path[256];
		//_getcwd(curr_path, 256);

		Close();
		Filename = filename;

		errno_t ret = fopen_s(&File, filename.c_str(), k_file_access[access]);

		if (ret == 0)
		{
			fseek(File, 0, SEEK_END);
			Size = ftell(File);
			fseek(File, 0, SEEK_SET);
			return true;
		}
		return false;
	}

	void TFile::Close()
	{
		if (File)
		{
			fclose(File);
			File = nullptr;
		}
	}

	int TFile::Read(void* buffer, int buffer_size, int read_size) const
	{
		if (!File)
			return 0;
		if (read_size > buffer_size)
			read_size = buffer_size;
		return (int32)fread(buffer, 1, read_size, File);
	}

	int TFile::Write(const void* buffer, int size)
	{
		if (!File)
			return 0;
		return (int32)fwrite(buffer, 1, size, File);
	}

	void TFile::Seek(int offset, bool relative /* = false */)
	{
		fseek(File, offset, relative ? SEEK_CUR : SEEK_SET);
	}

	int TFile::Tell() const
	{
		return ftell(File);
	}

	bool TFile::IsEnd() const
	{
		int pos = ftell(File);
		return pos >= Size;
	}

	int TFile::BytesLeft() const
	{
		int pos = ftell(File);
		return Size - pos;
	}
	
	char* TFile::GetMemoryPointer(int size)
	{
		TI_ASSERT(0);	// not supported in TiFile, only in TiFileBuffer;
		return nullptr;
	}

	//////////////////////////////////////////////////////////////////////////

	TiFileBuffer::TiFileBuffer(bool delete_buffer)
		: FileBuffer(nullptr)
		, ReadPos(0)
		, DeleteBuffer(delete_buffer)
	{
	}

	TiFileBuffer::~TiFileBuffer()
	{
		if (DeleteBuffer)
		{
			ti_delete[] FileBuffer;
		}
	}

	bool TiFileBuffer::Open(const TString& filename, char* file_buffer, int size)
	{
		Filename	= filename;
		Size		= size;
		FileBuffer	= file_buffer;
		ReadPos		= 0;
		return true;
	}

	int TiFileBuffer::Read(void* buffer, int buffer_size, int read_size) const
	{
		TI_ASSERT(FileBuffer);
		if (read_size > buffer_size)
			read_size	= buffer_size;

		memcpy(buffer, FileBuffer + ReadPos, read_size);
		ReadPos			+= read_size;
		return read_size;
	}

	char* TiFileBuffer::GetMemoryPointer(int read_size)
	{
		TI_ASSERT(Size - ReadPos >= read_size);
		char* result	= FileBuffer + ReadPos;
		ReadPos			+= read_size;
		return result;
	}

	void TiFileBuffer::Seek(int offset, bool relative /* = false */)
	{
		if (relative)
			ReadPos		+= offset;
		else
			ReadPos		= offset;
	}

	int TiFileBuffer::Tell() const
	{
		return ReadPos;
	}

	bool TiFileBuffer::IsEnd() const
	{
		return ReadPos >= Size;
	}

	int TiFileBuffer::BytesLeft() const
	{
		return Size - ReadPos;
	}
}
