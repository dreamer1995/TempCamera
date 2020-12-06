#include "SkyBox.h"
#include "Cube.h"
#include "BindableCommon.h"
#include "ConstantBuffersEx.h"
#include "imgui/imgui.h"
#include "DynamicConstant.h"
#include "TechniqueProbe.h"
#include "TransformCbufScaling.h"
#include "Channels.h"

SkyBox::SkyBox(Graphics& gfx, float size)
{
	using namespace Bind;
	namespace dx = DirectX;

	auto model = Cube::MakePosOnly();
	const auto geometryTag = "$skybox." + std::to_string(size);
	pVertices = VertexBuffer::Resolve(gfx, geometryTag, model.vertices);
	pIndices = IndexBuffer::Resolve(gfx, geometryTag, model.indices);
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// auto tcb = std::make_shared<TransformCbufScaling>(gfx);
	auto tcb = std::make_shared<TransformCbuf>(gfx);

	{	
		Technique shade("Shade1", { Chan::main });
		{
			Step only("environment");

			//only.AddBindable(Texture::Resolve(gfx, "Images\\EpicQuadPanorama_CC+EV1.jpg"));
			only.AddBindable(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Wrap, 0u, 0b1u));

			auto pvs = VertexShader::Resolve(gfx, "SkyBoxVS.cso");
			only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *pvs));
			only.AddBindable(std::move(pvs));

			only.AddBindable(PixelShader::Resolve(gfx, "SkyBoxPS.cso"));

			Dcb::RawLayout lay;
			lay.Add<Dcb::Float4>("color");
			auto buf = Dcb::Buffer(std::move(lay));
			buf["color"] = dx::XMFLOAT4{ 1.0f,1.0f,1.0f,1.0f };
			only.AddBindable(std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u));

			only.AddBindable(Rasterizer::Resolve(gfx, true));

			only.AddBindable(tcb);

			shade.AddStep(std::move(only));
		}
		AddTechnique(std::move(shade));
	}
}

//void SkyBox::SetPos(DirectX::XMFLOAT3 pos) noexcept
//{
//	this->pos = pos;
//}

void SkyBox::SetRotation(float roll, float pitch, float yaw) noexcept
{
	this->roll = roll;
	this->pitch = pitch;
	this->yaw = yaw;
}

DirectX::XMMATRIX SkyBox::GetTransformXM() const noexcept
{
	//return DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
	//	DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
	return DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);
}

void SkyBox::SpawnControlWindow(Graphics& gfx, const char* name) noexcept
{
	if (ImGui::Begin(name))
	{
		//ImGui::Text("Position");
		//ImGui::SliderFloat("X", &pos.x, -80.0f, 80.0f, "%.1f");
		//ImGui::SliderFloat("Y", &pos.y, -80.0f, 80.0f, "%.1f");
		//ImGui::SliderFloat("Z", &pos.z, -80.0f, 80.0f, "%.1f");
		ImGui::Text("Orientation");
		ImGui::SliderAngle("Pitch", &pitch, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &yaw, -180.0f, 180.0f);
		ImGui::SliderAngle("Roll", &roll, -180.0f, 180.0f);

		class Probe : public TechniqueProbe
		{
		public:
			void OnSetTechnique() override
			{
				using namespace std::string_literals;
				ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, pTech->GetName().c_str());
				bool active = pTech->IsActive();
				ImGui::Checkbox(("Tech Active##"s + std::to_string(techIdx)).c_str(), &active);
				pTech->SetActiveState(active);
			}
			bool OnVisitBuffer(Dcb::Buffer& buf) override
			{
				namespace dx = DirectX;
				float dirty = false;
				const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };
				auto tag = [tagScratch = std::string{}, tagString = "##" + std::to_string(bufIdx)]
				(const char* label) mutable
				{
					tagScratch = label + tagString;
					return tagScratch.c_str();
				};

				//if (auto v = buf["scale"]; v.Exists())
				//{
				//	dcheck(ImGui::SliderFloat(tag("Scale"), &v, 1.0f, 2.0f, "%.3f", 3.5f));
				//}
				if (auto v = buf["color"]; v.Exists())
				{
					dcheck(ImGui::ColorPicker4(tag("Color"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT4&>(v))));
				}

				return dirty;
			}
		} probe;

		Accept(probe);
	}
	ImGui::End();
}
