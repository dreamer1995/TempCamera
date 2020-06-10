#include "PreCalSimpleCube.h"
#include "Sink.h"
#include "Source.h"
#include "PixelShader.h"
#include "Stencil.h"
#include "Sampler.h"

using namespace Bind;

namespace Rgph
{
	PreCalSimpleCube::PreCalSimpleCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight)
		:
		PreCubeCalculatePass(std::move(name), gfx)
	{
		AddBind(PixelShader::Resolve(gfx, "BlurOutline_PS.cso"));
		AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
		AddBind(Sampler::Resolve(gfx, Sampler::Type::Bilinear, true));

		pPreCalSimpleCube = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullHeight, fullHeight, 0u);
		renderTarget = pPreCalSimpleCube;
		RegisterSource(DirectBindableSource<RenderTarget>::Make("HDOut", renderTarget));
	}

	// see the note on HorizontalBlurPass::Execute
	void PreCalSimpleCube::Execute(Graphics& gfx) const noxnd
	{
		PreCubeCalculatePass::Execute(gfx);
	}
}
