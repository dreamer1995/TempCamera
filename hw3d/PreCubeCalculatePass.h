#pragma once
#include "BindingPass.h"
#include "ConstantBuffers.h"

namespace Bind
{
	class IndexBuffer;
	class VertexBuffer;
	class VertexShader;
	class InputLayout;
}

namespace Rgph
{
	class PreCubeCalculatePass : public BindingPass
	{
	public:
		PreCubeCalculatePass(const std::string name, Graphics& gfx) noxnd;
		void Execute(Graphics& gfx) const noxnd override;
	protected:
		struct Transforms
		{
			DirectX::XMMATRIX matrix_MVP;
		};
	private:
		std::unique_ptr<Bind::VertexConstantBuffer<Transforms>> pVcbuf;
	};
}
