#include "PreCalculateRenderGraph.h"
#include "BufferClearPass.h"
#include "PreCalBlurCube.h"
#include "PreCalMipCube.h"
#include "PreCalSimpleCube.h"


namespace Rgph
{
	PreCalculateRenderGraph::PreCalculateRenderGraph(Graphics& gfx)
		:
		RenderGraph(gfx, RenderGraph::Type::PreCal)
	{
		{
			auto pass = std::make_unique<PreCalSimpleCube>("preCalSimpleCube", gfx, gfx.GetWidth(), gfx.GetHeight());
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<PreCalBlurCube>("preCalBlurCube", gfx, 64u, 64u);
			pass->SetSinkLinkage("HDIn", "preCalSimpleCube.HDOut");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<PreCalMipCube>("preCalBlurCube", gfx, 256u, 256u);
			pass->SetSinkLinkage("HDIn", "preCalSimpleCube.HDOut");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<BufferClearPass>("clearRT3");
			pass->SetSinkLinkage("buffer", "$.backbuffer");
			AppendPass(std::move(pass));
		}
		SetSinkTarget("backbuffer", "clearRT.buffer");
		Finalize();
	}
}
