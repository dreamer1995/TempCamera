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
	PreCalSimpleCube::PreCalSimpleCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight, const std::string& path)
		:
		PreCubeCalculatePass(std::move(name), gfx)
	{
		AddBind(Texture::Resolve(gfx, path));
		AddBind(PixelShader::Resolve(gfx, "SphereToCubePS.cso"));
		AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
		AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Wrap, 0u, 0b1u, 0.0f));

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

	void PreCalSimpleCube::DumpShadowMap(Graphics& gfx, const std::string& path) const
	{
		pPreCalSimpleCube->ToCube(gfx, path);
	}
}
