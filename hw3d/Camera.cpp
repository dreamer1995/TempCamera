#include "Camera.h"
#include "imgui/imgui.h"

namespace dx = DirectX;

Camera::Camera(Graphics& gfx, std::string name, DirectX::XMFLOAT3 homePos, float homePitch, float homeYaw,
	bool tethered, bool isPerspective, float FOV, float aspect, float farPlane, float nearPlane) noexcept
	:
	name( std::move( name ) ),
	homePos( homePos ),
	homePitch( homePitch ),
	homeYaw( homeYaw ),
	proj(gfx, FOV, aspect, nearPlane, farPlane, isPerspective),
	indicator( gfx ),
	tethered( tethered ),
	vCbuf(gfx, 1u),
	pCbuf(gfx, 1u),
	isPerspective(isPerspective)
{
	if( tethered )
	{
		pos = homePos;
		indicator.SetPos( pos );
		proj.SetPos( pos );
	}
	Reset( gfx );
}

void Camera::BindToGraphics( Graphics& gfx ) const
{
	gfx.SetCamera( GetMatrix() );
	gfx.SetProjection( proj.GetMatrix() );
}

DirectX::XMMATRIX Camera::GetMatrix() const noexcept
{
	using namespace dx;

	const dx::XMVECTOR forwardBaseVector = XMVectorSet( 0.0f,0.0f,1.0f,0.0f );
	// apply the camera rotations to a base vector
	const auto lookVector = XMVector3Transform( forwardBaseVector,
		XMMatrixRotationRollPitchYaw( pitch,yaw,0.0f )
	);
	// generate camera transform (applied to all objects to arrange them relative
	// to camera position/orientation in world) from cam position and direction
	// camera "top" always faces towards +Y (cannot do a barrel roll)
	const auto camPosition = XMLoadFloat3( &pos );
	const auto camTarget = camPosition + lookVector;

	XMVECTOR upVector;
	if (-PI / 2.0f >= pitch || pitch >= PI / 2.0f)
	{
		upVector = XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f);
	}
	else
	{
		upVector = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	}
	return XMMatrixLookAtLH(camPosition, camTarget, upVector);
}

DirectX::XMMATRIX Camera::GetProjection() const noexcept
{
	return proj.GetMatrix();
}

void Camera::SpawnControlWidgets(Graphics& gfx) noexcept
{
	bool rotDirty = false;
	bool posDirty = false;
const auto dcheck = []( bool d,bool& carry ) { carry = carry || d; };
	if( !tethered )
	{
		ImGui::Text( "Position" );
		dcheck( ImGui::SliderFloat( "X",&pos.x,-80.0f,80.0f,"%.1f" ),posDirty );
		dcheck( ImGui::SliderFloat( "Y",&pos.y,-80.0f,80.0f,"%.1f" ),posDirty );
		dcheck( ImGui::SliderFloat( "Z",&pos.z,-80.0f,80.0f,"%.1f" ),posDirty );
	}
	ImGui::Text( "Orientation" );
	dcheck( ImGui::SliderAngle( "Pitch",&pitch,0.995f * -90.0f,0.995f * 90.0f ), rotDirty );
	dcheck( ImGui::SliderAngle( "Yaw",&yaw,-180.0f,180.0f ), rotDirty );
	proj.RenderWidgets( gfx );
	ImGui::Checkbox( "Camera Indicator",&enableCameraIndicator );
	ImGui::Checkbox( "Frustum Indicator",&enableFrustumIndicator );
	if( ImGui::Button( "Reset" ) )
	{
		Reset( gfx );
	}

	if( rotDirty )
	{
		yaw_ = yaw;
		const dx::XMFLOAT3 angles = { pitch,yaw,0.0f };
		indicator.SetRotation( angles );
		proj.SetRotation( angles );
	}
	if( posDirty )
	{
		indicator.SetPos( pos );
		proj.SetPos( pos );
	}
}

void Camera::Reset( Graphics& gfx ) noexcept
{
	/*pos = { 0.0f,0.0f,-14.0f };
	pitch = 0.0f;
	yaw = 0.0f;*/
	if( !tethered )
	{
		pos = homePos;
		indicator.SetPos( pos );
		proj.SetPos( pos );
	}
	pitch = homePitch;
	yaw = homeYaw;
	yaw_ = homeYaw;

	const dx::XMFLOAT3 angles = { pitch,yaw,0.0f };
	indicator.SetRotation( angles );
	proj.SetRotation( angles );
	proj.Reset( gfx );
}

void Camera::Rotate( float dx,float dy ) noexcept
{
	yaw = wrap_angle( yaw + dx * rotationSpeed );
	//pitch = std::clamp( pitch + dy * rotationSpeed,0.995f * -PI / 2.0f,0.995f * PI / 2.0f );
	pitch = wrap_angle(pitch + dy * rotationSpeed);
	yaw_ = yaw;
	const dx::XMFLOAT3 angles = { pitch,yaw,0.0f };
	indicator.SetRotation( angles );
	proj.SetRotation( angles );
}

