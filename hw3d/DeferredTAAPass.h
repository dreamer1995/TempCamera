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
	class DeferredTAAPass : public FullscreenPass
	{
	public:
		DeferredTAAPass(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight)
			:
			FullscreenPass(std::move(name), gfx)
		{
			using namespace Bind;
			AddBindSink<RenderTarget>("scratchIn");
			currentRT = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 1u);
			historyRT = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 1u);
			renderTarget = currentRT;
			AddBind(PixelShader::Resolve(gfx, "TAA.cso"));
			AddBind(Blender::Resolve(gfx, false));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			AddBind(historyRT);
			RegisterSource(DirectBindableSource<ShaderInputRenderTarget>::Make("renderTarget", currentRT));
		}
		// this override is necessary because we cannot (yet) link input bindables directly into
		// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			FullscreenPass::Execute(gfx);
			//renderTarget = currentRT;
		}
	private:
		std::shared_ptr<Bind::ShaderInputRenderTarget> currentRT;
		std::shared_ptr<Bind::ShaderInputRenderTarget> historyRT;
	};
}