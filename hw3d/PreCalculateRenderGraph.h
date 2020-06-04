#pragma once
#include "RenderGraph.h"
#include <memory>
#include "ConstantBuffersEx.h"

class Graphics;
class Camera;
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
		void BindMainCamera(Camera& cam);
	};
}
