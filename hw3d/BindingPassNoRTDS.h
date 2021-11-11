#pragma once
#include "Pass.h"
#include "Sink.h"
#include "BindingPass.h"

namespace Bind
{
	class Bindable;
}

namespace Rgph
{
	class BindingPassNoRTDS : public BindingPass
	{
	protected:
		BindingPassNoRTDS(std::string name, std::vector<std::shared_ptr<Bind::Bindable>> binds = {});
		void BindAll(Graphics& gfx) const noxnd;
		void Finalize() override;
	protected:
		std::shared_ptr<Bind::RenderTarget> renderTarget;
		std::shared_ptr<Bind::DepthStencil> depthStencil;
	private:
		void BindBufferResources(Graphics& gfx) const noxnd;
	};
}