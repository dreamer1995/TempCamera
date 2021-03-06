#include "TransformCbuf.h"

namespace Bind
{
	TransformCbuf::TransformCbuf(Graphics& gfx, UINT otherShaderIndex)
		:
		otherShaderIndex(otherShaderIndex)
	{
		if( !pVcbuf )
		{
			pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>( gfx,0 );
		}
		if (!pPcbuf)
		{
			pPcbuf = std::make_unique<PixelConstantBuffer<Transforms>>(gfx, 0);
		}
		if (!pDcbuf && otherShaderIndex & 0b00000001)
		{
			pDcbuf = std::make_unique<DomainConstantBuffer<Transforms>>(gfx, 0);
		}
	}

	void TransformCbuf::Bind( Graphics& gfx ) noxnd
	{
		INFOMAN_NOHR( gfx );
		GFX_THROW_INFO_ONLY( UpdateBindImpl( gfx,GetTransforms( gfx ) ) );
	}

	void TransformCbuf::InitializeParentReference( const Drawable& parent ) noexcept
	{
		pParent = &parent;
	}

	std::unique_ptr<CloningBindable> TransformCbuf::Clone() const noexcept
	{
		return std::make_unique<TransformCbuf>( *this );
	}

	void TransformCbuf::UpdateBindImpl( Graphics& gfx,const Transforms& tf ) noxnd
	{
		assert( pParent != nullptr );
		pVcbuf->Update( gfx,tf );
		pVcbuf->Bind( gfx );
		pPcbuf->Update(gfx, tf);
		pPcbuf->Bind(gfx);
		if (otherShaderIndex & 0b00000001)
		{
			pDcbuf->Update(gfx, tf);
			pDcbuf->Bind(gfx);
		}
	}

	TransformCbuf::Transforms TransformCbuf::GetTransforms( Graphics& gfx ) noxnd
	{
		assert( pParent != nullptr );
		const auto matrix_M2W = DirectX::XMMatrixTranspose(pParent->GetTransformXM());
		const auto matrix_W2M = DirectX::XMMatrixInverse(nullptr, matrix_M2W);
		const auto matrix_V = DirectX::XMMatrixTranspose(gfx.GetCamera());
		const auto matrix_MV = matrix_V * matrix_M2W;
		const auto matrix_P = DirectX::XMMatrixTranspose(gfx.GetProjection());
		const auto matrix_VP = matrix_P * matrix_V;
		const auto matrix_MVP = matrix_P * matrix_MV;
		const auto matrix_T_MV = pParent->GetTransformXM() * gfx.GetCamera();
		const auto matrix_IT_MV = DirectX::XMMatrixInverse(nullptr, matrix_T_MV);
		const auto matrix_I_V = DirectX::XMMatrixInverse(nullptr, matrix_V);
		const auto matrix_I_P = DirectX::XMMatrixInverse(nullptr, matrix_P);
		return {matrix_MVP,
				matrix_MV,
				matrix_V,
				matrix_P,
				matrix_VP,
				matrix_T_MV,
				matrix_IT_MV,
				matrix_M2W,
				matrix_W2M,
				matrix_I_V,
				matrix_I_P
		};
	}

	std::unique_ptr<VertexConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::pVcbuf;
	std::unique_ptr<PixelConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::pPcbuf;
	std::unique_ptr<DomainConstantBuffer<TransformCbuf::Transforms>> TransformCbuf::pDcbuf;
}