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
	class RenderTarget;
}

namespace Rgph
{
	class DeferredScatteringLutPass : public ComputeShaderPass
	{
	public:
		DeferredScatteringLutPass(std::string name, Graphics& gfx, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			ComputeShaderPass(std::move(name), gfx),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			AddBindSink<RenderTarget>("transmittanceLutIn");
			AddBindSink<CachingComputeConstantBufferEx>("skyConstants");
			//AddBindSink<Bind::CachingPixelConstantBufferEx>("volumeParams");
			width = 32u;
			height = 32u;
			unorderedAccessView = std::make_shared<ShaderInputUAV>(gfx, 32u, 32u, 1u, 0b100001u);
			pDShadowCBuf = std::make_shared<Bind::ShadowCameraCBuf>(gfx, 5u, 0b100000u);
			AddBind(pDShadowCBuf);
			AddBindSink<Bindable>("dShadowMap");
			AddBind(Texture::Resolve(gfx, "Images\\bluenoise.exr", 10u));
			AddBind(ComputeShader::Resolve(gfx, "ScatteringLUT.cso"));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u, 0b100000u));
			AddBind(masterDepth);
			AddBind(Blender::Resolve(gfx, false));
			AddBindSink<Bindable>("shadowControl");
			AddBindSink<Bindable>("shadowSampler");
			RegisterSource(DirectBindableSource<UnorderedAccessView>::Make("scratchOut", unorderedAccessView));
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
			gfx.ClearRenderTarget();

			ComputeShaderPass::SetDispatchVector(width, height, 1u);
			ComputeShaderPass::Execute(gfx);

			gfx.ClearShaderResources(8u);
			gfx.ClearUAV(0u);
		}
	private:
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
		std::shared_ptr<Bind::ShadowCameraCBuf> pDShadowCBuf;
		UINT width;
		UINT height;
	};
}