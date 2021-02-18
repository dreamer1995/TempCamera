#pragma once
#include <DirectXMath.h>
#include "Frustum.h"

class Graphics;
namespace Rgph
{
	class RenderGraph;
}

class Projection
{
public:
	Projection(Graphics& gfx, float FOV, float aspect, float nearZ, float farZ, bool isPerspective);
	DirectX::XMMATRIX GetMatrix() const;
	void RenderWidgets( Graphics& gfx );
	void SetPos( DirectX::XMFLOAT3 );
	void SetRotation( DirectX::XMFLOAT3 );
	void Submit( size_t channel ) const;
	void LinkTechniques( Rgph::RenderGraph& rg );
	void Reset( Graphics& gfx );
	void SetProjection(float _width, float _height, float _nearZ, float _farZ);
	DirectX::XMFLOAT2 GetFarNearPlane() const;
	float GetFOV() const;
	float GetAspect() const;
private:
	float width;
	float height;
	float nearZ;
	float farZ;
	float homeWidth;
	float homeHeight;
	float homeNearZ;
	float homeFarZ;
	std::unique_ptr<Frustum> frust;
	bool isPerspective;
	float FOV;
	float UIFOV;
	float aspect;
	float homeFOV;
	float homeAspect;
};