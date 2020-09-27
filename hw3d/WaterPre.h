#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include <vector>
#include "Sink.h"
#include "Source.h"
#include "Camera.h"
#include "ConstantBuffers.h"
#include "BindableCommon.h"
#include "Plane.h"

using namespace Bind;

class Graphics;

namespace Rgph
{
	class WaterPrePass : public RenderQueuePass
	{
	public:
		WaterPrePass(Graphics& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
			:
			RenderQueuePass(std::move(name))
		{
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::Off));
			AddBind(Bind::Rasterizer::Resolve(gfx, false));

			AddBindSink<Bind::CachingVertexConstantBufferEx>("waterFlow");
			AddBindSink<Bind::CachingPixelConstantBufferEx>("waterRipple");
			renderTarget = std::make_shared<ShaderInputRenderTarget>(gfx, fullWidth, fullWidth, 3u);
			RegisterSource(DirectBindableSource<RenderTarget>::Make("waterPreOut", renderTarget));
		}

		void Execute(Graphics& gfx) const noxnd override
		{
			RenderQueuePass::Execute(gfx);
		}
	protected:
		struct Transforms
		{
			DirectX::XMMATRIX matrix_MVP;
		};
	};
}