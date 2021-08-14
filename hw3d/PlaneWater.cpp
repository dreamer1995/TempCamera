#include "PlaneWater.h"
#include "Plane.h"
#include "BindableCommon.h"
//#include "TransformCbufDoubleboi.h"
#include "imgui/imgui.h"
#include "DynamicConstant.h"
#include "TechniqueProbe.h"
#include "TransformCbufScaling.h"
#include "Channels.h"

PlaneWater::PlaneWater(Graphics& gfx, float size)
	:
	waterCaustics(gfx, 1.0f)
{
	using namespace Bind;
	namespace dx = DirectX;

	auto model = Plane::Make(Plane::Type::PlaneTexturedTBN, 128);
	model.Transform(dx::XMMatrixScaling(size, size, 1.0f));
	const auto geometryTag = "$water." + std::to_string(size);
	pVertices = VertexBuffer::Resolve(gfx, geometryTag, model.vertices);
	pIndices = IndexBuffer::Resolve(gfx, geometryTag, model.indices);
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto tcb = std::make_shared<TransformCbufScaling>(gfx);

	{
		Technique shade("Shade", Chan::main);
		{
			Step only("water");

			only.AddBindable(Texture::Resolve(gfx, "Images\\T_MediumWaves_H.jpg"));
			only.AddBindable(Texture::Resolve(gfx, "Images\\T_MediumWaves_N.jpg", 1u));
			only.AddBindable(Texture::Resolve(gfx, "Images\\T_SmallWaves_N.jpg", 2u));
			//AddBind(TexturePre::Resolve(gfx, 3u, gfx.GetShaderResourceView('C')));
			only.AddBindable(Texture::Resolve(gfx, "Images\\DesertSand_albedo.jpg", 3u));
			only.AddBindable(Texture::Resolve(gfx, "Images\\white.jpg", 4u, 0b10001));

			only.AddBindable(Sampler::Resolve(gfx));
			only.AddBindable(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 1u));

			auto pvs = VertexShader::Resolve(gfx, "FluidVS.cso");
			only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *pvs));
			only.AddBindable(std::move(pvs));

			only.AddBindable(PixelShader::Resolve(gfx, "FluidPS.cso"));

			{
				Dcb::RawLayout lay;
				lay.Add<Dcb::Float3>("color");
				lay.Add<Dcb::Float3>("attenuation");
				lay.Add<Dcb::Float3>("scatteringKd");
				lay.Add<Dcb::Float>("depth");
				auto buf = Dcb::Buffer(std::move(lay));
				buf["color"] = dx::XMFLOAT3{ 0.45f,0.785f,0.956f };
				buf["attenuation"] = dx::XMFLOAT3{ 5.0f,5.0f,5.0f };
				buf["scatteringKd"] = dx::XMFLOAT3{ 1.0f,1.0f,1.0f };
				buf["depth"] = 2.471f;
				vmc = std::make_shared<Bind::CachingVertexConstantBufferEx>(gfx, buf, 11u);
				only.AddBindable(vmc);
			}
			{
				Dcb::RawLayout lay;
				lay.Add<Dcb::Float>("metallic");
				lay.Add<Dcb::Float>("tilling");
				lay.Add<Dcb::Float>("_depth");
				auto buf = Dcb::Buffer(std::move(lay));
				buf["metallic"] = 0.572f;
				buf["tilling"] = 1.0f;
				buf["_depth"] = 1.0f;
				pmc = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 11u);
				only.AddBindable(pmc);
			}

			only.AddBindable(Rasterizer::Resolve(gfx, true));

			only.AddBindable(tcb);

			shade.AddStep(std::move(only));
		}
		AddTechnique(std::move(shade));
	}

	Technique preNormal("PreNormal", Chan::waterPre);
	{
		Step only("waterPre");

		auto vs = VertexShader::Resolve(gfx, "CausticBakeNVS.cso");
		only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *vs));
		only.AddBindable(std::move(vs));

		only.AddBindable(Texture::Resolve(gfx, "Images\\T_MediumWaves_H.jpg"));
		only.AddBindable(Texture::Resolve(gfx, "Images\\T_MediumWaves_N.jpg", 1u));
		only.AddBindable(Texture::Resolve(gfx, "Images\\T_SmallWaves_N.jpg", 2u));

		only.AddBindable(PixelShader::Resolve(gfx, "CausticBakeNPS.cso"));
		only.AddBindable(Sampler::Resolve(gfx));

		only.AddBindable(std::move(tcb));

		preNormal.AddStep(std::move(only));
	}
	AddTechnique(std::move(preNormal));
}

void PlaneWater::SetPos(DirectX::XMFLOAT3 pos) noexcept
{
	this->pos = pos;
}

void PlaneWater::SetRotation(float roll, float pitch, float yaw) noexcept
{
	this->roll = roll;
	this->pitch = pitch;
	this->yaw = yaw;
}

DirectX::XMMATRIX PlaneWater::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

void PlaneWater::SpawnControlWindow(Graphics& gfx, const char* name) noexcept
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
					dcheck(ImGui::SliderFloat(tag("Scale"), &v, 0.0f, 2.0f, "%.3f", 1.0f));
				}
				if (auto v = buf["color"]; v.Exists())
				{
					dcheck(ImGui::ColorEdit3(tag("Water Color"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
				}
				if (auto v = buf["attenuation"]; v.Exists())
				{
					dcheck(ImGui::DragFloat3(tag("Attenuation"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
				}
				if (auto v = buf["scatteringKd"]; v.Exists())
				{
					dcheck(ImGui::DragFloat3(tag("ScatteringKd"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
				}
				if (auto v = buf["depth"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Depth"), &v, 0.0f, 4.0f));
				}
				if (auto v = buf["metallic"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Metallic"), &v, 0.0f, 1.0f, "%.3f", 1.0f));
				}
				if (auto v = buf["tilling"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Tilling"), &v, 0.0f, 2.0f));
				}
				return dirty;
			}
		} probe;

		Accept(probe);
	}
	ImGui::End();
	waterCaustics.SpawnControlWindow(gfx, "Caustics");
}

void PlaneWater::SubmitEX(size_t channels1, size_t channels2) const
{
	Submit(channels1);

	auto buf = waterCaustics.dmc->GetBuffer();
	auto buf2 = vmc->GetBuffer();
	float f = buf2["depth"];
	buf["depth"] = f;
	waterCaustics.dmc->SetBuffer(buf);
	auto buf3 = pmc->GetBuffer();
	buf3["_depth"] = f;
	pmc->SetBuffer(buf3);

	waterCaustics.Submit(channels2);
	Submit(channels2);
}
void PlaneWater::LinkTechniquesEX(Rgph::RenderGraph& rg)
{
	LinkTechniques(rg);
	waterCaustics.LinkTechniques(rg);
}