void Camera::Translate( DirectX::XMFLOAT3 translation ) noexcept
{
	if (!tethered)
	{
		dx::XMStoreFloat3(&translation, dx::XMVector3Transform(
			dx::XMLoadFloat3(&translation),
			dx::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f) *
			dx::XMMatrixScaling(travelSpeed, travelSpeed, travelSpeed)
		));
		pos = {
			pos.x + translation.x,
			pos.y + translation.y,
			pos.z + translation.z
		};
		indicator.SetPos(pos);
		proj.SetPos(pos);
	}
}

DirectX::XMFLOAT3 Camera::GetPos() const noexcept
{
	return pos;
}

void Camera::SetPos( const DirectX::XMFLOAT3& pos ) noexcept
{
	this->pos = pos;
	indicator.SetPos( pos );
	proj.SetPos( pos );
}

const std::string& Camera::GetName() const noexcept
{
	return name;
}

void Camera::LinkTechniques( Rgph::RenderGraph& rg )
{
	indicator.LinkTechniques( rg );
	proj.LinkTechniques( rg );
}

void Camera::Submit( size_t channels ) const
{
	if( enableCameraIndicator )
	{
		indicator.Submit( channels );
	}
	if( enableFrustumIndicator )
	{
		proj.Submit( channels );
	}
}

void Camera::LookZero(DirectX::XMFLOAT3 position) noexcept
{
	DirectX::XMFLOAT3 delta = { pos.x - position.x,pos.y - position.y, pos.z - position.z };
	pitch = wrap_angle(atan2(delta.y, sqrt(delta.x * delta.x + delta.z * delta.z)));
	yaw = wrap_angle(atan2(delta.x, delta.z) + PI);
	yaw_ = yaw;
}

void Camera::KeepLookFront(DirectX::XMFLOAT3 position) noexcept
{
	DirectX::XMFLOAT3 delta = { pos.x - position.x,pos.y - position.y, pos.z - position.z };
	if (-PI / 2.0f >= pitch || pitch >= PI / 2.0f)
	{
		if (0.3 * PI < abs(yaw_ - wrap_angle(atan2(delta.x, delta.z))) && abs(yaw_ - wrap_angle(atan2(delta.x, delta.z))) < 0.9 * PI * 2)
		{
			pitch = wrap_angle(atan2(delta.y, sqrt(delta.x * delta.x + delta.z * delta.z)));
			yaw = wrap_angle(atan2(delta.x, delta.z) + PI);
			yaw_ = yaw;
		}
		else
		{
			pitch = wrap_angle(-atan2(delta.y, sqrt(delta.x * delta.x + delta.z * delta.z)) - PI);
			yaw = wrap_angle(atan2(delta.x, delta.z));
			yaw_ = yaw;
		}
	}
	else
	{
		if (0.3 * PI < abs(yaw_ - wrap_angle(atan2(delta.x, delta.z) + PI)) && abs(yaw_ - wrap_angle(atan2(delta.x, delta.z) + PI)) < 0.9 * PI * 2)
		{
			pitch = wrap_angle(-atan2(delta.y, sqrt(delta.x * delta.x + delta.z * delta.z)) - PI);
			yaw = wrap_angle(atan2(delta.x, delta.z));
			yaw_ = yaw;
		}
		else
		{
			pitch = wrap_angle(atan2(delta.y, sqrt(delta.x * delta.x + delta.z * delta.z)));
			yaw = wrap_angle(atan2(delta.x, delta.z) + PI);
			yaw_ = yaw;
		}
	}
}

void Camera::RotateAround(float dx, float dy, DirectX::XMFLOAT3 centralPoint) noexcept
{
	using namespace dx;
	// Rotate camera around a point
	{
		DirectX::XMFLOAT3 lookVector, destination;
		XMVECTOR rotateVector = XMVectorSubtract(XMLoadFloat3(&pos), XMLoadFloat3(&centralPoint));
		dx::XMStoreFloat3(&lookVector, XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f)));
		XMFLOAT3 finalRatationViewVector;
		XMStoreFloat3(&finalRatationViewVector, dx::XMVector3Transform(rotateVector,
			dx::XMMatrixTranslation(lookVector.x, lookVector.y, lookVector.z) *
			XMMatrixRotationQuaternion(XMQuaternionRotationAxis(XMVector3Transform(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),
				XMMatrixRotationRollPitchYaw(0, yaw, 0.0f)), dy * rotationSpeed))
			* XMMatrixRotationQuaternion(XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), dx * rotationSpeed))
		));
		dx::XMStoreFloat3(&destination,
			XMVector3Transform(XMLoadFloat3(&centralPoint), XMMatrixTranslation(finalRatationViewVector.x, finalRatationViewVector.y, finalRatationViewVector.z)));
		XMFLOAT3 finalRatationVector;
		XMStoreFloat3(&finalRatationVector, XMVector3Transform(rotateVector,
			XMMatrixRotationQuaternion(XMQuaternionRotationAxis(XMVector3Transform(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f),
				XMMatrixRotationRollPitchYaw(0, yaw, 0.0f)), dy * rotationSpeed))
			* XMMatrixRotationQuaternion(XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), dx * rotationSpeed))
		));
		XMStoreFloat3(&pos,
			XMVector3Transform(XMLoadFloat3(&centralPoint), XMMatrixTranslation(finalRatationVector.x, finalRatationVector.y, finalRatationVector.z)));
		KeepLookFront(destination);
	}
}

