#include "ShadowCameraCBuf.h"
#include "Camera.h"
#include "PointLight.h"

namespace dx = DirectX;

namespace Bind
{
	ShadowCameraCBuf::ShadowCameraCBuf(Graphics& gfx, UINT slot, UINT shaderIndex)
		:
		shaderIndex(shaderIndex)
	{
		assert(shaderIndex & 0b00111111);
		if (shaderIndex & 0b00010000)
		{
			pVcbuf = std::make_unique<VertexConstantBuffer<Transform>>(gfx, slot);
		}
		if (shaderIndex & 0b00001000)
		{
			pHcbuf = std::make_unique<HullConstantBuffer<Transform>>(gfx, slot);
		}
		if (shaderIndex & 0b00000100)
		{
			pDcbuf = std::make_unique<DomainConstantBuffer<Transform>>(gfx, slot);
		}
		if (shaderIndex & 0b00000010)
		{
			pGcbuf = std::make_unique<GeometryConstantBuffer<Transform>>(gfx, slot);
		}
		if (shaderIndex & 0b00000001)
		{
			pPcbuf = std::make_unique<PixelConstantBuffer<Transform>>(gfx, slot);
		}
		if (shaderIndex & 0b00100000)
		{
			pCcbuf = std::make_unique<ComputeConstantBuffer<Transform>>(gfx, slot);
		}
	}
	void ShadowCameraCBuf::Bind( Graphics& gfx ) noxnd
	{
		if (shaderIndex & 0b00010000)
		{
			pVcbuf->Bind(gfx);
		}
		if (shaderIndex & 0b00001000)
		{
			pHcbuf->Bind(gfx);
		}
		if (shaderIndex & 0b00000100)
		{
			pDcbuf->Bind(gfx);
		}
		if (shaderIndex & 0b00000010)
		{
			pGcbuf->Bind(gfx);
		}
		if (shaderIndex & 0b00000001)
		{
			pPcbuf->Bind(gfx);
		}
		if (shaderIndex & 0b00100000)
		{
			pCcbuf->Bind(gfx);
		}
	}
	void ShadowCameraCBuf::Update( Graphics& gfx )
	{
		const Transform t{
			dx::XMMatrixTranspose( 
				pCamera->GetMatrix() * pCamera->GetProjection()
			)
		};
		if (shaderIndex & 0b00010000)
		{
			pVcbuf->Update(gfx, t);
		}
		if (shaderIndex & 0b00001000)
		{
			pHcbuf->Update(gfx, t);
		}
		if (shaderIndex & 0b00000100)
		{
			pDcbuf->Update(gfx, t);
		}
		if (shaderIndex & 0b00000010)
		{
			pGcbuf->Update(gfx, t);
		}
		if (shaderIndex & 0b00000001)
		{
			pPcbuf->Update(gfx, t);
		}
		if (shaderIndex & 0b00100000)
		{
			pCcbuf->Update(gfx, t);
		}
	}
	void ShadowCameraCBuf::SetCamera( const Camera* p ) noexcept
	{
		pCamera = p;
	}
	void ShadowCameraCBuf::SetPointLight(std::shared_ptr<PointLight> light) noexcept
	{
		this->light = light;
	}
	void ShadowCameraCBuf::UpdatePointLight(Graphics& gfx)
	{
		auto pos = light->GetPos();
		const Transform t{
			dx::XMMatrixTranspose(
				dx::XMMatrixTranslation(-pos.x,-pos.y,-pos.z)
			)
		};
		if (shaderIndex & 0b00010000)
		{
			pVcbuf->Update(gfx, t);
		}
		if (shaderIndex & 0b00001000)
		{
			pHcbuf->Update(gfx, t);
		}
		if (shaderIndex & 0b00000100)
		{
			pDcbuf->Update(gfx, t);
		}
		if (shaderIndex & 0b00000010)
		{
			pGcbuf->Update(gfx, t);
		}
		if (shaderIndex & 0b00000001)
		{
			pPcbuf->Update(gfx, t);
		}
		if (shaderIndex & 0b00100000)
		{
			pCcbuf->Update(gfx, t);
		}
	}
}
