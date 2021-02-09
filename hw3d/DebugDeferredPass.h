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
	class DebugDeferredPass : public FullscreenPass
	{
	public:
		DebugDeferredPass(std::string name, Graphics& gfx)
			:
			FullscreenPass(std::move(name), gfx)
		{
			using namespace Bind;
			AddBind(PixelShader::Resolve(gfx, "DebugDeferred.cso"));
			AddBind(Blender::Resolve(gfx, true));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));

			AddBindSink<Bindable>("gbufferIn");
			AddBindSink<Bindable>("depthIn");
			RegisterSink(DirectBufferSink<RenderTarget>::Make("renderTarget", renderTarget));
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));

			RegisterSource(DirectBufferSource<RenderTarget>::Make("renderTarget", renderTarget));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
		}
	};
}