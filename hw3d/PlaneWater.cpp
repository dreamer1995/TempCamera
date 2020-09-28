#include "PlaneWater.h"
#include "Plane.h"
#include "BindableCommon.h"
//#include "TransformCbufDoubleboi.h"
#include "ConstantBuffersEx.h"
#include "imgui/imgui.h"
#include "DynamicConstant.h"
#include "TechniqueProbe.h"
#include "TransformCbufScaling.h"
#include "Channels.h"

PlaneWater::PlaneWater(Graphics& gfx, float size)
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

			//only.AddBindable(Texture::Resolve(gfx, "Images\\T_MediumWaves_H.jpg"));
			//only.AddBindable(Texture::Resolve(gfx, "Images\\T_MediumWaves_N.jpg", 1u));
			//only.AddBindable(Texture::Resolve(gfx, "Images\\T_SmallWaves_N.jpg", 2u));
			////AddBind(TexturePre::Resolve(gfx, 3u, gfx.GetShaderResourceView('C')));
			//only.AddBindable(Texture::Resolve(gfx, "Images\\DesertSand_albedo.jpg", 4u));
			//only.AddBindable(Texture::Resolve(gfx, "Images\\white.jpg", 5u));
			////AddBind(TexturePre::Resolve(gfx, 6u, gfx.GetShaderResourceView('N')));
			////AddBind(Texture::Resolve(gfx, "Images\\white.jpg", 30u, false, true));

			only.AddBindable(Sampler::Resolve(gfx));
			//only.AddBindable(Sampler::Resolve(gfx, Sampler::Type::Clamp, 1u));

			auto pvs = VertexShader::Resolve(gfx, "FluidVS.cso");
			only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *pvs));
			only.AddBindable(std::move(pvs));

			only.AddBindable(PixelShader::Resolve(gfx, "FluidPS.cso"));

			//Dcb::RawLayout lay;
			//lay.Add<Dcb::Float>("roughness");
			//lay.Add<Dcb::Float>("metallic");
			//lay.Add<Dcb::Bool>("normalMappingEnabled");
			//lay.Add<Dcb::Matrix>("EVRotation");
			//lay.Add<Dcb::Float>("depth");
			//auto buf = Dcb::Buffer(std::move(lay));
			//buf["roughness"] = 0.321f;
			//buf["metallic"] = 0.572f;
			//buf["useNormalMap"] = true;
			//dx::XMStoreFloat4x4(&buf["EVRotation"], dx::XMMatrixIdentity());
			//buf["depth"] = 2.471f;
			//cBuf = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
			//only.AddBindable(cBuf);

			only.AddBindable(Rasterizer::Resolve(gfx, true));

			only.AddBindable(tcb);

			shade.AddStep(std::move(only));
		}
		AddTechnique(std::move(shade));
	}
	//AddBind(Texture::Resolve(gfx, "Images\\T_MediumWaves_H.jpg"));
	//AddBind(Texture::Resolve(gfx, "Images\\T_MediumWaves_N.jpg", 1u));
	//AddBind(Texture::Resolve(gfx, "Images\\T_SmallWaves_N.jpg", 2u));
	//AddBind(TexturePre::Resolve(gfx, 3u, gfx.GetShaderResourceView('C')));
	//AddBind(Texture::Resolve(gfx, "Images\\DesertSand_albedo.jpg", 4u));
	//AddBind(Texture::Resolve(gfx, "Images\\white.jpg", 5u));
	//AddBind(TexturePre::Resolve(gfx, 6u, gfx.GetShaderResourceView('N')));
	//AddBind(TexturePre::Resolve(gfx, 10u, gfx.GetShaderResourceView()));
	//AddBind(TexturePre::Resolve(gfx, 11u, gfx.GetShaderResourceView('M')));
	//AddBind(TexturePre::Resolve(gfx, 12u, gfx.GetShaderResourceView('L')));
	//AddBind(Texture::Resolve(gfx, "Images\\white.jpg", 30u, false, true));

	//auto pvs = VertexShader::Resolve(gfx, "FluidVS.cso");
	//auto pvsbc = pvs->GetBytecode();
	//AddBind(std::move(pvs));

	//AddBind(PixelShader::Resolve(gfx, "FluidPS.cso"));

	//AddBind(VertexConstantBuffer<VSMaterialConstant>::Resolve(gfx, vmc, 2u));

	//AddBind(PixelConstantBuffer<PSMaterialConstant>::Resolve(gfx, pmc, 5u));

	//AddBind(InputLayout::Resolve(gfx, model.vertices.GetLayout(), pvsbc));

	//AddBind(Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	//AddBind(std::make_shared<TransformCbufDoubleboi>(gfx, *this, 0u, 4u));

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
		only.AddBindable(Sampler::Resolve(gfx, Sampler::Type::Bilinear));

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
				if (auto v = buf["depth"]; v.Exists())
				{
					dcheck(ImGui::SliderFloat(tag("Depth"), &v, 0.0f, 4.0f, "%.1f"));
				}
				return dirty;
			}
		} probe;

		Accept(probe);
	}
	ImGui::End();
}

void PlaneWater::UpdateENV(float pitch, float yaw, float roll) noexcept
{
	auto k = cBuf->GetBuffer();
	DirectX::XMStoreFloat4x4(&k["EVRotation"], DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
	cBuf->SetBuffer(k);
}