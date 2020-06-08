#include "PreCalSimpleCube.h"
#include "Sink.h"
#include "Source.h"
#include "PixelShader.h"
#include "Stencil.h"
#include "Sampler.h"

using namespace Bind;

namespace Rgph
{
	VerticalBlurPass::VerticalBlurPass(std::string name, Graphics& gfx)
		:
		PreCubeCalculatePass(std::move(name), gfx)
	{
		AddBind(PixelShader::Resolve(gfx, "BlurOutline_PS.cso"));
		AddBind(Stencil::Resolve(gfx, Stencil::Mode::Off));
		AddBind(Sampler::Resolve(gfx, Sampler::Type::Bilinear, true));

		RegisterSink(DirectBufferSink<RenderTarget>::Make("renderTarget", renderTarget));
		RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));

		RegisterSource(DirectBufferSource<RenderTarget>::Make("renderTarget", renderTarget));
		RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
	}

	// see the note on HorizontalBlurPass::Execute
	void VerticalBlurPass::Execute(Graphics& gfx) const noxnd
	{
		PreCubeCalculatePass::Execute(gfx);
	}
}
