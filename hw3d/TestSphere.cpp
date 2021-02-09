#include "TestSphere.h"
#include "Sphere.h"
#include "BindableCommon.h"
#include "imgui/imgui.h"
#include "DynamicConstant.h"
#include "TechniqueProbe.h"
#include "TransformCbufScaling.h"
#include "Channels.h"

TestSphere::TestSphere(Graphics& gfx, float size)
{
	using namespace Bind;
	namespace dx = DirectX;

	using Dvtx::VertexLayout;
	VertexLayout vl;
	vl.Append(VertexLayout::Position3D);
	vl.Append(VertexLayout::Normal);
	vl.Append(VertexLayout::Tangent);
	vl.Append(VertexLayout::Binormal);
	vl.Append(VertexLayout::Texture2D);
	auto model = Sphere::MakeNormalUVed(vl, true);
	model.Transform(dx::XMMatrixScaling(size, size, size));
	const auto geometryTag = "$sphere." + std::to_string(size);
	pVertices = VertexBuffer::Resolve(gfx, geometryTag, model.vertices);
	pIndices = IndexBuffer::Resolve(gfx, geometryTag, model.indices);
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto tcb = std::make_shared<TransformCbufScaling>(gfx);

	{
		Technique shade("Shade", Chan::main);
		{
			Step only("lambertian");

			/*only.AddBindable(Texture::Resolve(gfx, "Images\\brickwall.jpg"));
			only.AddBindable(Texture::Resolve(gfx, "Images\\brickwall_normal.jpg", 2u));*/
			only.AddBindable(Sampler::Resolve(gfx));
			only.AddBindable(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 1u));

			auto pvs = VertexShader::Resolve(gfx, "PBRVS.cso");
			only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *pvs));
			only.AddBindable(std::move(pvs));

			only.AddBindable(PixelShader::Resolve(gfx, "PBRPS.cso"));

			Dcb::RawLayout lay;
			lay.Add<Dcb::Float3>("baseColor");
			lay.Add<Dcb::Float>("roughness");
			lay.Add<Dcb::Float>("metallic");
			lay.Add<Dcb::Bool>("useNormalMap");
			lay.Add<Dcb::Float>("normalMapWeight");
			auto buf = Dcb::Buffer(std::move(lay));
			buf["baseColor"] = dx::XMFLOAT3{ 1.0f,1.0f,1.0f };
			buf["roughness"] = 0.05f;
			buf["metallic"] = 0.0f;
			buf["useNormalMap"] = false;
			buf["normalMapWeight"] = 1.0f;
			only.AddBindable(std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u));

			only.AddBindable(Rasterizer::Resolve(gfx, false));

			only.AddBindable(tcb);

			shade.AddStep(std::move(only));
		}
		AddTechnique(std::move(shade));
	}
	{
		Technique shade("gbufferDraw", Chan::gbuffer);
		{
			Step only("gbuffer");

			/*only.AddBindable(Texture::Resolve(gfx, "Images\\brickwall.jpg"));
			only.AddBindable(Texture::Resolve(gfx, "Images\\brickwall_normal.jpg", 2u));*/
			only.AddBindable(Sampler::Resolve(gfx));
			only.AddBindable(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Clamp, 1u));

			auto pvs = VertexShader::Resolve(gfx, "PBRNoShadow_VS.cso");
			only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *pvs));
			only.AddBindable(std::move(pvs));

			only.AddBindable(PixelShader::Resolve(gfx, "PBREncodeToGbuffer.cso"));

			Dcb::RawLayout lay;
			lay.Add<Dcb::Bool>("enableAbedoMap");
			lay.Add<Dcb::Bool>("enableMRAMap");
			lay.Add<Dcb::Bool>("enableNormalMap");
			lay.Add<Dcb::Bool>("useAbedoMap");
			lay.Add<Dcb::Float3>("baseColor");
			lay.Add<Dcb::Bool>("useMetallicMap");
			lay.Add<Dcb::Bool>("useRoughnessMap");
			lay.Add<Dcb::Float>("roughness");
			lay.Add<Dcb::Float>("metallic");
			lay.Add<Dcb::Bool>("useNormalMap");
			lay.Add<Dcb::Float>("normalMapWeight");
			auto buf = Dcb::Buffer(std::move(lay));
			buf["baseColor"] = dx::XMFLOAT3{ 1.0f,1.0f,1.0f };
			buf["roughness"] = 0.05f;
			buf["metallic"] = 0.0f;
			buf["useNormalMap"] = false;
			buf["normalMapWeight"] = 1.0f;
			only.AddBindable(std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u));

			only.AddBindable(Rasterizer::Resolve(gfx, false));

			only.AddBindable(tcb);

			shade.AddStep(std::move(only));
		}
		AddTechnique(std::move(shade));
	}
	{
		Technique outline("Outline", Chan::main, false);
		{
			Step mask("outlineMask");

			// TODO: better sub-layout generation tech for future consideration maybe
			mask.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *VertexShader::Resolve(gfx, "Solid_VS.cso")));

			mask.AddBindable(std::move(tcb));

			// TODO: might need to specify rasterizer when doubled-sided models start being used

			outline.AddStep(std::move(mask));
		}
		{
			Step draw("outlineDraw");

			Dcb::RawLayout lay;
			lay.Add<Dcb::Float4>("color");
			auto buf = Dcb::Buffer(std::move(lay));
			buf["color"] = DirectX::XMFLOAT4{ 1.0f,0.4f,0.4f,1.0f };
			draw.AddBindable(std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u));

			// TODO: better sub-layout generation tech for future consideration maybe
			draw.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *VertexShader::Resolve(gfx, "Solid_VS.cso")));

			draw.AddBindable(std::make_shared<TransformCbufScaling>(gfx));

			// TODO: might need to specify rasterizer when doubled-sided models start being used

			outline.AddStep(std::move(draw));
		}
		AddTechnique(std::move(outline));
	}
	// shadow map technique
	{
		Technique map{ "ShadowMap",Chan::shadow,true };
		{
			Step draw("shadowMap");

			// TODO: better sub-layout generation tech for future consideration maybe
			draw.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *VertexShader::Resolve(gfx, "Solid_VS.cso")));

			draw.AddBindable(std::make_shared<TransformCbuf>(gfx));

			// TODO: might need to specify rasterizer when doubled-sided models start being used

			map.AddStep(std::move(draw));
		}
		AddTechnique(std::move(map));
	}
}

