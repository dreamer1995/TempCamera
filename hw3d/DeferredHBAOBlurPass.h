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
	class DeferredHBAOBlurPass : public FullscreenPass
	{
	public:
		DeferredHBAOBlurPass(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			FullscreenPass(std::move(name), gfx),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			AddBind(masterDepth);
			AddBindSink<Bind::CachingPixelConstantBufferEx>("AOParams");
			AddBindSink<RenderTarget>("scratchIn");
			tempRT = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 1u);
			RegisterSink(DirectBindableSink<RenderTarget>::Make("renderTarget", renderTarget));
			AddBind(PixelShader::Resolve(gfx, "HBAOBlur.cso"));
			AddBind(Blender::Resolve(gfx, true, Blender::BlendMode::OneMinus));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));

			Dcb::RawLayout lay;
			lay.Add<Dcb::Bool>("isHorizontal");
			auto buf = Dcb::Buffer(std::move(lay));
			direction = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 11u);
			AddBind(direction);

			RegisterSource(DirectBindableSource<RenderTarget>::Make("renderTarget", renderTarget));
		}
		// this override is necessary because we cannot (yet) link input bindables directly into
		// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			const_cast<DeferredHBAOBlurPass*>(this)->finalRT = renderTarget;
			const_cast<DeferredHBAOBlurPass*>(this)->renderTarget = tempRT;
			masterDepth->BreakRule();
			auto buf = direction->GetBuffer();
			buf["isHorizontal"] = true;
			direction->SetBuffer(buf);

			FullscreenPass::Execute(gfx);

			gfx.ClearShaderResources(8u);

			const_cast<DeferredHBAOBlurPass*>(this)->renderTarget = finalRT;
			masterDepth->BreakRule();
			gfx.ClearRenderTarget();
			tempRT->Bind(gfx);
			buf["isHorizontal"] = false;
			direction->SetBuffer(buf);

			FullscreenPass::Execute(gfx);

			gfx.ClearShaderResources(8u);
		}
	private:
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
		std::shared_ptr<Bind::ShaderInputRenderTarget> tempRT;
		std::shared_ptr<Bind::RenderTarget> finalRT;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> direction;
	};
}