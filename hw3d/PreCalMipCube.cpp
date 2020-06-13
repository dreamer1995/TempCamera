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
		AddBind(PixelShader::Resolve(gfx, "PrefilterMapPixelShader.cso"));
		AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
		AddBind(Sampler::Resolve(gfx, Sampler::Type::Bilinear));
		Dcb::RawLayout l;
		l.Add<Dcb::Float>("roughness");
		Dcb::Buffer buf{ std::move(l) };
		roughness = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 4u);
		AddBind(roughness);

		AddBindSink<Bind::RenderTarget>("HDIn");

		pPreCalMipCube = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 12u, ShaderInputRenderTarget::Type::PreCalMipCube);
		renderTarget = pPreCalMipCube;
	}

	// see the note on HorizontalBlurPass::Execute
	void PreCalMipCube::Execute(Graphics& gfx) const noxnd
	{
		for (short int i = 0; i < 5; i++)
		{
			if (i != 0)
			{
				pPreCalMipCube->ChangeMipSlice(gfx, i);
			}
			pPreCalMipCube->_width = pPreCalMipCube->_height = 256u * (float)pow(0.5, i);
			auto k = roughness->GetBuffer();
			k["roughness"] = i / 4.0f;
			roughness->SetBuffer(k);
			for (short int j = 0; j < 6; j++)
			{
				gfx.SetCamera(viewmatrix[j]);
				pPreCalMipCube->targetIndex = j;
				PreCubeCalculatePass::Execute(gfx);
			}
		}
	}
}
