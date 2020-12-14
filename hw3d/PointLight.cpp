#include "PointLight.h"
#include "imgui/imgui.h"
//#include "Camera.h"
#include "ChiliMath.h"

PointLight::PointLight(Graphics& gfx, DirectX::XMFLOAT3 pos, float radius, UINT slot)
	:
	mesh( gfx,radius ),
	cbuf(gfx, slot)
{
	home = {
		pos,
		{ 0.0f,0.0f,0.0f }, // Ambient
		{ 1.0f,1.0f,1.0f }, // Color
		1.0f, // Intensity
		1.0f, // AttConst
		0.045f, // Lin
		0.0075f, // Quad
	};
	Reset();
	//pCamera = std::make_shared<Camera>(gfx, "PointLight", cbData.pos, 0.0f, PI / 2, true);
}

void PointLight::SpawnControlWindow(const char* name) noexcept
{
	if (ImGui::Begin(name))
	{
		//bool dirtyPos = false;
		//const auto d = [&dirtyPos]( bool dirty ){dirtyPos = dirtyPos || dirty;};

		//ImGui::Text( "Position" );
		//d( ImGui::SliderFloat( "X",&cbData.pos.x,-60.0f,60.0f,"%.1f" ) );
		//d( ImGui::SliderFloat( "Y",&cbData.pos.y,-60.0f,60.0f,"%.1f" ) );
		//d( ImGui::SliderFloat( "Z",&cbData.pos.z,-60.0f,60.0f,"%.1f" ) );

		//if( dirtyPos )
		//{
		//	pCamera->SetPos( cbData.pos );
		//}

		ImGui::Text( "Position" );
		ImGui::SliderFloat( "X",&cbData.pos.x,-60.0f,60.0f,"%.1f" );
		ImGui::SliderFloat( "Y",&cbData.pos.y,-60.0f,60.0f,"%.1f" );
		ImGui::SliderFloat( "Z",&cbData.pos.z,-60.0f,60.0f,"%.1f" );

		ImGui::Text( "Intensity/Color" );
		ImGui::SliderFloat( "Intensity",&cbData.diffuseIntensity,0.0f,2.0f,"%.2f",2 );
		ImGui::ColorEdit3( "Diffuse Color",&cbData.diffuseColor.x );
		ImGui::ColorEdit3( "Ambient",&cbData.ambient.x );
		
		ImGui::Text( "Falloff" );
		ImGui::SliderFloat( "Constant",&cbData.attConst,0.05f,10.0f,"%.2f",4 );
		ImGui::SliderFloat( "Linear",&cbData.attLin,0.0001f,4.0f,"%.4f",8 );
		ImGui::SliderFloat( "Quadratic",&cbData.attQuad,0.0000001f,10.0f,"%.7f",10 );

		if( ImGui::Button( "Reset" ) )
		{
			Reset();
		}

		mesh.ChangeSphereMaterialState();
	}
	ImGui::End();
}

void PointLight::Reset() noexcept
{
	cbData = home;
	auto buf = mesh.cbuf->GetBuffer();
	buf["color"] = DirectX::XMFLOAT3{
		cbData.diffuseColor.x * cbData.diffuseIntensity,
		cbData.diffuseColor.y * cbData.diffuseIntensity,
		cbData.diffuseColor.z * cbData.diffuseIntensity
	};
	mesh.cbuf->SetBuffer(buf);
}

void PointLight::Submit( size_t channels ) const noxnd
{
	mesh.SetPos( cbData.pos );
	mesh.SetColor({ 
		cbData.diffuseColor.x * cbData.diffuseIntensity,
		cbData.diffuseColor.y* cbData.diffuseIntensity,
		cbData.diffuseColor.z* cbData.diffuseIntensity
		});
	mesh.Submit( channels );
}

void PointLight::Bind(Graphics& gfx) const noexcept
{
	//auto dataCopy = cbData;
	//const auto pos = DirectX::XMLoadFloat3( &cbData.pos );
	//DirectX::XMStoreFloat3( &dataCopy.pos,DirectX::XMVector3Transform( pos,view ) );
	cbuf.Update( gfx, cbData);
	cbuf.Bind( gfx );
}

void PointLight::LinkTechniques( Rgph::RenderGraph& rg )
{
	mesh.LinkTechniques( rg );
}

//std::shared_ptr<Camera> PointLight::ShareCamera() const noexcept
//{
//	return pCamera;
//}

DirectX::XMFLOAT3 PointLight::GetPos() noexcept
{
	return cbData.pos;
}

void PointLight::RotateAround(float dx, float dy, DirectX::XMFLOAT3 centralPoint, float speed) noexcept
{
	using namespace DirectX;
	// Rotate camera around a point
	{
		DirectX::XMFLOAT3 destination;
		XMVECTOR rotateVector = XMVectorSubtract(XMLoadFloat3(&cbData.pos), XMLoadFloat3(&centralPoint));
		XMFLOAT3 finalRatationVector;
		XMStoreFloat3(&finalRatationVector, XMVector3Transform(rotateVector,
			XMMatrixRotationQuaternion(XMQuaternionRotationAxis(XMVector3Transform(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),
				XMMatrixRotationRollPitchYaw(0, 0.0f, 0.0f)), dy * speed))
			* XMMatrixRotationQuaternion(XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), dx * speed))
		));
		XMStoreFloat3(&cbData.pos,
			XMVector3Transform(XMLoadFloat3(&centralPoint), XMMatrixTranslation(finalRatationVector.x, finalRatationVector.y, finalRatationVector.z)));
	}
}