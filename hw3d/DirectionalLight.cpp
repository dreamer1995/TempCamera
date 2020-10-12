#include "DirectionalLight.h"
#include "imgui/imgui.h"

DirectionalLight::DirectionalLight(Graphics& gfx, float radius, float size)
	:
	mesh(gfx, radius),
	arrow(gfx, size),
	cbuf(gfx, 3u)
{
	Reset();
}

void DirectionalLight::SpawnControlWindow() noexcept
{
	if (ImGui::Begin("DirectionalLight"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &pos.x, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Y", &pos.y, -60.0f, 60.0f, "%.1f");
		ImGui::SliderFloat("Z", &pos.z, -60.0f, 60.0f, "%.1f");
		ImGui::Text("Orientation");
		ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
		ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);
		ImGui::Text("Intensity/Color");
		ImGui::SliderFloat("Intensity", &cbData.diffuseIntensity, 0.01f, 2.0f, "%.2f", 2);
		ImGui::ColorEdit3("Diffuse Color", &cbData.diffuseColor.x);

		if (ImGui::Button("Reset"))
		{
			Reset();
		}
		ImGui::Text("Arrow");
		mesh.ChangeSphereMaterialState();

		ImGui::Text("Arrow");
		arrow.ChangeArrowColor();
		ImGui::SliderFloat("Arrow Length", &length, 0.0f, 5.0, "%.1f");
	}
	ImGui::End();
}

void DirectionalLight::Reset() noexcept
{
	cbData = {
		{ 0.0f,-1.0f,0.0f },
		{ 1.0f,1.0f,1.0f },
		1.0f,
	};
	pos = { 0.0f,10.0f,0.0f };
	pitch = -PI / 4;
	yaw = -PI / 4;

	roll = 0.0f;
}

void DirectionalLight::Submit(size_t channels) const noxnd
{
	mesh.SetPos(pos);
	mesh.Submit(channels);
	arrow.SetTransform(pos, pitch, yaw, roll, length);
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
				XMVector3Transform(XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f),
					XMMatrixRotationRollPitchYaw(pitch, yaw, roll)
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
}