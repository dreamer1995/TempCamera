#include "PhongSphere.h"
#include "BindableCommon.h"
#include "GraphicsThrowMacros.h"
#include "Vertex.h"
#include "Sphere.h"
#include "Stencil.h"
#include "DynamicConstant.h"
#include "TechniqueProbe.h"
#include "ConstantBuffersEx.h"
#include "imgui/imgui.h"
#include "Channels.h"

PhongSphere::PhongSphere(Graphics& gfx, float radius)
{
	using namespace Bind;
	namespace dx = DirectX;

	using Dvtx::VertexLayout;
	VertexLayout vl;
	vl.Append(VertexLayout::Position3D);
	vl.Append(VertexLayout::Normal);
	auto model = Sphere::Make(vl, true);
	model.Transform(dx::XMMatrixScaling(radius, radius, radius));
	const auto geometryTag = "$phongsphere." + std::to_string(radius);
	pVertices = VertexBuffer::Resolve(gfx, geometryTag, model.vertices);
	pIndices = IndexBuffer::Resolve(gfx, geometryTag, model.indices);
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	{
		Technique solid{ Chan::main };
		Step only("lambertian");

		auto pvs = VertexShader::Resolve(gfx, "PhongVSDirectional.cso");
		only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *pvs));
		only.AddBindable(std::move(pvs));

		only.AddBindable(PixelShader::Resolve(gfx, "PhongPSDirectional.cso"));

		Dcb::RawLayout lay;
		lay.Add<Dcb::Float3>("color");
		lay.Add<Dcb::Float>("specularWeight");
		lay.Add<Dcb::Float>("specularGloss");
		auto buf = Dcb::Buffer(std::move(lay));
		buf["color"] = DirectX::XMFLOAT3{ 1.0f,1.0f,1.0f };
		buf["specularWeight"] = 0.3f;
		buf["specularGloss"] = 30.0f;
		only.AddBindable(std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u));

		only.AddBindable(std::make_shared<TransformCbuf>(gfx));

		only.AddBindable(Rasterizer::Resolve(gfx, false));

		solid.AddStep(std::move(only));
		AddTechnique(std::move(solid));
	}
}

void PhongSphere::SetPos(DirectX::XMFLOAT3 pos) noexcept
{
	this->pos = pos;
}

DirectX::XMMATRIX PhongSphere::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

void PhongSphere::ChangeSphereMaterialState() noexcept
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

			if (auto v = buf["color"]; v.Exists())
			{
				dcheck(ImGui::ColorPicker3(tag("Color"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
			}
			if (auto v = buf["specularWeight"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat(tag("Spec. Intens."), &v, 0.0f, 1.0f));
			}
			if (auto v = buf["specularGloss"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat(tag("Glossiness"), &v, 1.0f, 100.0f, "%.1f", 1.5f));
			}

			return dirty;
		}
	} probe;

	Accept(probe);
}