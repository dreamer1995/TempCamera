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
	class PreCalMipCube : public PreCubeCalculatePass
	{
	public:
		PreCalMipCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight);
		void Execute(Graphics& gfx) const noxnd override;
		void DumpCubeMap(Graphics& gfx, const std::string& path) const;
	public:
		std::shared_ptr<Bind::ShaderInputRenderTarget> pPreCalMipCube;
	private:
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> roughness;
	};
}