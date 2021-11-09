#pragma once
#include "Bindable.h"
namespace Bind
{
	class UnorderedAccessView : public Bindable
	{
	public:
		UnorderedAccessView(Graphics& gfx, UINT slot, UINT shaderIndex);
		void Bind(Graphics& gfx) noxnd override;
		static std::shared_ptr<UnorderedAccessView> Resolve(Graphics& gfx, UINT slot = 0, UINT shaderIndex = 0b1u);
		static std::string GenerateUID(UINT slot, UINT shaderIndex);
		std::string GetUID() const noexcept override;
	private:
		unsigned int slot;
		UINT shaderIndex;
	protected:
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUnorderedAccessView;
	};
}
