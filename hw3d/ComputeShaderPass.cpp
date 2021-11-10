#include "ComputeShaderPass.h"
#include "BindableCommon.h"


namespace Rgph
{
	namespace dx = DirectX;

	ComputeShaderPass::ComputeShaderPass(const std::string name, Graphics& gfx) noxnd
		:
	BindingPass(std::move(name))
	{
		
	}

	void ComputeShaderPass::Execute(Graphics& gfx) const noxnd
	{
		BindAll(gfx);
		gfx.Dispatch(8u, 8u, 1u);
	}
}
