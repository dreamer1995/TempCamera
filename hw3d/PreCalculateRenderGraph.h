#pragma once
#include "RenderGraph.h"
#include <memory>
#include "ConstantBuffersEx.h"
#include "PreCalBlurCube.h"
#include "PreCalMipCube.h"
#include "PreCalSimpleCube.h"

class Graphics;
namespace Bind
{
	class Bindable;
	class RenderTarget;
}

namespace Rgph
{
	class PreCalculateRenderGraph : public RenderGraph
	{
	public:
		PreCalculateRenderGraph(Graphics& gfx);
	public:
		std::unique_ptr<PreCalSimpleCube> pPreCalSimpleCube;
		std::unique_ptr<PreCalBlurCube> pPreCalBlurCube;
		std::unique_ptr<PreCalMipCube> pPreCalMipCube;
	};
}
