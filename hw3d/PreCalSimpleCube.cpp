#include "PreCalSimpleCube.h"
#include "Sink.h"
#include "Source.h"
#include "PixelShader.h"
#include "Stencil.h"
#include "Sampler.h"
#include "Texture.h"

using namespace Bind;

namespace Rgph
{
	PreCalSimpleCube::PreCalSimpleCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight)
		:
		PreCubeCalculatePass(std::move(name), gfx)
	{
		AddBind(Texture::Resolve(gfx, "Images\\EpicQuadPanorama_CC+EV1.jpg"));
		AddBind(PixelShader::Resolve(gfx, "SphereToCubePS.cso"));
		AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
		AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear));

		pPreCalSimpleCube = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullHeight, fullHeight, 0u, ShaderInputRenderTarget::Type::PreCalSimpleCube);
		renderTarget = pPreCalSimpleCube;
		RegisterSource(DirectBindableSource<RenderTarget>::Make("HDOut", renderTarget));
	}

	// see the note on HorizontalBlurPass::Execute
	void PreCalSimpleCube::Execute(Graphics& gfx) const noxnd
	{
		for (short int i = 0; i < 6; i++)
		{
			gfx.SetCamera(viewmatrix[i]);
			pPreCalSimpleCube->targetIndex = i;
			PreCubeCalculatePass::Execute(gfx);
		}
	}
}
