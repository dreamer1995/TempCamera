#include "SolidArrow.h"
#include "BindableCommon.h"
#include "GraphicsThrowMacros.h"
#include "Vertex.h"
#include "MiscShape.h"
#include "Stencil.h"
#include "DynamicConstant.h"
#include "TechniqueProbe.h"
#include "ConstantBuffersEx.h"
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
		Technique solid{ Chan::main };
		Step only("lambertian");

		auto pvs = VertexShader::Resolve(gfx, "Solid_VS.cso");
		only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *pvs));
		only.AddBindable(std::move(pvs));

		only.AddBindable(PixelShader::Resolve(gfx, "Solid_PS.cso"));

		Dcb::RawLayout lay;
		lay.Add<Dcb::Float3>("Color");
		auto buf = Dcb::Buffer(std::move(lay));
		buf["Color"] = DirectX::XMFLOAT3{ 1.0f,0.0f,0.0f };
		only.AddBindable(std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 4u));

		only.AddBindable(std::make_shared<TransformCbuf>(gfx));

		only.AddBindable(Rasterizer::Resolve(gfx, false));

		solid.AddStep(std::move(only));
		AddTechnique(std::move(solid));
	}
}

void SolidArrow::SetTransform(DirectX::XMFLOAT3 pos, float pitch, float yaw, float roll, float length) noexcept
{
	this->pos = pos;
	this->pitch = pitch;
	this->yaw = yaw;
	this->roll = roll;
	this->length = length;
}

DirectX::XMMATRIX SolidArrow::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixScaling(1.0f, length, 1.0f) *
		DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll) *
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

			if (auto v = buf["Color"]; v.Exists())
			{
				dcheck(ImGui::ColorPicker3(tag("ArrowColor"), reinterpret_cast<float*>(&static_cast<dx::XMFLOAT3&>(v))));
			}

			return dirty;
		}
	} probe;

	Accept(probe);
}