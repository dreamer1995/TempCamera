#include "PreCubeCalculatePass.h"
#include "BindableCommon.h"
#include "Cube.h"

namespace Rgph
{
	namespace dx = DirectX;

	PreCubeCalculatePass::PreCubeCalculatePass(const std::string name, Graphics& gfx) noxnd
		:
	BindingPass(std::move(name))
	{
		pVcbuf = std::make_unique<Bind::VertexConstantBuffer<Transforms>>(gfx, 0u);
		auto model = Cube::MakePosOnly();
		AddBind(Bind::VertexBuffer::Resolve(gfx, "$preskybox", std::move(model.vertices)));
		count = (UINT)model.indices.size();
		AddBind(Bind::IndexBuffer::Resolve(gfx, "$preskybox", std::move(model.indices)));
		// setup other common fullscreen bindables
		auto vs = Bind::VertexShader::Resolve(gfx, "SkyBoxVS.cso");
		AddBind(Bind::InputLayout::Resolve(gfx, model.vertices.GetLayout(), *vs));
		AddBind(std::move(vs));
		AddBind(Bind::Topology::Resolve(gfx));
		AddBind(Bind::Rasterizer::Resolve(gfx, true));
	}

	void PreCubeCalculatePass::Execute(Graphics& gfx) const noxnd
	{
		pVcbuf->Update(gfx, { DirectX::XMMatrixTranspose(gfx.GetProjection()) * DirectX::XMMatrixTranspose(gfx.GetCamera()) });
		pVcbuf->Bind(gfx);
		BindAll(gfx);
		gfx.DrawIndexed(count);
	}
}
