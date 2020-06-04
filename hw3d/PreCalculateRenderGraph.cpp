#include "PreCalculateRenderGraph.h"
#include "BufferClearPass.h"


namespace Rgph
{
	PreCalculateRenderGraph::PreCalculateRenderGraph(Graphics& gfx)
		:
		RenderGraph(gfx,false)
	{

		{
			auto pass = std::make_unique<BufferClearPass>("clearRT");
			pass->SetSinkLinkage("buffer", "$.backbuffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<BufferClearPass>("clearDS");
			pass->SetSinkLinkage("buffer", "$.masterDepth");
			AppendPass(std::move(pass));
		}
		SetSinkTarget("backbuffer", "clearRT.buffer");
		Finalize();
	}
}
