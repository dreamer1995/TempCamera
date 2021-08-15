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
	class DeferredVolumeFogApplyPass : public FullscreenPass
	{
	public:
		DeferredVolumeFogApplyPass(std::string name, Graphics& gfx)
			:
			FullscreenPass(std::move(name), gfx)
		{
			using namespace Bind;
			AddBindSink<RenderTarget>("scratchIn");
			RegisterSink(DirectBindableSink<RenderTarget>::Make("renderTarget", renderTarget));
			AddBind(PixelShader::Resolve(gfx, "VolumeMerge.cso"));
			AddBind(Blender::Resolve(gfx, true, Blender::BlendMode::OneMinus));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			RegisterSource(DirectBindableSource<RenderTarget>::Make("renderTarget", renderTarget));
		}
	};
}