#include "BindingPassNoRTDS.h"
#include "Bindable.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
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
		if (!renderTarget && !depthStencil)
		{
			throw RGC_EXCEPTION("BindingPass [" + GetName() + "] needs at least one of a renderTarget or depthStencil");
		}
	}

	void BindingPassNoRTDS::BindBufferResources(Graphics& gfx) const noxnd
	{
		if (renderTarget)
		{
			renderTarget->BindAsBuffer(gfx, depthStencil.get());
		}
		else
		{
			depthStencil->BindAsBuffer(gfx);
		}
	}
}