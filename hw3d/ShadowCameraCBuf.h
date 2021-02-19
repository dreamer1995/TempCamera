#pragma once
#include "Bindable.h"
#include "ConstantBuffers.h"
#include "PointLight.h"

class Camera;

namespace Bind
{
	class ShadowCameraCBuf : public Bindable
	{
	protected:
		struct Transform
		{
			DirectX::XMMATRIX ViewProj;
		};
	public:
		ShadowCameraCBuf(Graphics& gfx, UINT slot = 0u, UINT shaderIndex = 0b1000u);
		void Bind( Graphics& gfx ) noxnd override;
		void Update( Graphics& gfx );
		void SetCamera( const Camera* pCamera ) noexcept;
		void SetPointLight(std::shared_ptr<PointLight> light) noexcept;
		void UpdatePointLight(Graphics& gfx);
	private:
		std::unique_ptr<VertexConstantBuffer<Transform>> pVcbuf;
		std::unique_ptr<HullConstantBuffer<Transform>> pHcbuf;
		std::unique_ptr<DomainConstantBuffer<Transform>> pDcbuf;
		std::unique_ptr<PixelConstantBuffer<Transform>> pPcbuf;
		const Camera* pCamera = nullptr;
		std::shared_ptr<PointLight> light;
		UINT shaderIndex;
	};
}