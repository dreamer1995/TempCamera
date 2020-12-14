#pragma once
#include "PreCubeCalculatePass.h"

class Graphics;
namespace Bind
{
	class PixelShader;
	class RenderTarget;
}

namespace Rgph
{
	class PreCalBlurCube : public PreCubeCalculatePass
	{
	public:
		PreCalBlurCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight);
		void Execute(Graphics& gfx) const noxnd override;
		void DumpCubeMap(Graphics& gfx, const std::string& path) const;
	public:
		std::shared_ptr<Bind::ShaderInputRenderTarget> pPreCalBlurCube;
	};
}