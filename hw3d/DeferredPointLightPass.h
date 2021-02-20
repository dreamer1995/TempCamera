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
	class DeferredPointLightPass : public FullscreenPass
	{
	public:
		DeferredPointLightPass(std::string name, Graphics& gfx, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			FullscreenPass(std::move(name), gfx),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			AddBindSink<Bindable>("pShadowMap0");
			AddBindSink<Bindable>("pShadowMap1");
			AddBindSink<Bindable>("pShadowMap2");
			AddBind(PixelShader::Resolve(gfx, "DeferredPointLight.cso"));
			AddBind(Blender::Resolve(gfx, true, Blender::BlendMode::Additive));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			AddBindSink<Bindable>("gbufferIn");
			AddBind(masterDepth);
			AddBindSink<Bindable>("shadowControl");
			AddBindSink<Bindable>("shadowSampler");
			RegisterSink(DirectBindableSink<RenderTarget>::Make("renderTarget", renderTarget));
			RegisterSource(DirectBindableSource<RenderTarget>::Make("renderTarget", renderTarget));
		}
		void BindMainCamera(const Camera& cam) noexcept
		{
			pMainCamera = &cam;
		}
		void BindShadowCamera(Graphics& gfx, std::vector<std::shared_ptr<PointLight>> pCams) noexcept
		{
			for (unsigned char i = 0; i < pCams.size(); i++)
			{
				pPShadowCBufs.emplace_back(std::make_shared<Bind::ShadowCameraCBuf>(gfx, 10u + i, 0b1u));
				pPShadowCBufs[i]->SetPointLight(pCams[i]);
				AddBind(pPShadowCBufs[i]);
			}
		}
		// this override is necessary because we cannot (yet) link input bindables directly into
		// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			assert(pMainCamera);
			pMainCamera->BindToGraphics(gfx);
			masterDepth->BreakRule();
			for (unsigned char i = 0; i < pPShadowCBufs.size(); i++)
				pPShadowCBufs[i]->UpdatePointLight(gfx);

			gfx.ClearConstantBuffers(11u);

			FullscreenPass::Execute(gfx);

			gfx.ClearShaderResources(8u);
		}
	private:
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
		const Camera* pMainCamera = nullptr;
		std::vector<std::shared_ptr<Bind::ShadowCameraCBuf>> pPShadowCBufs;
	};
}