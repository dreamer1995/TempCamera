#include "BindingPassWithUAV.h"
#include "Bindable.h"
#include "UnorderedAccessView.h"
#include "RenderGraphCompileException.h"


namespace Rgph
{
	BindingPassWithUAV::BindingPassWithUAV(std::string name, std::vector<std::shared_ptr<Bind::Bindable>> binds)
		:
		BindingPass(name, binds)
	{}

	void BindingPassWithUAV::BindAll(Graphics& gfx) const noxnd
	{
		BindBufferResources(gfx);
		BindingPass::BindAll(gfx);
	}

	void BindingPassWithUAV::Finalize()
	{
		BindingPass::Finalize();
		if (!unorderedAccessView)
		{
			throw RGC_EXCEPTION("BindingPass [" + GetName() + "] needs at least one of a unorderedAccessView");
		}
	}

	void BindingPassWithUAV::BindBufferResources(Graphics& gfx) const noxnd
	{
		unorderedAccessView->BindAsBuffer(gfx);
	}
}