void Camera::Bind(Graphics& gfx) noexcept
{
	using namespace dx;
	const dx::XMVECTOR forwardBaseVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	// apply the camera rotations to a base vector
	dx::XMFLOAT3 lookVector;
	XMStoreFloat3(&lookVector, XMVector3Transform(forwardBaseVector,
		XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f)
	));
	DirectX::XMFLOAT4 vWBasisX;
	DirectX::XMFLOAT4 vWBasisY;
	DirectX::XMFLOAT4 vWBasisZ;
	ProjectScreenToWorldExpansionBasis(vWBasisX, vWBasisY, vWBasisZ);
	CameraCBuf cbData = { pos,lookVector,proj.GetFarNearPlane(),vWBasisX,vWBasisY,vWBasisZ };
	vCbuf.Update(gfx, cbData);
	vCbuf.Bind(gfx);
	pCbuf.Update(gfx, cbData);
	pCbuf.Bind(gfx);
	proj.UpdateScreenResolution(gfx);
}

void Camera::SetRotation(float pitch, float yaw) noexcept
{
	this->pitch = pitch;
	this->yaw = yaw;
	yaw_ = yaw;
	const dx::XMFLOAT3 angles = { pitch,yaw,0.0f };
	indicator.SetRotation(angles);
	proj.SetRotation(angles);
}

void Camera::ProjectScreenToWorldExpansionBasis(dx::XMFLOAT4& vWBasisX, dx::XMFLOAT4& vWBasisY, dx::XMFLOAT4& vWBasisZ) const noxnd
{
	DirectX::XMFLOAT4X4 cameraToWorld;
	dx::XMStoreFloat4x4(&cameraToWorld, DirectX::XMMatrixInverse(nullptr, GetMatrix()));

	float FOV = proj.GetFOV();
	float farPlane = proj.GetFarNearPlane().x;

	dx::XMFLOAT3 vX = { cameraToWorld._11,cameraToWorld._12,cameraToWorld._13 };
	dx::XMFLOAT3 vY = { -cameraToWorld._21,-cameraToWorld._22,-cameraToWorld._23 };
	dx::XMFLOAT3 vZ = { cameraToWorld._31,cameraToWorld._32,cameraToWorld._33 };
	auto NormalizeFloat3 = [](dx::XMFLOAT3& float3)
	{
		float length = std::sqrt(float3.x * float3.x + float3.y * float3.y + float3.z * float3.z);
		float3 = { float3.x / length, float3.y / length, float3.z / length };
	};
	NormalizeFloat3(vX);
	NormalizeFloat3(vY);
	NormalizeFloat3(vZ);

	float tanHalfFovToPlaneY = farPlane * std::tan(FOV * 0.5f);
	float tanHalfFovToPlaneX = proj.GetAspect() * tanHalfFovToPlaneY;

	vY = { vY.x * tanHalfFovToPlaneY,vY.y * tanHalfFovToPlaneY,vY.z * tanHalfFovToPlaneY };
	vX = { vX.x * tanHalfFovToPlaneX,vX.y * tanHalfFovToPlaneX,vX.z * tanHalfFovToPlaneX };
	vZ = { vZ.x * farPlane			,vZ.y * farPlane		  ,vZ.z * farPlane };

	// remapping for input in range (x:[0, 1], y:[0, 1], z:[0,1])
	vZ = { vZ.x - vX.x - vY.x,vZ.y - vX.y - vY.y,vZ.z - vX.z - vY.z };
	vX = { vX.x * 2,vX.y * 2,vX.z * 2 };
	vY = { vY.x * 2,vY.y * 2,vY.z * 2 };

	vWBasisX.x = vX.x; vWBasisX.y = vX.y; vWBasisX.z = vX.z; vWBasisX.w = 0.0f;
	vWBasisY.x = vY.x; vWBasisY.y = vY.y; vWBasisY.z = vY.z; vWBasisY.w = 0.0f;
	vWBasisZ.x = vZ.x; vWBasisZ.y = vZ.y; vWBasisZ.z = vZ.z; vWBasisZ.w = 0.0f;
}

void Camera::SetOffsetPixels(float offsetX, float offsetY) noxnd
{
	proj.SetOffsetPixels(offsetX, offsetY);
}