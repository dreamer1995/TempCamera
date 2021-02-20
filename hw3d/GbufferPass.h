#pragma once
#include "RenderQueuePass.h"
#include "Camera.h"
#include "ConstantBuffersEx.h"

class Graphics;

namespace Rgph
{
	class GbufferPass : public RenderQueuePass
	{
	public:
		GbufferPass(Graphics& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight);
		void Execute(Graphics& gfx) const noxnd override;
		void BindMainCamera(Camera& cam) noexcept;
	private:
		Camera* pMainCamera = nullptr;
		//std::shared_ptr<Bind::ShaderInputDepthStencil> depthStencilRT;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> TAAIndex;
	};
}