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
		AddBind(PixelShader::Resolve(gfx, "SkyboxConvolutionPS.cso"));
		AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
		AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear));

		AddBindSink<Bind::RenderTarget>("HDIn");

		pPreCalBlurCube = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 11u, ShaderInputRenderTarget::Type::PreCalSimpleCube);
		renderTarget = pPreCalBlurCube;
	}

	// see the note on HorizontalBlurPass::Execute
	void PreCalBlurCube::Execute(Graphics& gfx) const noxnd
	{
		for (unsigned char i = 0; i < 6; i++)
		{
			gfx.SetCamera(viewmatrix[i]);
			pPreCalBlurCube->targetIndex = i;
			PreCubeCalculatePass::Execute(gfx);
		}
	}	
	
	void PreCalBlurCube::DumpCubeMap(Graphics& gfx, const std::string& path) const
	{
		pPreCalBlurCube->ToCube(gfx, path);
	}
}
