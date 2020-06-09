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
	class PreCalBlurCube : public PreCubeCalculatePass
	{
	public:
		PreCalBlurCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight);
		void Execute(Graphics& gfx) const noxnd override;
	private:
		//std::shared_ptr<Bind::CachingPixelConstantBufferEx> direction;
	};
}