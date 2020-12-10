#include "DirectionalLight.h"
#include "imgui/imgui.h"
#include "Camera.h"

DirectionalLight::DirectionalLight(Graphics& gfx, DirectX::XMFLOAT3 pos, float pitch, float yaw, float radius, float size)
	:
	mesh(gfx, radius),
	arrow(gfx, size),
	cbuf(gfx, 3u)
{
	cbData = {
	{ 0.0f,-1.0f,0.0f },
	{ 1.0f,1.0f,1.0f }, // Color
	0.0f, // Intensity
	};
	home = { cbData,
		pos,
		pitch,
		yaw,
		1.0f // Arrow Length
	};
	Reset();
	pCamera = std::make_shared<Camera>(gfx, "DirectionalLight", pos, pitch, yaw, true);
}

void DirectionalLight::SpawnControlWindow() noexcept
{
	if (ImGui::Begin("DirectionalLight"))
	{
		bool rotDirty = false;
		bool posDirty = false;
		const auto dcheck = [](bool d, bool& carry) { carry = carry || d; };
		ImGui::Text("Position");
		dcheck(ImGui::SliderFloat("X", &pos.x, -80.0f, 80.0f, "%.1f"), posDirty);
		dcheck(ImGui::SliderFloat("Y", &pos.y, -80.0f, 80.0f, "%.1f"), posDirty);
		dcheck(ImGui::SliderFloat("Z", &pos.z, -80.0f, 80.0f, "%.1f"), posDirty);
		if (posDirty)
		{
			pCamera->SetPos(pos);
		}
		ImGui::Text("Orientation");
		dcheck(ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f), rotDirty);
		dcheck(ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f), rotDirty);
		if (rotDirty)
		{
			pCamera->SetRotation(pitch, yaw);
		}

		ImGui::Text("Intensity/Color");
		ImGui::SliderFloat("Intensity", &cbData.diffuseIntensity, 0.01f, 2.0f, "%.2f", 2);
		ImGui::ColorPicker3("Diffuse Color", &cbData.diffuseColor.x);

		if (ImGui::Button("Reset"))
		{
			Reset();
		}
		mesh.ChangeSphereMaterialState();

		arrow.ChangeArrowColor();
	}
	ImGui::End();
}

void DirectionalLight::Reset() noexcept
{
	cbData = home.cbData;
	pos = home.pos;
	pitch = home.pitch;
	yaw = home.yaw;
	auto buf = arrow.cbuf->GetBuffer();
	buf["length"] = home.length;
	arrow.cbuf->SetBuffer(buf);
}

void DirectionalLight::Submit(size_t channels) const noxnd
{
	mesh.SetPos(pos);
	mesh.Submit(channels);
	arrow.SetTransform(pos, pitch, yaw);
	arrow.SetColor(cbData.diffuseColor);
	arrow.Submit(channels);
}

void DirectionalLight::Bind(Graphics& gfx) const noexcept
{
	auto dataCopy = cbData;

	namespace dx = DirectX;
	using namespace dx;
	XMStoreFloat3(&dataCopy.direction,
		XMVector3Normalize(
			XMVectorNegate(
				XMVector3Transform(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f),
					XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f)
				))));
	//const auto lightPos = XMVectorSet(pos.x, pos.y, pos.z, 0.0f);

	cbuf.Update(gfx, dataCopy);
	cbuf.Bind(gfx);
}

void DirectionalLight::LinkTechniques(Rgph::RenderGraph& rg)
{
	mesh.LinkTechniques(rg);
	arrow.LinkTechniques(rg);
}

DirectX::XMFLOAT3 DirectionalLight::GetPos() noexcept
{
	return pos;
}

DirectX::XMFLOAT3 DirectionalLight::GetDirection() noexcept
{
	return cbData.direction;
}

void DirectionalLight::Rotate(float dx, float dy) noexcept
{
	yaw = wrap_angle(yaw + dx * rotationSpeed);
	pitch = wrap_angle(pitch + dy * rotationSpeed);
	pCamera->SetRotation(pitch, yaw);
}

std::shared_ptr<Camera> DirectionalLight::ShareCamera() const noexcept
{
	return pCamera;
}