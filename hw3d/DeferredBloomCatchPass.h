#pragma once
#include "FullscreenPass.h"
#include "Sink.h"
#include "Source.h"
#include "PixelShader.h"
#include "Blender.h"
#include "Stencil.h"
#include "Sampler.h"

class Graphics;
namespace Bind
{
	class PixelShader;
	class RenderTarget;
}

namespace Rgph
{
	class DeferredBloomCatchPass : public FullscreenPass
	{
	public:
		DeferredBloomCatchPass(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight)
			:
			FullscreenPass(std::move(name), gfx)
		{
			using namespace Bind;
			AddBindSink<RenderTarget>("TAA0");
			AddBindSink<RenderTarget>("TAA1");
			AddBindSink<CachingPixelConstantBufferEx>("bloomParams");
			renderTarget = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 0u,
				RenderTarget::Type::Default, 0b1u, DXGI_FORMAT_R32G32B32A32_FLOAT);
			AddBind(PixelShader::Resolve(gfx, "BloomCatch.cso"));
			AddBind(Blender::Resolve(gfx, false));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			RegisterSource(DirectBindableSource<RenderTarget>::Make("scratchOut", renderTarget));
		}
	};
}