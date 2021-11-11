#pragma once
#include "BindingPassWithRTDS.h"
#include "Job.h"
#include <vector>

namespace Rgph
{
	class RenderQueuePass : public BindingPassWithRTDS
	{
	public:
		using BindingPassWithRTDS::BindingPassWithRTDS;
		void Accept( Job job ) noexcept;
		void Execute( Graphics& gfx ) const noxnd override;
		void Reset() noxnd override;
	private:
		std::vector<Job> jobs;
	};
}