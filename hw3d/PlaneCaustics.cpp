#include "PlaneCaustics.h"
#include "Plane.h"
#include "BindableCommon.h"
#include "ConstantBuffersEx.h"
#include "imgui/imgui.h"
#include "DynamicConstant.h"
#include "TechniqueProbe.h"
#include "TransformCbuf.h"
#include "Channels.h"

PlaneCaustics::PlaneCaustics(Graphics& gfx, float size)
{
	using namespace Bind;
	namespace dx = DirectX;

	auto model = Plane::Make(Plane::Type::TessellatedQuad, 128);
	model.Transform(dx::XMMatrixScaling(size, size, 1.0f));
	const auto geometryTag = "$waterCaustics." + std::to_string(size);
	pVertices = VertexBuffer::Resolve(gfx, geometryTag, model.vertices);
	pIndices = IndexBuffer::Resolve(gfx, geometryTag, model.indices);
	pTopology = Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST);

	auto tcb = std::make_shared<TransformCbuf>(gfx);

	{
		Technique shade("Shade", Chan::main);
		{
			Step only("waterCaustic");

			auto vs = VertexShader::Resolve(gfx, "CausticVS.cso");
			only.AddBindable(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *vs));
			only.AddBindable(std::move(vs));

			only.AddBindable(Texture::Resolve(gfx, "Images\\white.jpg", 10u, Texture::Type::Domin));

			only.AddBindable(Topology::Resolve(gfx));
			only.AddBindable(PixelShader::Resolve(gfx, "CausticBakeNPS.cso"));
			only.AddBindable(Sampler::Resolve(gfx, Sampler::Type::Bilinear));

			only.AddBindable(std::move(tcb));

			shade.AddStep(std::move(only));
		}
		AddTechnique(std::move(shade));		
	}
}

DirectX::XMMATRIX PlaneCaustics::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixRotationRollPitchYaw(0.0f, 0.0f, 0.0f);
}