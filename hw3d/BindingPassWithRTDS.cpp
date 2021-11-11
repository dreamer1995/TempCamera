#include "BindingPassWithRTDS.h"
#include "Bindable.h"
#include "RenderTarget.h"
#include "DepthStencil.h"
#include "RenderGraphCompileException.h"


namespace Rgph
{
	BindingPassWithRTDS::BindingPassWithRTDS(std::string name, std::vector<std::shared_ptr<Bind::Bindable>> binds)
		:
		BindingPass(name, binds)
	{}

	void BindingPassWithRTDS::BindAll(Graphics& gfx) const noxnd
	{
		BindBufferResources(gfx);
		BindingPass::BindAll(gfx);
	}

	void BindingPassWithRTDS::Finalize()
	{
		BindingPass::Finalize();
		if (!renderTarget && !depthStencil)
		{
			throw RGC_EXCEPTION("BindingPass [" + GetName() + "] needs at least one of a renderTarget or depthStencil");
		}
	}

	void BindingPassWithRTDS::BindBufferResources(Graphics& gfx) const noxnd
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