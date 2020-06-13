#pragma once
#include "RenderGraph.h"
#include <memory>
#include "ConstantBuffersEx.h"

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
		std::shared_ptr<Bind::RenderTarget> pPreCalSimpleCube;
		std::shared_ptr<Bind::RenderTarget> pPreCalBlurCube;
		std::shared_ptr<Bind::RenderTarget> pPreCalMipCube;
		std::shared_ptr<Bind::RenderTarget> pPreCalLUTPlane;
	};
}
