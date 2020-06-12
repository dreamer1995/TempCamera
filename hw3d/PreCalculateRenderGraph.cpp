#include "PreCalculateRenderGraph.h"

namespace Rgph
{
	PreCalculateRenderGraph::PreCalculateRenderGraph(Graphics& gfx)
		:
		RenderGraph(gfx, RenderGraph::Type::PreCal)
	{
		gfx.SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 1.0f, 0.5f, 400.0f));
		{
			pPreCalSimpleCube = std::make_unique<PreCalSimpleCube>("preCalSimpleCube", gfx, gfx.GetWidth(), gfx.GetHeight());
			AppendPass(std::move(pPreCalSimpleCube));
		}
		{
			pPreCalBlurCube = std::make_unique<PreCalBlurCube>("preCalBlurCube", gfx, 64u, 64u);
			pPreCalBlurCube->SetSinkLinkage("HDIn", "preCalSimpleCube.HDOut");
			AppendPass(std::move(pPreCalBlurCube));
		}
		{
			pPreCalMipCube = std::make_unique<PreCalMipCube>("preCalMipCube", gfx, 256u, 256u);
			pPreCalMipCube->SetSinkLinkage("HDIn", "preCalSimpleCube.HDOut");
			AppendPass(std::move(pPreCalMipCube));
		}
		//{
		//	auto pass = std::make_unique<BufferClearPass>("clearRT3");
		//	pass->SetSinkLinkage("buffer", "$.backbuffer");
		//	AppendPass(std::move(pass));
		//}
		//SetSinkTarget("backbuffer", "clearRT.buffer");
		Finalize();
	}
}
