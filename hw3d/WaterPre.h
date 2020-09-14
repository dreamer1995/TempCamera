#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include <vector>
#include "Sink.h"
#include "Source.h"
#include "Camera.h"
#include "ConstantBuffers.h"
#include "BindableCommon.h"
#include "Cube.h"

using namespace Bind;

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
			pVcbuf = std::make_unique<VertexConstantBuffer<Transforms>>(gfx, 0u);
			auto model = Cube::MakeIndependentSimple();
			AddBind(VertexBuffer::Resolve(gfx, "$waterpre", model.vertices));
			AddBind(IndexBuffer::Resolve(gfx, "$waterpre", model.indices));

			auto vs = VertexShader::Resolve(gfx, "CausticBakeNVS.cso");
			AddBind(InputLayout::Resolve(gfx, model.vertices.GetLayout(), *vs));
			AddBind(std::move(vs));

			AddBind(Texture::Resolve(gfx, "Images\\T_MediumWaves_H.jpg"));
			AddBind(Texture::Resolve(gfx, "Images\\T_MediumWaves_N.jpg", 1u));
			AddBind(Texture::Resolve(gfx, "Images\\T_SmallWaves_N.jpg", 2u));

			AddBind(Topology::Resolve(gfx));
			AddBind(PixelShader::Resolve(gfx, "SphereToCubePS.cso"));
			AddBind(Sampler::Resolve(gfx, Sampler::Type::Bilinear));
			AddBind(Stencil::Resolve(gfx, Stencil::Mode::Off));
			AddBind(Bind::Rasterizer::Resolve(gfx, false));

			renderTarget = std::make_shared<ShaderInputRenderTarget>(gfx, fullWidth, fullWidth, 3u);
			RegisterSource(DirectBindableSource<RenderTarget>::Make("waterPreOut", renderTarget));
		}

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
		std::unique_ptr<VertexConstantBuffer<Transforms>> pVcbuf;
	};
}