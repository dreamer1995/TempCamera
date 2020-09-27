#pragma once
#include "Drawable.h"
#include "Bindable.h"
#include "IndexBuffer.h"
#include "ConstantBuffersEx.h"
#include "ChiliMath.h"

class PlaneWaterNormalBake : public Drawable
{
public:
	PlaneWaterNormalBake(Graphics& gfx, float size);
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControlWindow(Graphics& gfx, const char* name) noexcept;
private:
	std::shared_ptr<Bind::CachingPixelConstantBufferEx> cBuf;
};