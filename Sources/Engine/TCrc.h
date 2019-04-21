/*
	TiX Engine v2.0 Copyright (C) 2018~2019
	By ZhaoShuai tirax.cn@gmail.com
*/

#pragma once

namespace tix
{
	/**
	* CRC hash generation for different types of input data
	**/
	class TI_API TCrc
	{
	public:
		/** lookup table with precalculated CRC values - slicing by 8 implementation */
		static uint32 CRCTable[256];
		static uint32 CRCTablesSB8[8][256];

		/** generates CRC hash of the memory area */
		static uint32 MemCrc32(const void* Data, int32 Length, uint32 CRC = 0);

		/** generates hash for string */
		static uint32 StringHash(const int8* Data);
	};
}