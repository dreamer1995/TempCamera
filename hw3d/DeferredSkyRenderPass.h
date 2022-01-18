#pragma once
#include "FullscreenPass.h"
#include "Sink.h"
#include "Source.h"
#include "PixelShader.h"

class Graphics;
namespace Bind
{
	class PixelShader;
	class RenderTarget;
}

namespace Rgph
{
	class DeferredSkyRenderPass : public FullscreenPass
	{
	public:
		DeferredSkyRenderPass(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			FullscreenPass(std::move(name), gfx),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			AddBindSink<RenderTarget>("transmittanceLutIn");
			AddBindSink<UnorderedAccessView>("scatteringLutIn");
			AddBindSink<CachingPixelConstantBufferEx>("skyConstants");
			renderTarget = std::make_shared<ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 4u, RenderTarget::Type::Default,
				0b100001u, DXGI_FORMAT_R32G32B32A32_FLOAT);
			pDShadowCBuf = std::make_shared<Bind::ShadowCameraCBuf>(gfx, 5u, 0b1u);
			AddBind(pDShadowCBuf);
			AddBindSink<Bindable>("dShadowMap");
			AddBind(Texture::Resolve(gfx, "Images\\bluenoise.exr", 10u));
			AddBind(PixelShader::Resolve(gfx, "Sky.cso"));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Blender::Resolve(gfx, true, Blender::BlendMode::Additive));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u, 0b1u));
			AddBind(masterDepth);
			AddBindSink<Bindable>("shadowControl");
			AddBindSink<Bindable>("shadowSampler");
			RegisterSource(DirectBindableSource<RenderTarget>::Make("scratchOut", renderTarget));
		}
		void BindShadowCamera(Graphics& gfx, const Camera& dCam) noexcept
		{
			pDShadowCBuf->SetCamera(&dCam);
		}
		// this override is necessary because we cannot (yet) link input bindables directly into
		// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			masterDepth->BreakRule();
			pDShadowCBuf->Update(gfx);

			BindAll(gfx);
			FullscreenPass::Execute(gfx);

			gfx.ClearShaderResources(8u);
		}

	private:
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
		std::shared_ptr<Bind::ShadowCameraCBuf> pDShadowCBuf;
	};
}