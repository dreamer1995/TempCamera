#include "SolidArrow.h"
#include "BindableCommon.h"
#include "GraphicsThrowMacros.h"
#include "Vertex.h"
#include "MiscShape.h"
#include "Stencil.h"
#include "DynamicConstant.h"
#include "TechniqueProbe.h"
#include "imgui/imgui.h"
#include "Channels.h"

SolidArrow::SolidArrow(Graphics& gfx, float size)
{
	using namespace Bind;
	namespace dx = DirectX;

	auto model = MiscShape::MakeArrow(1.0f);
	model.Transform(dx::XMMatrixScaling(size, size, size));
	const auto geometryTag = "$dlightarrow." + std::to_string(size);
	pVertices = VertexBuffer::Resolve(gfx, geometryTag, model.vertices);
	pIndices = IndexBuffer::Resolve(gfx, geometryTag, model.indices);
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	{
		Technique solid{ "Arrow",Chan::main,false };
		Step only("lambertian");

		auto pvs = VertexShader::Resolve(gfx, "Solid_VS.cso");
		only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *pvs));
		only.AddBindable(std::move(pvs));

		only.AddBindable(PixelShader::Resolve(gfx, "Solid_PS.cso"));

		Dcb::RawLayout lay;
		lay.Add<Dcb::Float3>("color");
		lay.Add<Dcb::Float>("length");
		lay.Add<Dcb::Bool>("active");
		auto buf = Dcb::Buffer(std::move(lay));
		buf["active"] = solid.IsActive();
		cbuf = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
		only.AddBindable(cbuf);
		only.AddBindable(std::make_shared<TransformCbuf>(gfx));

		only.AddBindable(Rasterizer::Resolve(gfx, false));

		solid.AddStep(std::move(only));
		AddTechnique(std::move(solid));
	}
}

void SolidArrow::SetTransform(DirectX::XMFLOAT3 pos, float pitch, float yaw) noexcept
{
	this->pos = pos;
	this->pitch = pitch;
	this->yaw = yaw;
}

DirectX::XMMATRIX SolidArrow::GetTransformXM() const noexcept
{
	auto buf = cbuf->GetBuffer();
	return DirectX::XMMatrixScaling(1.0f, 1.0f, buf["length"]) *
		DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f) *
		DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
}

void SolidArrow::ChangeArrowColor() noexcept
{
	class Probe : public TechniqueProbe
	{
	public:
		void OnSetTechnique() override
		{
			using namespace std::string_literals;
			ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, pTech->GetName().c_str());
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
			if (auto v = buf["active"]; v.Exists())
			{
				using namespace std::string_literals;
				dcheck(ImGui::Checkbox("Tech Active##", &v));
				pTech->SetActiveState(v);
			}			
			if (auto v = buf["length"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat(tag("Arrow Length"), &v, 0.0f, 5.0, "%.1f"));
			}
			return dirty;
		}
	} probe;

	Accept(probe);
}

void SolidArrow::SetColor(DirectX::XMFLOAT3 diffuseColor) noexcept
{
	auto buf = cbuf->GetBuffer();
	buf["color"] = diffuseColor;
	cbuf->SetBuffer(buf);
}