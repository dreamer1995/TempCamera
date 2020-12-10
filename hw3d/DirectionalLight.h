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

class Camera;

class DirectionalLight
{
public:
	DirectionalLight(Graphics& gfx, DirectX::XMFLOAT3 pos = { 0.0f,10.0f,0.0f }, float pitch = -PI / 4, float yaw = -PI / 4,
		float radius = 0.5f, float size = 1.0f);
	void SpawnControlWindow() noexcept;
	void Reset() noexcept;
	void Submit(size_t channels) const noxnd;
	void Bind(Graphics& gfx) const noexcept;
	void LinkTechniques(Rgph::RenderGraph&);
	DirectX::XMFLOAT3 GetPos() noexcept;
	DirectX::XMFLOAT3 GetDirection() noexcept;
	void Rotate(float dx, float dy) noexcept;
	std::shared_ptr<Camera> ShareCamera() const noexcept;
private:
	struct DirectionalLightCBuf
	{
		alignas(16) DirectX::XMFLOAT3 direction;
		alignas(16) DirectX::XMFLOAT3 diffuseColor;
		float diffuseIntensity;
		DirectX::XMMATRIX lightSpaceVP;
		float padding[3];
	};
	struct DirectionalLightProperties
	{
		DirectionalLightCBuf cbData;
		DirectX::XMFLOAT3 pos;
		float pitch;
		float yaw;
		float length;
	};
private:
	DirectionalLightCBuf cbData;
	mutable PhongSphere mesh;
	mutable SolidArrow arrow;
	mutable Bind::PixelConstantBuffer<DirectionalLightCBuf> cbuf;
	DirectX::XMFLOAT3 pos;
	float pitch;
	float yaw;
	static constexpr float rotationSpeed = 0.004f;
	DirectionalLightProperties home;
	std::shared_ptr<Camera> pCamera;
};