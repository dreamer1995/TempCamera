#pragma once
#include "TransformCbuf.h"
#include "DynamicConstant.h"

namespace Bind
{
	class TransformCbufScaling : public TransformCbuf
	{
	public:
		TransformCbufScaling(Graphics& gfx, float scale = 1.0f, UINT otherShaderIndex = 0b0u);
		void Accept( TechniqueProbe& probe ) override;
		void Bind( Graphics& gfx ) noxnd override;
		std::unique_ptr<CloningBindable> Clone() const noexcept override;
	private:
		static Dcb::RawLayout MakeLayout();
	private:
		Dcb::Buffer buf;
	};
}