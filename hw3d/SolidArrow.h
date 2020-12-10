#pragma once
#include "Drawable.h"
#include "ConstantBuffersEx.h"

class SolidArrow : public Drawable
{
public:
	SolidArrow(Graphics& gfx, float size);
	void SetTransform(DirectX::XMFLOAT3 pos, float pitch, float yaw) noexcept;
	void SetColor(DirectX::XMFLOAT3 diffuseColor) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void ChangeArrowColor() noexcept;
private:
	DirectX::XMFLOAT3 pos;
	float pitch;
	float yaw;
public:
	std::shared_ptr<Bind::CachingPixelConstantBufferEx> cbuf;
};