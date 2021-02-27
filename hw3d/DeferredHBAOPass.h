#pragma once
#include "FullscreenPass.h"
#include "Sink.h"
#include "Source.h"
#include "PixelShader.h"
#include "Blender.h"
#include "Stencil.h"
#include "Sampler.h"
#include "ConstantBuffersEx.h"

class Graphics;
namespace Bind
{
	class PixelShader;
	class RenderTarget;
}

namespace Rgph
{
	class DeferredHBAOPass : public FullscreenPass
	{
	public:
		DeferredHBAOPass(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			FullscreenPass(std::move(name), gfx),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			AddBind(masterDepth);
			AddBindSink<Bind::CachingPixelConstantBufferEx>("AOParams");
			renderTarget = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 0u);
			AddBind(PixelShader::Resolve(gfx, "HBAO.cso"));
			AddBind(Blender::Resolve(gfx, false));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			RegisterSource(DirectBindableSource<RenderTarget>::Make("scratchOut", renderTarget));
		}
		// this override is necessary because we cannot (yet) link input bindables directly into
		// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			masterDepth->BreakRule();

			FullscreenPass::Execute(gfx);

			gfx.ClearShaderResources(8u);
		}
	private:
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
	};
}