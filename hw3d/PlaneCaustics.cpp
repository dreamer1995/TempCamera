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
	using namespace Bind;
	namespace dx = DirectX;

	auto model = Plane::Make(Plane::Type::TessellatedQuadTextured, 128);
	model.Transform(dx::XMMatrixScaling(size, size, 1.0f));
	const auto geometryTag = "$waterCaustics." + std::to_string(size);
	pVertices = VertexBuffer::Resolve(gfx, geometryTag, model.vertices);
	pIndices = IndexBuffer::Resolve(gfx, geometryTag, model.indices);
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	auto tcb = std::make_shared<TransformCbufScaling>(gfx, 1.0f, 0b100u);

	{
		Technique shade("Shade", Chan::main);
		{
			Step only("waterCaustic");

			auto vs = VertexShader::Resolve(gfx, "CausticVS.cso");
			only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *vs));
			only.AddBindable(std::move(vs));

			only.AddBindable(Texture::Resolve(gfx, "Images\\white.jpg", 0u, 0b100u));

			only.AddBindable(HullShader::Resolve(gfx, "CausticHS.cso"));
			only.AddBindable(DomainShader::Resolve(gfx, "CausticDS.cso"));
			only.AddBindable(PixelShader::Resolve(gfx, "CausticPS.cso"));

			{
				Dcb::RawLayout lay;
				lay.Add<Dcb::Float4>("color");
				auto buf = Dcb::Buffer(std::move(lay));
				buf["color"] = dx::XMFLOAT4{ 0.0132813f,0.0132813f,0.0132813f,1.0f };
				only.AddBindable(std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u));
			}
			
			{
				Dcb::RawLayout lay;
				lay.Add<Dcb::Integer>("tessellation");
				auto buf = Dcb::Buffer(std::move(lay));
				buf["tessellation"] = 6;
				only.AddBindable(std::make_shared<Bind::CachingHullConstantBufferEx>(gfx, buf, 10u));
			}

			{
				Dcb::RawLayout lay;
				lay.Add<Dcb::Float>("depth");
				auto buf = Dcb::Buffer(std::move(lay));
				buf["depth"] = 2.471f;
				dmc = std::make_shared<Bind::CachingDomainConstantBufferEx>(gfx, buf, 11u);
				only.AddBindable(dmc);
			}
			
			only.AddBindable(Sampler::Resolve(gfx, Sampler::Filter::Bilinear, Sampler::Address::Wrap, 0u, 0b100));

			only.AddBindable(std::move(tcb));

			shade.AddStep(std::move(only));
		}
		AddTechnique(std::move(shade));		
	}
}

DirectX::XMMATRIX PlaneCaustics::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(PI / 2.0f, 0.0f, 0.0f);
}

void PlaneCaustics::SpawnControlWindow(Graphics& gfx, const char* name) noexcept
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
					dcheck(ImGui::SliderFloat(tag("Scale"), &v, 0.0f, 2.0f, "%.3f", 1.0f));
				}
				if (auto v = buf["color"]; v.Exists())
				{
					dcheck(ImGui::ColorPicker4(tag("CausticBrightness"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT4&>(v))));
				}
				if (auto v = buf["tessellation"]; v.Exists())
				{
					dcheck(ImGui::SliderInt(tag("Tessellation"), &v, 0, 16));
				}
				return dirty;
			}
		} probe;

		Accept(probe);
	}
	ImGui::End();
}