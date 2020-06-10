#include "PreCalMipCube.h"
#include "Sink.h"
#include "Source.h"
#include "PixelShader.h"
#include "Stencil.h"
#include "Sampler.h"

using namespace Bind;

namespace Rgph
{
	PreCalMipCube::PreCalMipCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight)
		:
		PreCubeCalculatePass(std::move(name), gfx)
	{
		AddBind(PixelShader::Resolve(gfx, "Solid_PS.cso"));
		AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
		AddBind(Sampler::Resolve(gfx, Sampler::Type::Bilinear, true));

		AddBindSink<Bind::RenderTarget>("HDIn");

		renderTarget = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 6u);
	}

	// see the note on HorizontalBlurPass::Execute
	void PreCalMipCube::Execute(Graphics& gfx) const noxnd
	{
		PreCubeCalculatePass::Execute(gfx);
	}
}
