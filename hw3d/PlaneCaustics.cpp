#include "PlaneCaustics.h"
#include "Plane.h"
#include "BindableCommon.h"
#include "ConstantBuffersEx.h"
#include "imgui/imgui.h"
#include "DynamicConstant.h"
#include "TechniqueProbe.h"
#include "TransformCbufScaling.h"
#include "Channels.h"

PlaneCaustics::PlaneCaustics(Graphics& gfx, float size)
{
	//using namespace Bind;
	//namespace dx = DirectX;

	//auto model = Plane::Make(Plane::Type::PlaneTexturedTBN, 128);
	//model.Transform(dx::XMMatrixScaling(size, size, 1.0f));
	//const auto geometryTag = "$waterpre." + std::to_string(size);
	//pVertices = VertexBuffer::Resolve(gfx, geometryTag, model.vertices);
	//pIndices = IndexBuffer::Resolve(gfx, geometryTag, model.indices);
	//pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//auto tcb = std::make_shared<TransformCbufScaling>(gfx);

	//{
	//	Technique shade("Shade", Chan::main);
	//	{
	//		Step only("waterPre");

	//		auto vs = VertexShader::Resolve(gfx, "CausticBakeNVS.cso");
	//		only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *vs));
	//		only.AddBindable(std::move(vs));

	//		only.AddBindable(Texture::Resolve(gfx, "Images\\T_MediumWaves_H.jpg"));
	//		only.AddBindable(Texture::Resolve(gfx, "Images\\T_MediumWaves_N.jpg", 1u));
	//		only.AddBindable(Texture::Resolve(gfx, "Images\\T_SmallWaves_N.jpg", 2u));

	//		only.AddBindable(Topology::Resolve(gfx));
	//		only.AddBindable(PixelShader::Resolve(gfx, "CausticBakeNPS.cso"));
	//		only.AddBindable(Sampler::Resolve(gfx, Sampler::Type::Bilinear));

	//		only.AddBindable(std::move(tcb));

	//		shade.AddStep(std::move(only));
	//	}
	//	AddTechnique(std::move(shade));		
	//}
}

DirectX::XMMATRIX PlaneCaustics::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
}

void PlaneCaustics::SpawnControlWindow(Graphics& gfx,const char* name) noexcept
{
	if (ImGui::Begin(name))
	{

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
				return dirty;
			}
		} probe;

		Accept(probe);
	}
	ImGui::End();
}