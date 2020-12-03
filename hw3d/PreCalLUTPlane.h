#pragma once
#include "FullscreenPass.h"

namespace dx = DirectX;

class Graphics;
namespace Bind
{
	class PixelShader;
	class RenderTarget;
}

namespace Rgph
{
	class PreCalLUTPlane : public FullscreenPass
	{
	public:
		PreCalLUTPlane(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight);
		void DumpShadowMap(Graphics& gfx, const std::string& path) const;
	public:
		std::shared_ptr<Bind::ShaderInputRenderTarget> pPreCalLUTPlane;
	};
}