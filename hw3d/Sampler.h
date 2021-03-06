#pragma once
#include "Bindable.h"

namespace Bind
{
	class Sampler : public Bindable
	{
	public:
		enum class Filter
		{
			Anisotropic,
			Bilinear,
			Point
		};
		enum class Address
		{
			Clamp,
			Wrap,
			Mirror
		};
	public:
		Sampler(Graphics& gfx, Filter filter, Address address, UINT slot, UINT shaderIndex, float LODRange);
		void Bind( Graphics& gfx ) noxnd override;
		static std::shared_ptr<Sampler> Resolve(Graphics& gfx, Filter filter = Filter::Anisotropic, Address address = Address::Wrap,
			UINT slot = 0u, UINT shaderIndex = 0b1u, float LODRange = 3.402823466e+38F);
		static std::string GenerateUID(Filter filter, Address address, UINT slot, UINT shaderIndex, float LODRange);
		std::string GetUID() const noexcept override;
	protected:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
		Filter filter;
		Address address;
		UINT shaderIndex;
		float LODRange;
	private:
		UINT slot;
	};
}
