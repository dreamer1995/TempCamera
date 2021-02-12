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
		DeferredSunLightPass(std::string name, Graphics& gfx, std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth)
			:
			FullscreenPass(std::move(name), gfx),
			masterDepth(masterDepth)
		{
			using namespace Bind;
			pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, 13u);
			AddBind(PixelShader::Resolve(gfx, "DeferredSunLight.cso"));
			AddBind(Blender::Resolve(gfx, true));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 0u));
			AddBindSink<Bindable>("gbufferIn");
			AddBind(masterDepth);
			RegisterSink(DirectBufferSink<RenderTarget>::Make("renderTarget", renderTarget));

			RegisterSource(DirectBufferSource<RenderTarget>::Make("renderTarget", renderTarget));
		}
		void BindMainCamera(const Camera& cam) noexcept
		{
			pMainCamera = &cam;
		}
		// this override is necessary because we cannot (yet) link input bindables directly into
	// the container of bindables (mainly because vector growth buggers references)
		void Execute(Graphics& gfx) const noxnd override
		{
			assert(pMainCamera);
			pMainCamera->BindToGraphics(gfx);
			masterDepth->BreakRule();
			pVcbuf->Update(gfx, { DirectX::XMMatrixTranspose(DirectX::XMMatrixIdentity() * gfx.GetCamera()) });
			pVcbuf->Bind(gfx);
			FullscreenPass::Execute(gfx);
			gfx.ClearShaderResources(8u);
		}

	private:
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
		const Camera* pMainCamera = nullptr;
		struct Transforms
		{
			DirectX::XMMATRIX matrix_I_P;
		};
		std::unique_ptr<Bind::VertexConstantBuffer<Transforms>> pVcbuf;
	};
}