#pragma once
#include "Graphics.h"
#include "SolidSphere.h"
#include "ConstantBuffers.h"
#include "ConditionalNoexcept.h"

namespace Rgph
{
	class RenderGraph;
}

//class Camera;

class PointLight
{
public:
	PointLight(Graphics& gfx, DirectX::XMFLOAT3 pos = { 10.0f,9.0f,2.5f }, float radius = 0.5f, UINT slot = 4u);
	void SpawnControlWindow(const char* name) noexcept;
	void Reset() noexcept;
	void Submit( size_t channels ) const noxnd;
	void Bind(Graphics& gfx) const noexcept;
	void LinkTechniques( Rgph::RenderGraph& );
	//std::shared_ptr<Camera> ShareCamera() const noexcept;
	DirectX::XMFLOAT3 GetPos() noexcept;
	void RotateAround(float dx, float dy, DirectX::XMFLOAT3 centralPoint, float speed) noexcept;
private:
	struct PointLightCBuf
	{
		alignas(16) DirectX::XMFLOAT3 pos;
		alignas(16) DirectX::XMFLOAT3 ambient;
		alignas(16) DirectX::XMFLOAT3 diffuseColor;
		float diffuseIntensity;
		float attConst;
		float attLin;
		float attQuad;
	};
private:
	PointLightCBuf home;
	PointLightCBuf cbData;
	mutable SolidSphere mesh;
	mutable Bind::PixelConstantBuffer<PointLightCBuf> cbuf;
	//std::shared_ptr<Camera> pCamera;
};