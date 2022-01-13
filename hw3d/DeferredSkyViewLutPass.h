#pragma once
#include "FullscreenPass.h"
#include "Sink.h"
#include "Source.h"
#include "PixelShader.h"

class Graphics;
namespace Bind
{
	class PixelShader;
	class RenderTarget;
}

namespace Rgph
{
	class DeferredSkyViewLutPass : public FullscreenPass
	{
	public:
		DeferredSkyViewLutPass(std::string name, Graphics& gfx)
			:
			FullscreenPass(std::move(name), gfx)
		{
			using namespace Bind;
			//AddBindSink<Bind::CachingPixelConstantBufferEx>("volumeParams");
			renderTarget = std::make_shared<ShaderInputRenderTarget>(gfx, 256u, 64u, 0u, RenderTarget::Type::Default,
				0b100001u, DXGI_FORMAT_R16G16B16A16_FLOAT);
			AddBindSink<CachingPixelConstantBufferEx>("skyConstants");
			AddBind(PixelShader::Resolve(gfx, "TransmittanceLUT.cso"));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
			AddBind(Blender::Resolve(gfx, false));
			RegisterSource(DirectBindableSource<RenderTarget>::Make("scratchOut", renderTarget));
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
			gfx.ClearRenderTarget();
			gfx.ClearShaderResources(0u);
			BindAll(gfx);
			gfx.DrawIndexed(6u);
		}

	private:
		const Camera* pMainCamera = nullptr;
	};
}