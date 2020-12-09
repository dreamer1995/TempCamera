#include "PreCalculateRenderGraph.h"
#include "PreCalBlurCube.h"
#include "PreCalMipCube.h"
#include "PreCalSimpleCube.h"
#include "PreCalLUTPlane.h"

namespace Rgph
{
	PreCalculateRenderGraph::PreCalculateRenderGraph(Graphics& gfx, const std::filesystem::path& path, unsigned char checkCode)
		:
		RenderGraph(gfx, RenderGraph::Type::PreCal)
	{
		if (checkCode > 0b1u)
		{
			gfx.SetProjection(dx::XMMatrixPerspectiveLH(1.0f, 1.0f, 0.5f, 400.0f));
			auto pass = std::make_unique<PreCalSimpleCube>("preCalSimpleCube", gfx, gfx.GetWidth(), gfx.GetWidth(), path.string());
			pPreCalSimpleCube = pass->pPreCalSimpleCube;
			AppendPass(std::move(pass));
		}
		if (checkCode & 0b100u)
		{
			auto pass = std::make_unique<PreCalBlurCube>("preCalBlurCube", gfx, 64u, 64u);
			pass->SetSinkLinkage("HDIn", "preCalSimpleCube.HDOut");
			pPreCalBlurCube = pass->pPreCalBlurCube;
			AppendPass(std::move(pass));
		}
		if (checkCode & 0b1000u)
		{
			auto pass = std::make_unique<PreCalMipCube>("preCalMipCube", gfx, 256u, 256u);
			pass->SetSinkLinkage("HDIn", "preCalSimpleCube.HDOut");
			pPreCalMipCube = pass->pPreCalMipCube;
			AppendPass(std::move(pass));
		}
		if(checkCode & 0b1u)
		{
			auto pass = std::make_unique<PreCalLUTPlane>("preCalLUTPlane", gfx, gfx.GetWidth(), gfx.GetHeight());
			pPreCalLUTPlane = pass->pPreCalLUTPlane;
			AppendPass(std::move(pass));
		}
		Finalize();
		Execute(gfx);
		if (checkCode & 0b10u)
			dynamic_cast<PreCalSimpleCube&>(FindPassByName("preCalSimpleCube")).DumpCubeMap(gfx,
				"Images\\PreCal\\" + path.stem().string() + "#0" + path.extension().string());
		if (checkCode & 0b100u)
			dynamic_cast<PreCalBlurCube&>(FindPassByName("preCalBlurCube")).DumpCubeMap(gfx,
				"Images\\PreCal\\" + path.stem().string() + "#1" + path.extension().string());
		if (checkCode & 0b1000u)
			dynamic_cast<PreCalMipCube&>(FindPassByName("preCalMipCube")).DumpCubeMap(gfx,
				"Images\\PreCal\\" + path.stem().string() + "#2" + path.extension().string());
		if (checkCode & 0b1u)
			dynamic_cast<PreCalLUTPlane&>(FindPassByName("preCalLUTPlane")).DumpLUTMap(gfx,
				"Images\\PreCal\\IBLBRDFLUT.png");
	}

	using namespace std::filesystem;
	unsigned char PreCalculateRenderGraph::CheckPreMapAvailability(const path& path)
	{
		unsigned char checkCode = 0b0u;
		for (unsigned char i = 0; i < 6; i++)
			if (!exists("Images\\PreCal\\" + path.stem().string() + "#0#" + std::to_string(i) + path.extension().string()))
			{
				checkCode |= 0b10u;
				break;
			}

		for (unsigned char i = 0; i < 6; i++)
			if (!exists("Images\\PreCal\\" + path.stem().string() + "#1#" + std::to_string(i) + path.extension().string()))
			{
				checkCode |= 0b100u;
				break;
			}

		for (unsigned char i = 0; i < 6; i++)
		{
			for (unsigned char j = 0; j < 5; j++)
				if (!exists("Images\\PreCal\\" + path.stem().string() + "#2#" + std::to_string(i) + "#" + std::to_string(j) + path.extension().string()))
				{
					checkCode |= 0b1000u;
					break;
				}
			if (checkCode & 0b1000u)
				break;
		}

		if (!exists("Images\\PreCal\\IBLBRDFLUT.png"))
			checkCode |= 0b1u;
		return checkCode;
	}
}