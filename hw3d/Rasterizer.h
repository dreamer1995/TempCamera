#pragma once
#include "Bindable.h"
#include <array>

namespace Bind
{
	class Rasterizer : public Bindable
	{
	public:
		Rasterizer( Graphics& gfx,bool twoSided );
		void Bind( Graphics& gfx ) noxnd override;
		static std::shared_ptr<Rasterizer> Resolve( Graphics& gfx,bool twoSided );
		static std::string GenerateUID( bool twoSided );
		std::string GetUID() const noexcept override;
		void ChangeFillMode(Graphics& gfx) noxnd;
	protected:
		Microsoft::WRL::ComPtr<ID3D11RasterizerState> pRasterizer;
		bool twoSided;
	};
}
