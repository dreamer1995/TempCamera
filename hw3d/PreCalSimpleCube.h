#pragma once
#include "PreCubeCalculatePass.h"
#include "ConstantBuffersEx.h"

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
		PreCalSimpleCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight);
		void Execute(Graphics& gfx) const noxnd override;
	public:
		std::shared_ptr<Bind::ShaderInputRenderTarget> pPreCalSimpleCube;
	};
}