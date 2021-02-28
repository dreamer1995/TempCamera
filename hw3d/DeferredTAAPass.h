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
	class DeferredTAAPass : public FullscreenPass
	{
	public:
		DeferredTAAPass(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			FullscreenPass(std::move(name), gfx),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			pPcbuf = std::make_unique<Bind::PixelConstantBuffer<Transforms>>(gfx, 10u);
			pPcbuf->Update(gfx, { DirectX::XMMatrixTranspose(gfx.GetProjection() * gfx.GetCamera()) });
			AddBindSink<RenderTarget>("scratchIn");
			AddBind(masterDepth);
			currentRT = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 1u,
				RenderTarget::Type::Default, 0b1u, DXGI_FORMAT_R32G32B32A32_FLOAT);
			historyRT = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 1u,
				RenderTarget::Type::Default, 0b1u, DXGI_FORMAT_R32G32B32A32_FLOAT);
			//currentRT->Clear(gfx, { 1,0,0,0 });
			//historyRT->Clear(gfx, { 0,1,0,0 });
			renderTarget = currentRT;
			AddBind(PixelShader::Resolve(gfx, "TAA.cso"));
			AddBind(Blender::Resolve(gfx, false));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			RegisterSource(DirectBindableSource<ShaderInputRenderTarget>::Make("renderTarget0", currentRT));
			RegisterSource(DirectBindableSource<ShaderInputRenderTarget>::Make("renderTarget1", historyRT));
		}
		void BindMainCamera(const Camera& cam) noexcept
		{
			pMainCamera = &cam;
		}
		// this override is necessary because we cannot (yet) link input bindables directly into
		// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			masterDepth->BreakRule();
			currentRT->BanToBind();
			pPcbuf->Bind(gfx);
			historyRT->Bind(gfx);
			FullscreenPass::Execute(gfx);
			historyRT->BanToBind();
			currentRT->ReleaseToBind();
			const_cast<DeferredTAAPass*>(this)->currentRT.swap(const_cast<DeferredTAAPass*>(this)->historyRT);
			const_cast<DeferredTAAPass*>(this)->renderTarget = currentRT;
			pPcbuf->Update(gfx, { DirectX::XMMatrixTranspose(gfx.GetCamera() * gfx.GetProjection()) });

			gfx.ClearShaderResources(8u);
		}
	private:
		std::shared_ptr<Bind::ShaderInputRenderTarget> currentRT;
		std::shared_ptr<Bind::ShaderInputRenderTarget> historyRT;
		const Camera* pMainCamera = nullptr;
		struct Transforms
		{
			DirectX::XMMATRIX matrix_MVP_Last;
		};
		std::unique_ptr<Bind::PixelConstantBuffer<Transforms>> pPcbuf;
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
	};
}