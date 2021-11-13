#pragma once
#include "BindingPassWithUAV.h"

namespace Bind
{
	
}

namespace Rgph
{
	class ComputeShaderPass : public BindingPassWithUAV
	{
	public:
		ComputeShaderPass(const std::string name, Graphics& gfx) noxnd;
		void Execute(Graphics& gfx) const noxnd override;
		void SetDispatchVector(UINT x, UINT y, UINT group) const noxnd;

	private:
		UINT dataX;
		UINT dataY;
		UINT dataGroup;
	};
}
