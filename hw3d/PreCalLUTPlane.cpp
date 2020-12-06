#include "PreCalLUTPlane.h"
#include "Sink.h"
#include "Source.h"
#include "PixelShader.h"
#include "Blender.h"
#include "Stencil.h"
#include "Sampler.h"

using namespace Bind;

namespace Rgph
{
	PreCalLUTPlane::PreCalLUTPlane(std::string name, Graphics& gfx, unsigned int fullWidth, unsigned int fullHeight)
		:
		FullscreenPass(std::move(name), gfx)
	{
		AddBind(PixelShader::Resolve(gfx, "IntegrateBRDFPixelShader.cso"));
		AddBind(Stencil::Resolve(gfx, Stencil::Mode::DepthOff));
		AddBind(Sampler::Resolve(gfx, Sampler::Filter::Bilinear));

		pPreCalLUTPlane = std::make_shared<Bind::ShaderInputRenderTarget>(gfx, fullWidth, fullHeight, 13u, ShaderInputRenderTarget::Type::PreBRDFPlane);
		renderTarget = pPreCalLUTPlane;
	}

	void PreCalLUTPlane::DumpLUTMap(Graphics& gfx, const std::string& path) const
	{
		pPreCalLUTPlane->ToSurface(gfx).Save(path);
	}
}
