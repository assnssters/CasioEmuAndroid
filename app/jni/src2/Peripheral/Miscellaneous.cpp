﻿#include "Miscellaneous.hpp"

#include "../Chipset/CPU.hpp"
#include "../Chipset/Chipset.hpp"
#include "../Emulator.hpp"
#include "../Logger.hpp"

#include <iomanip>
#include <sstream>

namespace casioemu {
	class Miscellaneous : public Peripheral {
		MMURegion region_dsr, region_F004, region_F046;

		static constexpr uint16_t addr[] = {
			// 0xF033, // both HW_ES_PLUS, HW_CLASSWIZ and HW_CLASSWIZ_II
			0xF035, 0xF036, /* 0xF039,*/ 0xF03D // HW_CLASSWIZ and HW_CLASSWIZ_II
		};
		static constexpr int N_BYTE = sizeof(addr) / sizeof(addr[0]);
		MMURegion region[N_BYTE];
		uint8_t data[N_BYTE];

	public:
		using Peripheral::Peripheral;

		void Initialise();
		void Tick();
		void Reset();
	};
	constexpr uint16_t Miscellaneous::addr[];

	void Miscellaneous::Initialise() {
		region_dsr.Setup(
			0xF000, 1, "Miscellaneous/DSR", this, [](MMURegion* region, size_t) { return (uint8_t)((Miscellaneous*)region->userdata)->emulator.chipset.cpu.impl_last_dsr; }, [](MMURegion* region, size_t, uint8_t data) {
			Miscellaneous* self = (Miscellaneous *)region->userdata;
			self->emulator.chipset.cpu.impl_last_dsr = data & self->emulator.chipset.cpu.dsr_mask; }, emulator);

		if (emulator.hardware_id == HW_CLASSWIZ) {
			// Only tested on fx-991cnx
			region_F004.Setup(
				0xF004, 1, "Miscellaneous/DataSegAccess", this, [](MMURegion* region, size_t) { return (uint8_t)((Miscellaneous*)region->userdata)->emulator.chipset.SegmentAccess; }, [](MMURegion* region, size_t, uint8_t data) {
				Miscellaneous* self = (Miscellaneous *)region->userdata;
				self->emulator.chipset.SegmentAccess = data & 1; }, emulator);
		}

		// * TODO: figure out what these are

		int n_byte;
		switch (emulator.hardware_id) {
		case HW_ES_PLUS:
		case HW_FX_5800P:
			n_byte = 0;
			break;
		case HW_CLASSWIZ_II:
		case HW_CLASSWIZ:
			n_byte = 3;
			break;
		default:
			PANIC("Unknown hardware_id\n");
		}
		for (int i = 0; i < n_byte; ++i) {
			std::ostringstream stream;
			stream << "Miscellaneous/Unknown/" << std::hex << std::uppercase << addr[i] << "*1";
			region[i].Setup(addr[i], 1, stream.str(), &data[i], MMURegion::DefaultRead<uint8_t>, MMURegion::DefaultWrite<uint8_t>, emulator);
		}
	}

	void Miscellaneous::Tick() {
	}

	void Miscellaneous::Reset() {
		if (emulator.hardware_id == HW_FX_5800P) {
			emulator.chipset.InputToPort(0, 3, true);
		}
	}
	Peripheral* CreateMiscellaneous(Emulator& emu) {
		return new Miscellaneous(emu);
	}
} // namespace casioemu
