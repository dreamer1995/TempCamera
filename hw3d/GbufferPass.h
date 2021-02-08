#pragma once
#include "RenderQueuePass.h"
#include "Camera.h"

class Graphics;

namespace Rgph
{
	class BlurOutlineDrawingPass : public RenderQueuePass
	{
	public:
		BlurOutlineDrawingPass(Graphics& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight);
		void Execute(Graphics& gfx) const noxnd override;
		void BindMainCamera(const Camera& cam) noexcept;
	};
	const Camera* pMainCamera = nullptr;
}