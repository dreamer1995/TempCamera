#pragma once
#include "ConstantBuffers.h"
#include "Drawable.h"
#include <DirectXMath.h>

namespace Bind
{
	class TransformCbuf : public CloningBindable
	{
	protected:
		struct Transforms
		{
			DirectX::XMMATRIX matrix_MVP;
			DirectX::XMMATRIX matrix_MV;
			DirectX::XMMATRIX matrix_V;
			DirectX::XMMATRIX matrix_P;
			DirectX::XMMATRIX matrix_VP;
			DirectX::XMMATRIX matrix_T_MV;
			DirectX::XMMATRIX matrix_IT_MV;
			DirectX::XMMATRIX matrix_M2W;
			DirectX::XMMATRIX matrix_W2M;
		};
	public:
		TransformCbuf(Graphics& gfx, UINT otherShaderIndex = 0b0u);
		void Bind( Graphics& gfx ) noxnd override;
		void InitializeParentReference( const Drawable& parent ) noexcept override;
		std::unique_ptr<CloningBindable> Clone() const noexcept override;
	protected:
		void UpdateBindImpl( Graphics& gfx,const Transforms& tf ) noxnd;
		Transforms GetTransforms( Graphics& gfx ) noxnd;
		UINT otherShaderIndex;
	private:
		static std::unique_ptr<VertexConstantBuffer<Transforms>> pVcbuf;
		static std::unique_ptr<PixelConstantBuffer<Transforms>> pPcbuf;
		const Drawable* pParent = nullptr;
		static std::unique_ptr<DomainConstantBuffer<Transforms>> pDcbuf;
	};
}