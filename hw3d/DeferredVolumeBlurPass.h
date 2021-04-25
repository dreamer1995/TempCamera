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
	class DeferredVolumeBlurPass : public FullscreenPass
	{
	public:
		DeferredVolumeBlurPass(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			FullscreenPass(std::move(name), gfx),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			AddBindSink<RenderTarget>("scratchIn");
			tempRT = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 1u,
				RenderTarget::Type::Default, 0b1u, DXGI_FORMAT_R32G32B32A32_FLOAT);
			AddBind(PixelShader::Resolve(gfx, "VolumeBlur.cso"));
			AddBind(Blender::Resolve(gfx, true, Blender::BlendMode::Additive));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			AddBind(masterDepth);
			RegisterSink(DirectBindableSink<RenderTarget>::Make("renderTarget", renderTarget));
			RegisterSource(DirectBindableSource<RenderTarget>::Make("renderTarget", renderTarget));

			Dcb::RawLayout lay;
			lay.Add<Dcb::Bool>("isHorizontal");
			auto buf = Dcb::Buffer(std::move(lay));
			direction = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
			AddBind(direction);
		}
		// this override is necessary because we cannot (yet) link input bindables directly into
		// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			const_cast<DeferredVolumeBlurPass*>(this)->finalRT = renderTarget;

			masterDepth->BreakRule();
			
			auto buf = direction->GetBuffer();
			buf["isHorizontal"] = true;
			direction->SetBuffer(buf);

			const_cast<DeferredVolumeBlurPass*>(this)->renderTarget = tempRT;

			FullscreenPass::Execute(gfx);

			gfx.ClearRenderTarget();
			tempRT->Bind(gfx);
			buf["isHorizontal"] = false;
			direction->SetBuffer(buf);

			const_cast<DeferredVolumeBlurPass*>(this)->renderTarget = finalRT;

			FullscreenPass::Execute(gfx);

			tempRT->Clear(gfx);
			gfx.ClearShaderResources(8u);
		}
	private:
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> direction;
		std::shared_ptr<Bind::ShaderInputRenderTarget> tempRT;
		std::shared_ptr<Bind::RenderTarget> finalRT;
	};
}