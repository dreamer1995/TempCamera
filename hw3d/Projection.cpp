#include "Projection.h"
#include "imgui/imgui.h"
#include "Graphics.h"
#include "ChiliMath.h"

Projection::Projection(Graphics& gfx, float FOV, float aspect, float nearZ, float farZ, bool isPerspective)
	:
	FOV(FOV),
	aspect(aspect),
	nearZ( nearZ ),
	farZ( farZ ),
	homeFOV(FOV), homeAspect(aspect), homeNearZ(nearZ), homeFarZ(farZ),
	isPerspective(isPerspective)
{
	if (isPerspective)
	{
		UIFOV = FOV / PI * 180.0f;
		height = 2 * nearZ * std::tan(FOV / 2.0f);
		width = height * aspect;
		frust = std::make_unique<Frustum>(gfx, width, height, nearZ, farZ, isPerspective);
		homeWidth = width;
		homeHeight = height;
	}
	else
	{
		width = aspect;
		height = FOV;
		homeWidth = width;
		homeHeight = height;
		frust = std::make_unique<Frustum>(gfx, width, height, nearZ, farZ, isPerspective);
	}
	mOffsetX = mOffsetY = 0;
	screenW = gfx.GetWidth();
	screenH = gfx.GetHeight();
}

DirectX::XMMATRIX Projection::GetMatrix() const
{
	if (isPerspective)
	{
		DirectX::XMFLOAT4X4 projArray4x4;
		DirectX::XMStoreFloat4x4(&projArray4x4, DirectX::XMMatrixPerspectiveFovLH(FOV, aspect, nearZ, farZ));
		float vDirection = -1;
		projArray4x4._31 = -2.0f * mOffsetX / screenW;
		projArray4x4._32 = vDirection * -2.0f * mOffsetY / screenH;
		return DirectX::XMLoadFloat4x4(&projArray4x4);
	}
	else
		return DirectX::XMMatrixOrthographicLH(width, height, nearZ, farZ);
}

void Projection::RenderWidgets( Graphics& gfx )
{
	bool dirty = false;
	const auto dcheck = [&dirty]( bool d ) { dirty = dirty || d; };

	if (isPerspective)
	{
		ImGui::Text("Projection");
		dcheck(ImGui::SliderFloat("FOV", &UIFOV, 1.0f, 179.0f, "%.0f"));
		dcheck(ImGui::SliderFloat("Aspect", &aspect, 0.01f, 1000.0f, "%.7f", 10.0f));
		dcheck(ImGui::SliderFloat("Near Z", &nearZ, 0.01f, farZ - 0.01f, "%.2f", 4.0f));
		dcheck(ImGui::SliderFloat("Far Z", &farZ, nearZ + 0.01f, 400.0f, "%.2f", 4.0f));
	}
	else
	{
		ImGui::Text("Projection");
		dcheck(ImGui::SliderFloat("Width", &width, 0.01f, 4096.f, "%.2f", 1.0f));
		dcheck(ImGui::SliderFloat("Height", &height, 0.01f, 4096.0f, "%.2f", 1.0f));
		dcheck(ImGui::SliderFloat("Near Z", &nearZ, 0.01f, farZ - 0.01f, "%.2f", 1.0f));
		dcheck(ImGui::SliderFloat("Far Z", &farZ, nearZ + 0.01f, 400.0f, "%.2f", 1.0f));
	}

	if( dirty )
	{
		if (isPerspective)
		{
			FOV = UIFOV / 180.0f * PI;
			height = 2 * nearZ * std::tan(FOV / 2.0f);
			width = height * aspect;
		}
		frust->SetVertices( gfx,width,height,nearZ,farZ );
	}
}

void Projection::SetPos( DirectX::XMFLOAT3 pos )
{
	frust->SetPos( pos );
}

void Projection::SetRotation( DirectX::XMFLOAT3 rot )
{
	frust->SetRotation( rot );
}

void Projection::Submit( size_t channel ) const
{
	frust->Submit( channel );
}

void Projection::LinkTechniques( Rgph::RenderGraph& rg )
{
	frust->LinkTechniques( rg );
}

void Projection::Reset( Graphics& gfx )
{
	width = homeWidth;
	height = homeHeight;
	nearZ = homeNearZ;
	farZ = homeFarZ;
	FOV = homeFOV;
	UIFOV = FOV / PI * 180.0f;
	aspect = homeAspect;
	frust->SetVertices( gfx,width,height,nearZ,farZ );	
}

void Projection::SetProjection(float _FOV, float _aspect, float _nearZ, float _farZ)
{	
	FOV = _FOV;
	aspect = _aspect;
	nearZ = _nearZ;
	farZ = _farZ;
	if (isPerspective)
	{
		UIFOV = FOV / PI * 180.0f;
		height = 2 * nearZ * std::tan(FOV / 2.0f);
		width = height * aspect;
	}
	else
	{
		width = FOV;
		height = aspect;	
	}
}

DirectX::XMFLOAT2 Projection::GetFarNearPlane() const
{
	return { farZ,nearZ };
}

float Projection::GetFOV() const
{
	return FOV;
}

float Projection::GetAspect() const
{
	return aspect;
}

void Projection::UpdateScreenResolution(Graphics& gfx)
{
	screenW = gfx.GetWidth();
	screenH = gfx.GetHeight();
}

void Projection::SetOffsetPixels(float offsetX, float offsetY) noxnd
{
	mOffsetX = offsetX;
	mOffsetY = offsetY;
}