#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include <vector>
#include "Sink.h"
#include "Source.h"
#include "Stencil.h"
#include "Camera.h"
#include "DepthStencil.h"

class Graphics;

namespace Rgph
{
	class WaterCaustics : public RenderQueuePass
	{
	public:
		WaterCaustics(Graphics& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
			:
			RenderQueuePass(std::move(name))
		{
			using namespace Bind;
			renderTarget = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 3u);
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::Off));
			RegisterSource(DirectBindableSource<Bind::RenderTarget>::Make("waterCausticOut", renderTarget));
		}
		//void BindMainCamera(const Camera& cam) noexcept
		//{
		//	pMainCamera = &cam;
		//}
		void Execute(Graphics& gfx) const noxnd override
		{
			renderTarget->Clear(gfx);
			//pMainCamera->BindToGraphics(gfx);
			RenderQueuePass::Execute(gfx);
		}
	private:
		//const Camera* pMainCamera = nullptr;
	};
}