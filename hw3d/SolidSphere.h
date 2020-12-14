#pragma once
#include "Drawable.h"
#include "ConstantBuffersEx.h"

class SolidSphere : public Drawable
{
public:
	SolidSphere( Graphics& gfx,float radius );
	void SetPos( DirectX::XMFLOAT3 pos ) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void ChangeSphereMaterialState() noexcept;
	void SetColor(DirectX::XMFLOAT3 diffuseColor) noexcept;
private:
	DirectX::XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
public:
	std::shared_ptr<Bind::CachingPixelConstantBufferEx> cbuf;
};