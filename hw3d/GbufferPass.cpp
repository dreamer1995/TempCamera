#pragma once
#include "GbufferPass.h"
#include "Job.h"
#include <vector>
#include "PixelShader.h"
#include "VertexShader.h"
#include "Stencil.h"
#include "Rasterizer.h"
#include "Source.h"
#include "RenderTarget.h"
#include "Blender.h"
#include "WireframePass.h"

namespace Rgph
{
	GbufferPass::GbufferPass(Graphics& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
		:
		RenderQueuePass(std::move(name))
	{
		using namespace Bind;
		renderTarget = std::make_unique<ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 0u, RenderTarget::Type::GBuffer);
		//depthStencilRT = std::make_unique<ShaderInputDepthStencil>(gfx, fullWidth, fullHeight, 8u, DepthStencil::Usage::ShadowDepth);
		//depthStencil = depthStencilRT;
		RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));
		AddBind(Stencil::Resolve(gfx, Stencil::Mode::Off));
		AddBind(Blender::Resolve(gfx, false));
		RegisterSource(DirectBindableSource<RenderTarget>::Make("gbufferOut", renderTarget));
		RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
		RegisterSink(DirectBindableSink<CachingPixelConstantBufferEx>::Make("TAAIndex", TAAIndex));
		//RegisterSource(DirectBindableSource<ShaderInputDepthStencil>::Make("depthOut", depthStencilRT));
	}
	void GbufferPass::BindMainCamera(Camera& cam) noexcept
	{
		pMainCamera = &cam;
	}
	void GbufferPass::Execute(Graphics& gfx) const noxnd
	{
		assert(pMainCamera);

		if (gfx.isTAA)
		{
			int TAASamples = 8;
			static int mOffsetIdx = 0;
			mOffsetIdx = (mOffsetIdx + 1) & (TAASamples - 1);

			auto buf = TAAIndex->GetBuffer();
			buf["TAAIndex"] = mOffsetIdx;
			TAAIndex->SetBuffer(buf);
			TAAIndex->Bind(gfx);

			auto TemporalHalton = [](int Index, int Base)
			{
				float Result = 0.0f;
				float InvBase = 1.0f / Base;
				float Fraction = InvBase;
				while (Index > 0)
				{
					Result += (Index % Base) * Fraction;
					Index /= Base;
					Fraction *= InvBase;
				}
				return Result;
			};

			float u = TemporalHalton(mOffsetIdx + 1, 2);
			float v = TemporalHalton(mOffsetIdx + 1, 3);
			float s = 0.47f;
			float outWindow = 0.5f;
			float inWindow = exp(-0.5f * (outWindow / s) * (outWindow / s));

			float t = 2.0f * PI * v;
			float r = s * sqrt(-2.0f * log((1.0f - u) * inWindow + u));

			float x = r * cos(t);
			float y = r * sin(t);

			pMainCamera->SetOffsetPixels(x, y);
		}

		pMainCamera->BindToGraphics(gfx);
		renderTarget->Clear(gfx);
		//depthStencil->Clear(gfx);
		RenderQueuePass::Execute(gfx);
	}
}