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
	class DeferredHDRPass : public FullscreenPass
	{
	public:
		DeferredHDRPass(std::string name, Graphics& gfx)
			:
			FullscreenPass(std::move(name), gfx)
		{
			using namespace Bind;
			AddBindSink<RenderTarget>("scratchIn");
			RegisterSink(DirectBufferSink<RenderTarget>::Make("renderTarget", renderTarget));
			AddBind(PixelShader::Resolve(gfx, "HDR.cso"));
			AddBind(Blender::Resolve(gfx, false));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			RegisterSource(DirectBufferSource<RenderTarget>::Make("renderTarget", renderTarget));
		}
	};
}