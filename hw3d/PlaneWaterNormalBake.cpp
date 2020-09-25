#include "PlaneWaterNormalBake.h"
#include "Plane.h"
#include "BindableCommon.h"
//#include "TransformCbufDoubleboi.h"
#include "ConstantBuffersEx.h"
#include "imgui/imgui.h"
#include "DynamicConstant.h"
#include "TechniqueProbe.h"
#include "TransformCbufScaling.h"
#include "Channels.h"

PlaneWaterNormalBake::PlaneWaterNormalBake(Graphics& gfx, float size)
{
	using namespace Bind;
	namespace dx = DirectX;

	auto model = Plane::Make(Plane::Type::PlaneTexturedTBN, 100);
	model.Transform(dx::XMMatrixScaling(size, size, 1.0f));
	const auto geometryTag = "$waterpre." + std::to_string(size);
	pVertices = VertexBuffer::Resolve(gfx, geometryTag, model.vertices);
	pIndices = IndexBuffer::Resolve(gfx, geometryTag, model.indices);
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto tcb = std::make_shared<TransformCbufScaling>(gfx);

	{
		Technique shade("Shade", Chan::main);
		{
			Step only("waterPre");

			auto vs = VertexShader::Resolve(gfx, "CausticBakeNVS.cso");
			only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *vs));
			only.AddBindable(std::move(vs));

			only.AddBindable(Texture::Resolve(gfx, "Images\\T_MediumWaves_H.jpg"));
			only.AddBindable(Texture::Resolve(gfx, "Images\\T_MediumWaves_N.jpg", 1u));
			only.AddBindable(Texture::Resolve(gfx, "Images\\T_SmallWaves_N.jpg", 2u));

			only.AddBindable(Topology::Resolve(gfx));
			only.AddBindable(PixelShader::Resolve(gfx, "CausticBakeNPS.cso"));
			only.AddBindable(Sampler::Resolve(gfx, Sampler::Type::Bilinear));

			Dcb::RawLayout lay;
			lay.Add<Dcb::Float>("speed");
			lay.Add<Dcb::Float>("roughness");
			lay.Add<Dcb::Float>("flatten1");
			lay.Add<Dcb::Float>("flatten2");
			lay.Add<Dcb::Bool>("normalMappingEnabled");
			auto buf = Dcb::Buffer(std::move(lay));
			buf["speed"] = 0.3f;
			buf["roughness"] = 0.572f;
			buf["flatten1"] = 0.182f;
			buf["flatten2"] = 0.0f;
			buf["normalMappingEnabled"] = true;
			cBuf = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
			only.AddBindable(cBuf);

			only.AddBindable(std::move(tcb));

			shade.AddStep(std::move(only));
		}
		AddTechnique(std::move(shade));		
	}
}

DirectX::XMMATRIX PlaneWaterNormalBake::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

void PlaneWaterNormalBake::SpawnControlWindow(Graphics& gfx,const char* name) noexcept
{
	if (ImGui::Begin(name))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &pos.x, -80.0f, 80.0f, "%.1f");
		ImGui::SliderFloat("Y", &pos.y, -80.0f, 80.0f, "%.1f");
		ImGui::SliderFloat("Z", &pos.z, -80.0f, 80.0f, "%.1f");
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

				if (auto v = buf["scale"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat("Scale", &v, 0.0f, 10.0f, "%.3f", 1.0f));
				}
				if (auto v = buf["speed"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Speed"), &v, 0.0f, 10.0f, "%.3f", 1.0f));
				}
				if (auto v = buf["roughness"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Roughness"), &v, 0.0f, 1.0f, "%.3f", 1.0f));
				}
				if (auto v = buf["flatten1"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Flatten1"), &v, 0.0f, 1.0f, "%.3f", 1.0f));
				}
				if (auto v = buf["flatten2"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Flatten2"), &v, 0.0f, 1.0f, "%.3f", 1.0f));
				}
				if (auto v = buf["normalMappingEnabled"]; v.Exists())
				{
					dcheck(ImGui::Checkbox(tag("Normal Map Enable"), &v));
				}
				return dirty;
			}
		} probe;

		Accept(probe);
	}
	ImGui::End();
}