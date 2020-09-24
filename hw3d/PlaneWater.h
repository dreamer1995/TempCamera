#pragma once
#include "Drawable.h"
#include "Bindable.h"
#include "IndexBuffer.h"
#include "ConstantBuffersEx.h"
#include "PlaneWaterNormalBake.h"

class PlaneWater : public Drawable
{
public:
	PlaneWater(Graphics& gfx, float size);
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	void SetRotation(float roll, float pitch, float yaw) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	void SpawnControlWindow(Graphics& gfx, const char* name) noexcept;
	void UpdateENV(float pitch, float yaw, float roll) noexcept;
	void LinkTechniquesEX(Rgph::RenderGraph& rg);
	void SubmitEX(size_t channels) const;
public:
	struct VSMaterialConstant
	{
		float time;
		alignas(16) DirectX::XMFLOAT4 amplitude = { 0.071f,0.032f,0.048f,0.063f };
		alignas(16) DirectX::XMFLOAT4 speed = { 0.097f,0.258f,0.179f,0.219f };
		alignas(16) DirectX::XMFLOAT4 wavelength = { 0.887f,0.774f,0.790f,0.844f };
		alignas(16) DirectX::XMFLOAT4 omega = { 0.0f,0.0f,0.0f,0.0f };
		alignas(16) DirectX::XMFLOAT4 Q = { 1.0f,0.871f,0.935f,0.844f };
		alignas(16) DirectX::XMFLOAT4 directionX = { 0.0f,0.113f,0.306f,0.281f };
		alignas(16) DirectX::XMFLOAT4 directionZ = { 0.629f,0.081f,0.484f,0.156f };
		alignas(16) DirectX::XMFLOAT3 color = { 0.0f,0.384313f,0.580392f };
		alignas(16) DirectX::XMFLOAT3 attenuation = { 5.0f,5.0f,5.0f };
		alignas(16) DirectX::XMFLOAT3 scatteringKd = { 1.0f,1.0f,1.0f };
		float depth = 1.0f;
		float padding[2];
	} vmc;
private:
	DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };
	float roll = 0.0f;
	float pitch = 0.0f;
	float yaw = 0.0f;
	std::shared_ptr<Bind::CachingPixelConstantBufferEx> cBuf;
	PlaneWaterNormalBake normalPlane;
};