#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include <vector>
#include "Sink.h"
#include "Source.h"
#include "Stencil.h"
#include "Camera.h"
#include "DepthStencil.h"
#include "Sampler.h"
#include "Texture.h"
#include "BindingPass.h"
#include "ConstantBuffers.h"
#include "Bindable.h"
#include "Cube.h"

namespace Bind
{
	class IndexBuffer;
	class VertexBuffer;
	class Topology;
	class InputLayout;
}

class Graphics;

namespace Rgph
{
	class WaterPrePass : public RenderQueuePass
	{
	public:
		WaterPrePass(Graphics& gfx, std::string name, unsigned int fullWidth, unsigned int fullHeight)
			:
			RenderQueuePass(std::move(name))
		{
			pVcbuf = std::make_unique<Bind::VertexConstantBuffer<Transforms>>(gfx, 0u);
			auto model = Cube::MakeIndependentSimple();
			AddBind(Bind::VertexBuffer::Resolve(gfx, "$preskybox", model.vertices));
			AddBind(Bind::IndexBuffer::Resolve(gfx, "$preskybox", model.indices));
			auto pvs = Bind::VertexShader::Resolve(gfx, "CausticBakeNVS.cso");
			auto pvsbc = pvs->GetBytecode();
			AddBind(std::move(pvs));

			AddBind(Bind::Texture::Resolve(gfx, "Images\\T_MediumWaves_H.jpg"));
			AddBind(Bind::Texture::Resolve(gfx, "Images\\T_MediumWaves_N.jpg", 1u));
			AddBind(Bind::Texture::Resolve(gfx, "Images\\T_SmallWaves_N.jpg", 2u));

			auto vs = Bind::VertexShader::Resolve(gfx, "SkyBoxVS.cso");
			AddBind(Bind::InputLayout::Resolve(gfx, model.vertices.GetLayout(), *vs));
			AddBind(std::move(vs));
			AddBind(Bind::Topology::Resolve(gfx));
			AddBind(PixelShader::Resolve(gfx, "SphereToCubePS.cso"));
			AddBind(Sampler::Resolve(gfx, Sampler::Type::Bilinear));
			renderTarget = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullWidth, 3u);
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::Off));
			RegisterSource(DirectBindableSource<Bind::RenderTarget>::Make("waterPreOut", renderTarget));
		}
		//void BindMainCamera(const Camera& cam) noexcept
		//{
		//	pMainCamera = &cam;
		//}
		void Execute(Graphics& gfx) const noxnd override
		{
			renderTarget->Clear(gfx);
			//pMainCamera->BindToGraphics(gfx);
			RenderQueuePass::Execute(gfx);
		}
	protected:
		struct Transforms
		{
			DirectX::XMMATRIX matrix_MVP;
		};
	private:
		std::unique_ptr<Bind::VertexConstantBuffer<Transforms>> pVcbuf;
	};
}