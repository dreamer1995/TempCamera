#pragma once
#include "Drawable.h"
#include "Bindable.h"
#include "IndexBuffer.h"
#include "ConstantBuffersEx.h"
#include "ChiliMath.h"
#include "PlaneCaustics.h"

class PlaneWater : public Drawable
{
public:
	PlaneWater(Graphics& gfx, float size);
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	void SetRotation(float roll, float pitch, float yaw) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControlWindow(Graphics& gfx, const char* name) noexcept;
	void SubmitEX(size_t channels1, size_t channels2) const;
	void LinkTechniquesEX(Rgph::RenderGraph& rg);
private:
	DirectX::XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };
	float roll = 0.0f;
	float pitch = PI / 2.0f;
	float yaw = 0.0f;
	PlaneCaustics waterCaustics;
	std::shared_ptr<Bind::CachingVertexConstantBufferEx> vmc;
};