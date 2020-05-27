#pragma once
#include "Graphics.h"
#include "PhongSphere.h"
#include "ConstantBuffers.h"
#include "ConditionalNoexcept.h"
#include "ChiliMath.h"
#include "SolidArrow.h"

namespace Rgph
{
	class RenderGraph;
}

class DirectionalLight
{
public:
	DirectionalLight(Graphics& gfx, float radius = 0.5f, float size = 1.0f);
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;
	void Submit(size_t channels) const noxnd;
	void Bind(Graphics& gfx) const noexcept;
	void LinkTechniques(Rgph::RenderGraph&);
	DirectX::XMFLOAT3 GetPos() noexcept;
	DirectX::XMFLOAT3 GetDirection() noexcept;
	void Rotate(float dx, float dy) noexcept;
private:
	struct DirectionalLightCBuf
	{
		alignas(16) DirectX::XMFLOAT3 direction;
		alignas(16) DirectX::XMFLOAT3 diffuseColor;
		float diffuseIntensity;
		DirectX::XMMATRIX lightSpaceVP;
		float padding[3];
	};
private:
	float length = 1.0f;
	DirectionalLightCBuf cbData;
	mutable PhongSphere mesh;
	mutable SolidArrow arrow;
	mutable Bind::PixelConstantBuffer<DirectionalLightCBuf> cbuf;
	DirectX::XMFLOAT3 pos = { 0.0f,10.0f,0.0f };
	float pitch = 0.0f;
	float yaw = 0.0f;
	float roll = 0.0f;
	static constexpr float rotationSpeed = 0.004f;
};