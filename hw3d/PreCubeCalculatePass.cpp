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
		auto model = Cube::MakeIndependentSimple();
		AddBind(Bind::VertexBuffer::Resolve(gfx, "$preskybox", model.vertices));
		AddBind(Bind::IndexBuffer::Resolve(gfx, "$preskybox", model.indices));
		// setup other common fullscreen bindables
		auto vs = Bind::VertexShader::Resolve(gfx, "SkyBoxVS.cso");
		AddBind(Bind::InputLayout::Resolve(gfx, model.vertices.GetLayout(), *vs));
		AddBind(std::move(vs));
		AddBind(Bind::Topology::Resolve(gfx));
		AddBind(Bind::Rasterizer::Resolve(gfx, true));
	}

	void PreCubeCalculatePass::Execute(Graphics& gfx) const noxnd
	{
		BindAll(gfx);
		gfx.DrawIndexed(36u);
	}
}
