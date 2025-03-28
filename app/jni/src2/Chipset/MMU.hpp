﻿#pragma once
#include "../Config.hpp"

#include "MMURegion.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace casioemu
{
	class Emulator;

	class MMU
	{
	private:
		Emulator &emulator;

		bool real_hardware;

		struct MemoryByte
		{
			MMURegion *region;
		};
		MemoryByte **segment_dispatch;
		std::vector<MMURegion*> regions;
	public:
		MMU(Emulator &emulator);
		~MMU();
		void SetupInternals();
		void GenerateSegmentDispatch(size_t segment_index);
		uint16_t ReadCode(size_t offset);
		uint8_t ReadData(size_t offset, bool softwareRead = true);
		void WriteData(size_t offset, uint8_t data, bool softwareWrite = true);
		size_t getRealOffset(size_t offset);


		std::vector<MMURegion*> GetRegions();
		void RegisterRegion(MMURegion *region);
		void UnregisterRegion(MMURegion *region);
	};
}

