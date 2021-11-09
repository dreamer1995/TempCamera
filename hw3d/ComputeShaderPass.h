#pragma once
#include "BindingPass.h"

namespace Bind
{
	class IndexBuffer;
	class VertexBuffer;
	class VertexShader;
	class InputLayout;
}

namespace Rgph
{
	class ComputeShaderPass : public BindingPass
	{
	public:
		ComputeShaderPass(const std::string name, Graphics& gfx) noxnd;
		void Execute(Graphics& gfx) const noxnd override;
	};
}
