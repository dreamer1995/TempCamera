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
	class DeferredSunLightPass : public FullscreenPass
	{
	public:
		DeferredSunLightPass(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			FullscreenPass(std::move(name), gfx),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			pDShadowCBuf = std::make_shared<Bind::ShadowCameraCBuf>(gfx, 5u, 0b1u);
			AddBind(pDShadowCBuf);
			AddBindSink<Bindable>("dShadowMap");
			AddBind(PixelShader::Resolve(gfx, "DeferredSunLight.cso"));
			AddBind(Blender::Resolve(gfx, false));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			AddBindSink<Bindable>("gbufferIn");
			AddBind(masterDepth);
			AddBindSink<Bindable>("shadowControl");
			AddBindSink<Bindable>("shadowSampler");
			AddBindSink<Bindable>("cubeMapBlurIn");
			AddBindSink<Bindable>("cubeMapMipIn");
			AddBindSink<Bindable>("planeBRDFLUTIn");
			renderTarget = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 0u,
				RenderTarget::Type::Default, 0b1u, DXGI_FORMAT_R32G32B32A32_FLOAT);
			RegisterSource(DirectBindableSource<RenderTarget>::Make("renderTarget", renderTarget));
		}
		void BindMainCamera(const Camera& cam) noexcept
		{
			pMainCamera = &cam;
		}
		void BindShadowCamera(Graphics& gfx, const Camera& dCam) noexcept
		{
			pDShadowCBuf->SetCamera(&dCam);
		}
		// this override is necessary because we cannot (yet) link input bindables directly into
		// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			assert(pMainCamera);
			pMainCamera->BindToGraphics(gfx);
			masterDepth->BreakRule();
			pDShadowCBuf->Update(gfx);

			FullscreenPass::Execute(gfx);

			gfx.ClearShaderResources(8u);
		}
	private:
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
		const Camera* pMainCamera = nullptr;
		std::shared_ptr<Bind::ShadowCameraCBuf> pDShadowCBuf;
	};
}