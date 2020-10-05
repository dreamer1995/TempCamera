#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include <vector>
#include "Sink.h"
#include "Source.h"
#include "BindableCommon.h"

class Graphics;

namespace Rgph
{
	class WaterCaustics : public RenderQueuePass
	{
	public:
		WaterCaustics(Graphics& gfx, std::string name, unsigned int fullWidth)
			:
			RenderQueuePass(std::move(name))
		{
			using namespace Bind;
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::Off));
			AddBind(Bind::Rasterizer::Resolve(gfx, true));
			AddBind(Blender::Resolve(gfx, true, Blender::BlendMode::Additive));
			AddBindSink<Bind::Bindable>("waterPreMap");
			AddBindSink<Bind::CachingDomainConstantBufferEx>("waterFlow");
			renderTarget = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullWidth, 5u);
			RegisterSource(DirectBindableSource<Bind::RenderTarget>::Make("waterCausticOut", renderTarget));
		}
		void Execute(Graphics& gfx) const noxnd override
		{
			renderTarget->Clear(gfx);
			RenderQueuePass::Execute(gfx);
			gfx.UnbindTessellationShaders();
		}
	};
}