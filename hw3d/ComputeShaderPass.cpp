#include "ComputeShaderPass.h"
#include "BindableCommon.h"


namespace Rgph
{
	namespace dx = DirectX;

	ComputeShaderPass::ComputeShaderPass(const std::string name, Graphics& gfx) noxnd
		:
	BindingPassWithUAV(std::move(name))
	{
		dataX = 8u;
		dataY = 8u;
		dataGroup = 1u;
	}

	void ComputeShaderPass::Execute(Graphics& gfx) const noxnd
	{
		BindAll(gfx);
		gfx.Dispatch(dataX, dataY, dataGroup);
	}

	void ComputeShaderPass::SetDispatchVector(UINT x, UINT y, UINT group) const noxnd
	{
		const_cast<ComputeShaderPass*>(this)->dataX = x;
		const_cast<ComputeShaderPass*>(this)->dataY = y;
		const_cast<ComputeShaderPass*>(this)->dataGroup = group;
	}
}
