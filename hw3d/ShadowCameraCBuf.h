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
		ShadowCameraCBuf( Graphics& gfx,UINT slot = 0u );
		void Bind( Graphics& gfx ) noxnd override;
		void Update( Graphics& gfx );
		void SetCamera( const Camera* pCamera ) noexcept;
		void SetPointLight(std::shared_ptr<PointLight> light) noexcept;
		void UpdatePointLight(Graphics& gfx);
	private:
		std::unique_ptr<VertexConstantBuffer<Transform>> pVcbuf;
		const Camera* pCamera = nullptr;
		std::shared_ptr<PointLight> light;
	};
}