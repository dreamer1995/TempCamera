#include "PreCalBlurCube.h"
#include "Sink.h"
#include "Source.h"
#include "PixelShader.h"
#include "Stencil.h"
#include "Sampler.h"

using namespace Bind;

namespace Rgph
{
	PreCalBlurCube::PreCalBlurCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight)
		:
		PreCubeCalculatePass(std::move(name), gfx)
	{
		AddBind(PixelShader::Resolve(gfx, "BlurOutline_PS.cso"));
		AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
		AddBind(Sampler::Resolve(gfx, Sampler::Type::Bilinear, true));

		AddBindSink<Bind::RenderTarget>("HDIn");

		renderTarget = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 5u);
	}

	// see the note on HorizontalBlurPass::Execute
	void PreCalBlurCube::Execute(Graphics& gfx) const noxnd
	{
		PreCubeCalculatePass::Execute(gfx);
	}
}
