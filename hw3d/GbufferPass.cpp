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
		//RegisterSource(DirectBindableSource<ShaderInputDepthStencil>::Make("depthOut", depthStencilRT));
	}
	void GbufferPass::BindMainCamera(const Camera& cam) noexcept
	{
		pMainCamera = &cam;
	}
	void GbufferPass::Execute(Graphics& gfx) const noxnd
	{
		assert(pMainCamera);
		pMainCamera->BindToGraphics(gfx);
		renderTarget->Clear(gfx);
		//depthStencil->Clear(gfx);
		RenderQueuePass::Execute(gfx);
	}
}