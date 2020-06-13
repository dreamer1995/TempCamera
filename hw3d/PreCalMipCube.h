#pragma once
#include "PreCubeCalculatePass.h"
#include "ConstantBuffersEx.h"

namespace dx = DirectX;

class Graphics;
namespace Bind
{
	class PixelShader;
	class RenderTarget;
}

namespace Rgph
{
	class PreCalMipCube : public PreCubeCalculatePass
	{
	public:
		PreCalMipCube(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight);
		void Execute(Graphics& gfx) const noxnd override;
	public:
		std::shared_ptr<Bind::ShaderInputRenderTarget> pPreCalMipCube;
		dx::XMMATRIX viewmatrix[6] =
		{
			dx::XMMatrixLookAtLH({ 0.0f,0.0f,0.0f }, { 1.0f,0.0f,0.0f }, { 0.0f,1.0f,0.0f }),
			dx::XMMatrixLookAtLH({ 0.0f,0.0f,0.0f }, { -1.0f,0.0f,0.0f }, { 0.0f,1.0f,0.0f }),
			dx::XMMatrixLookAtLH({ 0.0f,0.0f,0.0f }, { 0.0f,1.0f,0.0f }, { 0.0f,0.0f,-1.0f }),
			dx::XMMatrixLookAtLH({ 0.0f,0.0f,0.0f }, { 0.0f,-1.0f,0.0f }, { 0.0f,0.0f,1.0f }),
			dx::XMMatrixLookAtLH({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,1.0f }, { 0.0f,1.0f,0.0f }),
			dx::XMMatrixLookAtLH({ 0.0f,0.0f,0.0f }, { 0.0f,0.0f,-1.0f }, { 0.0f,1.0f,0.0f })
		};
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> roughness;
	};
}