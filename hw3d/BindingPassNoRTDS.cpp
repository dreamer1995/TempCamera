#include "BindingPassNoRTDS.h"
#include "Bindable.h"
#include "UnorderedAccessView.h"
#include "RenderGraphCompileException.h"


namespace Rgph
{
	BindingPassNoRTDS::BindingPassNoRTDS(std::string name, std::vector<std::shared_ptr<Bind::Bindable>> binds)
		:
		BindingPass(name, binds)
	{}

	void BindingPassNoRTDS::BindAll(Graphics& gfx) const noxnd
	{
		BindBufferResources(gfx);
		BindingPass::BindAll(gfx);
	}

	void BindingPassNoRTDS::Finalize()
	{
		BindingPass::Finalize();
		if (!unorderedAccessView)
		{
			throw RGC_EXCEPTION("BindingPass [" + GetName() + "] needs at least one of a unorderedAccessView");
		}
	}

	void BindingPassNoRTDS::BindBufferResources(Graphics& gfx) const noxnd
	{
		unorderedAccessView->Bind(gfx);
	}
}