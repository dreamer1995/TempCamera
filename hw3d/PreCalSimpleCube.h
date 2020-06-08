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
	class VerticalBlurPass : public PreCubeCalculatePass
	{
	public:
		VerticalBlurPass(std::string name, Graphics& gfx);
		void Execute(Graphics& gfx) const noxnd override;
	private:
		//std::shared_ptr<Bind::CachingPixelConstantBufferEx> direction;
	};
}