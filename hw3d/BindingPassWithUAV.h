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
	class BindingPassWithUAV : public BindingPass
	{
	protected:
		BindingPassWithUAV(std::string name, std::vector<std::shared_ptr<Bind::Bindable>> binds = {});
		void BindAll(Graphics& gfx) const noxnd;
		void Finalize() override;
	protected:
		std::shared_ptr<Bind::UnorderedAccessView> unorderedAccessView;
	private:
		void BindBufferResources(Graphics& gfx) const noxnd;
	};
}