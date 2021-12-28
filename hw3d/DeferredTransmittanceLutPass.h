#pragma once
#include "ComputeShaderPass.h"
#include "Sink.h"
#include "Source.h"
#include "Sampler.h"
#include "ComputeShader.h"

class Graphics;
namespace Bind
{
	class UnorderedAccessView;
	class ComputeShader;
}

namespace Rgph
{
	class DeferredTransmittanceLutPass : public ComputeShaderPass
	{
	public:
		DeferredTransmittanceLutPass(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			ComputeShaderPass(std::move(name), gfx),
			width(fullWidth),
			height(fullHeight),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			AddBindSink<RenderTarget>("scratchIn");
			//AddBindSink<Bind::CachingPixelConstantBufferEx>("volumeParams");
			unorderedAccessView = std::make_shared<ShaderInputUAV>(gfx, fullWidth, fullHeight, 0u);
			pDShadowCBuf = std::make_shared<Bind::ShadowCameraCBuf>(gfx, 5u, 0b100000u);
			AddBind(pDShadowCBuf);
			AddBindSink<Bindable>("dShadowMap");
			AddBind(ComputeShader::Resolve(gfx, "VolumeFog3DTexture.cso"));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u, 0b100000u));
			AddBind(masterDepth);
			AddBind(Blender::Resolve(gfx, false));
			AddBindSink<Bindable>("shadowControl");
			AddBindSink<Bindable>("shadowSampler");
			RegisterSource(DirectBindableSource<UnorderedAccessView>::Make("scratchOut", unorderedAccessView));
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
			gfx.ClearRenderTarget();

			ComputeShaderPass::SetDispatchVector((width + 7u) >> 3u, (height + 7u) >> 3u, 1u);
			ComputeShaderPass::Execute(gfx);

			gfx.ClearShaderResources(8u);
			gfx.ClearUAV(0u);
		}
	private:
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
		const Camera* pMainCamera = nullptr;
		std::shared_ptr<Bind::ShadowCameraCBuf> pDShadowCBuf;
		UINT width;
		UINT height;
	};
}