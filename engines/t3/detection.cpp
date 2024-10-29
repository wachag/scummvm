/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/advancedDetector.h"

#include "base/plugins.h"
#include "t3/t3.h"

static const PlainGameDescriptor t3_setting[] = {
	{ "t3", "T3: Telltale Tool engine" },
	{ nullptr, nullptr }
};

static const ADGameDescription t3Descriptions[] = {
	{
		"t3",
		"Tales of Monkey Island - Launch of the Screaming Narwhal (GOG edition)",
		AD_ENTRY1s("1_MonkeyIsland101_pc_data.ttarch", "85c1600abcac3baca56ff4fb6bf8d040",125174824),
		Common::EN_ANY,
		Common::kPlatformWindows,
		ADGF_NO_FLAGS,
		GUIO2(GUIO_NOLAUNCHLOAD, GUIO_NOMIDI								)
	},



	AD_TABLE_END_MARKER
};

class T3MetaEngineDetection : public AdvancedMetaEngineDetection<ADGameDescription> {
public:
	T3MetaEngineDetection() : AdvancedMetaEngineDetection(t3Descriptions, t3_setting) {
		_md5Bytes = 512;
	}

	const char *getName() const override {
		return "t3";
	}

	const char *getEngineName() const override {
		return "T3: Telltale Tool engine";
	}

	const char *getOriginalCopyright() const override {
		return "Copyright (C) Telltale Games";
	}
};

REGISTER_PLUGIN_STATIC(T3_DETECTION, PLUGIN_TYPE_ENGINE_DETECTION, T3MetaEngineDetection);
