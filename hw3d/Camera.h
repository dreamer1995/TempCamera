#pragma once
#include "Graphics.h"
#include "ConstantBuffers.h"
#include <DirectXMath.h>
#include <string>
#include "Projection.h"
#include "CameraIndicator.h"
#include "ChiliMath.h"

class Graphics;
namespace Rgph
{
	class RenderGraph;
}

class Camera
{
public:
	Camera(Graphics& gfx, std::string name, DirectX::XMFLOAT3 homePos = { 0.0f,0.0f,0.0f }, float homePitch = 0.0f, float homeYaw = 0.0f,
		bool tethered = false, bool isPerspective = true, float FOV = 60.0f / 180.0f * PI, float aspect = 16.0f / 9.0f,
		float farPlane = 400.0f, float nearPlane = 0.5f) noexcept;
	void BindToGraphics( Graphics& gfx ) const;
	DirectX::XMMATRIX GetMatrix() const noexcept;
	DirectX::XMMATRIX GetProjection() const noexcept;
	void SpawnControlWidgets( Graphics& gfx ) noexcept;
	void Reset( Graphics& gfx ) noexcept;
	void Rotate( float dx,float dy ) noexcept;
	void Translate( DirectX::XMFLOAT3 translation ) noexcept;
	DirectX::XMFLOAT3 GetPos() const noexcept;
	void SetPos( const DirectX::XMFLOAT3& pos ) noexcept;
	const std::string& GetName() const noexcept;
	void LinkTechniques( Rgph::RenderGraph& rg );
	void Submit( size_t channel ) const;

	void LookZero(DirectX::XMFLOAT3 position) noexcept;
	void RotateAround(float dx, float dy, DirectX::XMFLOAT3 centralPoint) noexcept;
	DirectX::XMFLOAT3 pos;
	void Bind(Graphics& gfx) const noexcept;
	float yaw;
	std::string name;
	void SetRotation(float pitch, float yaw) noexcept;
	void ProjectScreenToWorldExpansionBasis(DirectX::XMFLOAT4& vWBasisX, DirectX::XMFLOAT4& vWBasisY, DirectX::XMFLOAT4& vWBasisZ,
		DirectX::XMFLOAT2& UVToViewA, DirectX::XMFLOAT2& UVToViewB) const noxnd;
	void SetOffsetPixels(float offsetX, float offsetY) noxnd;
private:
	struct CameraCBuf
	{
		alignas(16) DirectX::XMFLOAT3 pos;
		alignas(16) DirectX::XMFLOAT3 direction;
		alignas(16) DirectX::XMFLOAT2 FNPlane;
		alignas(16) DirectX::XMFLOAT4 vWBasisX;
		alignas(16) DirectX::XMFLOAT4 vWBasisY;
		alignas(16) DirectX::XMFLOAT4 vWBasisZ;
		DirectX::XMFLOAT2 UVToViewA;
		DirectX::XMFLOAT2 UVToViewB;
	};

private:
	bool tethered;
	DirectX::XMFLOAT3 homePos;
	float homePitch;
	float homeYaw;
	float pitch;
	static constexpr float travelSpeed = 12.0f;
	static constexpr float rotationSpeed = 0.004f;
	bool enableCameraIndicator = false;
	bool enableFrustumIndicator = false;
	Projection proj;
	CameraIndicator indicator;

	void KeepLookFront(DirectX::XMFLOAT3 position) noexcept;
	float yaw_;
	mutable Bind::VertexConstantBuffer<CameraCBuf> vCbuf;
	mutable Bind::PixelConstantBuffer<CameraCBuf> pCbuf;
	bool isPerspective;
};