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
		DebugDeferredPass(std::string name, Graphics& gfx, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			FullscreenPass(std::move(name), gfx),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			AddBind(PixelShader::Resolve(gfx, "DebugDeferred.cso"));
			AddBind(Blender::Resolve(gfx, true));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			AddBindSink<Bindable>("gbufferIn");
			AddBind(masterDepth);
			RegisterSink(DirectBufferSink<RenderTarget>::Make("renderTarget", renderTarget));

			RegisterSource(DirectBufferSource<RenderTarget>::Make("renderTarget", renderTarget));
		}

		// this override is necessary because we cannot (yet) link input bindables directly into
	// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			masterDepth->BreakRule();
			FullscreenPass::Execute(gfx);
		}

	private:
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
	};
}