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
	class DeferredBloomBlurPass : public FullscreenPass
	{
	public:
		DeferredBloomBlurPass(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight, int& bloomQuality)
			:
			FullscreenPass(std::move(name), gfx),
			bloomQuality(&bloomQuality)
		{
			using namespace Bind;
			RegisterSink(DirectBindableSink<ShaderInputRenderTarget>::Make("scratchIn", tempRT0));
			AddBindSink<CachingPixelConstantBufferEx>("kernel");
			tempRT1 = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 0u,
				RenderTarget::Type::Default, 0b1u, DXGI_FORMAT_R32G32B32A32_FLOAT);
			finalRT = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 0u,
				RenderTarget::Type::Default, 0b1u, DXGI_FORMAT_R32G32B32A32_FLOAT);
			renderTarget = finalRT;
			AddBind(PixelShader::Resolve(gfx, "BloomBlur.cso"));
			AddBind(Blender::Resolve(gfx, false));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));

			Dcb::RawLayout lay;
			lay.Add<Dcb::Bool>("isHorizontal");
			auto buf = Dcb::Buffer(std::move(lay));
			direction = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 11u);
			AddBind(direction);

			RegisterSource(DirectBindableSource<RenderTarget>::Make("scratchOut", renderTarget));
		}
		// this override is necessary because we cannot (yet) link input bindables directly into
		// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			for (unsigned char i = 0; i < *bloomQuality; i++)
			{
				gfx.ClearShaderResources(0u);
				gfx.ClearRenderTarget();
				tempRT0->Bind(gfx);
				const_cast<DeferredBloomBlurPass*>(this)->renderTarget = tempRT1;
				auto buf = direction->GetBuffer();
				buf["isHorizontal"] = true;
				direction->SetBuffer(buf);

				FullscreenPass::Execute(gfx);

				if (i != *bloomQuality - 1)
				{
					const_cast<DeferredBloomBlurPass*>(this)->renderTarget = tempRT0;
					tempRT0->BanToBind();
					gfx.ClearRenderTarget();
					tempRT1->Bind(gfx);
					buf["isHorizontal"] = false;
					direction->SetBuffer(buf);

					FullscreenPass::Execute(gfx);

					tempRT0->ReleaseToBind();
				}
				else
				{
					const_cast<DeferredBloomBlurPass*>(this)->renderTarget = finalRT;
					gfx.ClearRenderTarget();
					tempRT1->Bind(gfx);
					buf["isHorizontal"] = false;
					direction->SetBuffer(buf);

					FullscreenPass::Execute(gfx);
				}
			}
		}
	private:
		std::shared_ptr<Bind::ShaderInputRenderTarget> tempRT0;
		std::shared_ptr<Bind::ShaderInputRenderTarget> tempRT1;
		std::shared_ptr<Bind::ShaderInputRenderTarget> finalRT;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> direction;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> bloomParams;
		int* bloomQuality;
	};
}