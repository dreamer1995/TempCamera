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
	class GbufferPass : public RenderQueuePass
	{
	public:
		GbufferPass(Graphics& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
			:
			RenderQueuePass(std::move(name))
		{
			using namespace Bind;
			renderTarget = std::make_unique<ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 0, RenderTarget::Type::GBuffer);
			RegisterSink(DirectBufferSink<DepthStencil>::Make("depthStencil", depthStencil));
			AddBind(VertexShader::Resolve(gfx, "Solid_VS.cso"));
			AddBind(PixelShader::Resolve(gfx, "Solid_PS.cso"));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::Off));
			AddBind(Blender::Resolve(gfx, false));
			RegisterSource(DirectBindableSource<Bind::RenderTarget>::Make("scratchOut", renderTarget));
			RegisterSource(DirectBufferSource<DepthStencil>::Make("depthStencil", depthStencil));
		}
		void BindMainCamera(const Camera& cam) noexcept
		{
			pMainCamera = &cam;
		}
		void Execute(Graphics& gfx) const noxnd override
		{
			assert(pMainCamera);
			pMainCamera->BindToGraphics(gfx);
			renderTarget->Clear(gfx);
			RenderQueuePass::Execute(gfx);
		}
	};
}