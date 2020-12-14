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
	class PreCalSimpleCube : public PreCubeCalculatePass
	{
	public:
		PreCalSimpleCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight, const std::string& path);
		void Execute(Graphics& gfx) const noxnd override;
		void DumpCubeMap(Graphics& gfx, const std::string& path) const;
	public:
		std::shared_ptr<Bind::ShaderInputRenderTarget> pPreCalSimpleCube;
	};
}