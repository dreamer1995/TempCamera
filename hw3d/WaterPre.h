#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include <vector>
#include "Sink.h"
#include "Source.h"
#include "BindableCommon.h"

using namespace Bind;

class Graphics;

namespace Rgph
{
	class WaterPrePass : public RenderQueuePass
	{
	public:
		WaterPrePass(Graphics& gfx, std::string name, unsigned int fullWidth)
			:
			RenderQueuePass(std::move(name))
		{
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::Off));
			AddBind(Bind::Rasterizer::Resolve(gfx, false));
			AddBind(Blender::Resolve(gfx, false));

			AddBindSink<Bind::CachingVertexConstantBufferEx>("waterFlow");
			AddBindSink<Bind::CachingPixelConstantBufferEx>("waterRipple");
			renderTarget = std::make_shared<ShaderInputRenderTarget>(gfx, fullWidth, fullWidth, 4u, RenderTarget::Type::Default, 0b0100u);
			RegisterSource(DirectBindableSource<RenderTarget>::Make("waterPreOut", renderTarget));
		}

		void Execute(Graphics& gfx) const noxnd override
		{
			// renderTarget->Clear(gfx);
			RenderQueuePass::Execute(gfx);
		}
	};
}