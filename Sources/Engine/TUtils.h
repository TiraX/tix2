/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

inline void FromUtf8ToUnicode(const int8* Utf8Buffer, int32 Utf8BufferSize, int16* &OutUnicodeBuffer, int32& OutSize)
{
	OutUnicodeBuffer = ti_new int16[Utf8BufferSize + 1];

	const uint8 *p = (const uint8*)Utf8Buffer;
	int resultsize = 0;
	uint8 *tmp = nullptr;

	tmp = (uint8*)OutUnicodeBuffer;

	while (*p)
	{
		if (*p >= 0x00 && *p <= 0x7f)
		{
			*tmp = *p;
			++tmp;
			*tmp = '\0';
			++tmp;
			resultsize += 2;
		}
		else if ((*p & (0xff << 5)) == 0xc0)
		{
			uint8 t1 = 0;
			uint8 t2 = 0;

			t1 = *p & (0xff >> 3);
			p++;
			t2 = *p & (0xff >> 2);

			*tmp = t2 | ((t1 & (0xff >> 6)) << 6);//t1 >> 2;
			++tmp;
			*tmp = t1 >> 2;//t2 | ((t1 & (0xff >> 6)) << 6);
			++tmp;
			resultsize += 2;
		}
		else if ((*p & (0xff << 4)) == 0xe0)
		{
			uint8 t1 = 0;
			uint8 t2 = 0;
			uint8 t3 = 0;

			t1 = *p & (0xff >> 3);
			++p;
			t2 = *p & (0xff >> 2);
			++p;
			t3 = *p & (0xff >> 2);

			//Little Endian
			*tmp = ((t2 & (0xff >> 6)) << 6) | t3;//(t1 << 4) | (t2 >> 2);
			++tmp;
			*tmp = (t1 << 4) | (t2 >> 2);//((t2 & (0xff >> 6)) << 6) | t3;
			++tmp;

			resultsize += 2;
		}
		p++;
	}

	*tmp = '\0';
	++tmp;
	*tmp = '\0';
	resultsize += 2;

	OutSize = resultsize;
}

inline void FromUnicodeToUtf8(const int16* UnicodeBuffer, int32 UnicodeBufferSize, int8* &OutUtf8Buffer, int32& OutSize)
{
	int32 outsize = 0;
	uint8 *buffer = nullptr;
	uint8 *tmp = nullptr;

	const int32 s_len = UnicodeBufferSize;
	buffer = ti_new uint8[s_len * 3 + 1];
	tmp = buffer;

	for (int i = 0; i < s_len; i++)
	{
		uint16 unicode = UnicodeBuffer[i];

		if (unicode >= 0x0000 && unicode <= 0x007f)
		{
			*tmp = (uint8)unicode;
			++tmp;
			outsize += 1;
		}
		else if (unicode >= 0x0080 && unicode <= 0x07ff)
		{
			*tmp = 0xc0 | (unicode >> 6);
			++tmp;
			*tmp = 0x80 | (unicode & (0xff >> 2));
			++tmp;
			outsize += 2;
		}
		else if (unicode >= 0x0800 && unicode <= 0xffff)
		{
			*tmp = 0xe0 | ((unicode & 0xf000) >> 12);
			++tmp;
			*tmp = 0x80 | ((unicode & 0x0fc0) >> 6 & 0x00ff);
			++tmp;
			*tmp = 0x80 | (unicode & 0x003f);
			++tmp;
			outsize += 3;
		}
	}
	*tmp = '\0';
	++outsize;

	OutSize = outsize;

	if (outsize <= s_len * 3 + 1)
	{
		OutUtf8Buffer = ti_new int8[outsize];
		memcpy(OutUtf8Buffer, buffer, outsize);
		ti_delete[] buffer;
	}

	OutUtf8Buffer = (int8*)buffer;
}

inline TString FromWString(const TWString& WStr)
{
	int8 * Utf8Buffer = nullptr;
	int32 Utf8Size = 0;
	FromUnicodeToUtf8((const int16*)WStr.c_str(), (int32)WStr.length(), Utf8Buffer, Utf8Size);
	TString Result(Utf8Buffer);
	ti_delete[] Utf8Buffer;
	return Result;
}

inline TWString FromString(const TString& Str)
{
	int16* UnicodeBuffer = nullptr;
	int32 UnicodeSize = 0;
	FromUtf8ToUnicode((const int8*)Str.c_str(), (int32)Str.length(), UnicodeBuffer, UnicodeSize);
	TWString Result((const wchar_t*)UnicodeBuffer);
	ti_delete[] UnicodeBuffer;
	return Result;
}