void TestSphere::SetPos(DirectX::XMFLOAT3 pos) noexcept
{
	this->pos = pos;
}

void TestSphere::SetRotation(float roll, float pitch, float yaw) noexcept
{
	this->roll = roll;
	this->pitch = pitch;
	this->yaw = yaw;
}

DirectX::XMMATRIX TestSphere::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

void TestSphere::SpawnControlWindow(Graphics& gfx, const char* name) noexcept
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
					dcheck(ImGui::ColorPicker4(tag("OutLineColor"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT4&>(v))));
				}
				if (auto v = buf["baseColor"]; v.Exists())
				{
					dcheck(ImGui::ColorPicker4(tag("BaseColor"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
				}
				if (auto v = buf["roughness"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Roughness"), &v, 0.0f, 1.0f, "%.3f", 1.0f));
				}
				if (auto v = buf["metallic"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Metallic"), &v, 0.0f, 1.0f, "%.3f", 1.0f));
				}
				if (auto v = buf["useNormalMap"]; v.Exists())
				{
					dcheck(ImGui::Checkbox(tag("Normal Map Enable"), &v));
				}
				if (auto v = buf["normalMapWeight"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Normal Map Weight"), &v, 0.0f, 1.0f, "%.1f"));
				}
				return dirty;
			}
		} probe;

		Accept(probe);
	}
	ImGui::End();
}
