#pragma once
#include "Bindable.h"
#include <array>
#include <optional>

namespace Bind
{
	class Blender : public Bindable
	{
	public:
		enum BlendMode
		{
			OneMinus,
			Additive
		};
		Blender(Graphics& gfx, bool blending, BlendMode blendMode = BlendMode::OneMinus, std::optional<float> factor = {});
		void Bind( Graphics& gfx ) noxnd override;
		void SetFactor( float factor ) noxnd;
		float GetFactor() const noxnd;
		static std::shared_ptr<Blender> Resolve(Graphics& gfx, bool blending, BlendMode blendMode = BlendMode::OneMinus, std::optional<float> factor = {});
		static std::string GenerateUID(bool blending, BlendMode blendMode, std::optional<float> factor);
		std::string GetUID() const noexcept override;
	protected:
		Microsoft::WRL::ComPtr<ID3D11BlendState> pBlender;
		bool blending;
		std::optional<std::array<float,4>> factors;
		BlendMode blendMode;
	};
}
