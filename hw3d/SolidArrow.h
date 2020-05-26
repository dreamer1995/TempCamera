#pragma once
#include "Drawable.h"

class SolidArrow : public Drawable
{
public:
	SolidArrow(Graphics& gfx, float size);
	void SetTransform(DirectX::XMFLOAT3 pos, float pitch, float yaw, float roll, float length) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void ChangeArrowColor() noexcept;
private:
	DirectX::XMFLOAT3 pos;
	float pitch;
	float yaw;
	float roll;
	float length;
};