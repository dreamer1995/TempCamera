#pragma once
#include "LambertianPass.h"
//#include "Job.h"
//#include <vector>
#include "Sink.h"
//#include "Source.h"

class Graphics;

namespace Rgph
{
	class LambertianPass_Water : public LambertianPass
	{
	public:
		LambertianPass_Water(Graphics& gfx, std::string name)
			:
			LambertianPass(gfx, name)
		{
			using namespace Bind;
			AddBindSink<Bind::Bindable>("waterMap");
		}
	};
}