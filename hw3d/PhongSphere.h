#pragma once
#include "Drawable.h"

class PhongSphere : public Drawable
{
public:
	PhongSphere(Graphics& gfx, float radius);
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void ChangeSphereMaterialState() noexcept;
private:
	DirectX::XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
};