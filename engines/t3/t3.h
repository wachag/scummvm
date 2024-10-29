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

#ifndef T3_H
#define T3_H

#include "common/array.h"

#include "graphics/framelimiter.h"

#include "engines/engine.h"

#include "engines/t3/gfx.h"
#include "engines/t3/detection.h"
namespace T3 {

class T3Engine : public Engine {
public:
	T3Engine(OSystem *syst, T3GameType gameType);
	~T3Engine() override;

	Common::Error run() override;

	T3GameType getGameType() { return _gameType; }

	bool hasFeature(EngineFeature f) const override;

	void processInput();

	void drawFrame(int testId);

private:
	OSystem *_system;
	Renderer *_gfx;
	Graphics::FrameLimiter *_frameLimiter;

	T3GameType _gameType;

	Math::Vector4d _clearColor;
	Math::Vector4d _fogColor;
	float _fade;
	bool _fadeIn;
	bool _fogEnable;
	Graphics::Surface *_rgbaTexture;
	Graphics::Surface *_rgbTexture;
	Graphics::Surface *_rgb565Texture;
	Graphics::Surface *_rgba5551Texture;
	Graphics::Surface *_rgba4444Texture;

	float _rotateAngleX, _rotateAngleY, _rotateAngleZ;

	Graphics::Surface *generateRgbaTexture(int width, int height, Graphics::PixelFormat format);
	void drawAndRotateCube();
	void drawPolyOffsetTest();
	void dimRegionInOut();
	void drawInViewport();
	void drawRgbaTexture();
};
	extern T3Engine *g_t3;

} // End of namespace T3

#endif // T3_H
