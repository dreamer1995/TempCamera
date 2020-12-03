#include "PreCalculateRenderGraph.h"
#include "PreCalBlurCube.h"
#include "PreCalMipCube.h"
#include "PreCalSimpleCube.h"
#include "PreCalLUTPlane.h"

namespace Rgph
{
	PreCalculateRenderGraph::PreCalculateRenderGraph(Graphics& gfx)
		:
		RenderGraph(gfx, RenderGraph::Type::PreCal)
	{
		gfx.SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 1.0f, 0.5f, 400.0f));
		{
			auto pass = std::make_unique<PreCalSimpleCube>("preCalSimpleCube", gfx, gfx.GetWidth(), gfx.GetWidth());
			pPreCalSimpleCube = pass->pPreCalSimpleCube;
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<PreCalBlurCube>("preCalBlurCube", gfx, 64u, 64u);
			pass->SetSinkLinkage("HDIn", "preCalSimpleCube.HDOut");
			pPreCalBlurCube = pass->pPreCalBlurCube;
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<PreCalMipCube>("preCalMipCube", gfx, 256u, 256u);
			pass->SetSinkLinkage("HDIn", "preCalSimpleCube.HDOut");
			pPreCalMipCube = pass->pPreCalMipCube;
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<PreCalLUTPlane>("preCalLUTPlane", gfx, gfx.GetWidth(), gfx.GetHeight());
			pPreCalLUTPlane = pass->pPreCalLUTPlane;
			AppendPass(std::move(pass));
		}
		Finalize();
		Execute(gfx);
		dynamic_cast<PreCalSimpleCube&>(FindPassByName("preCalSimpleCube")).DumpShadowMap(gfx, "Images\\Env\\Test.jpg");
	}
}
