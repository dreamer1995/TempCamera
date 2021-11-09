#pragma once
#include "Bindable.h"
namespace Bind
{
	template <typename T>
	class UnorderedAccessView : public Bindable
	{
	public:
		struct DataPerCell
		{
			T r;
			T g;
			T b;
			T a;
		};

		UnorderedAccessView(Graphics& gfx, T& dataType, UINT width, UINT height, UINT slot, UINT shaderIndex);
		void Bind(Graphics& gfx) noxnd override;
		static std::shared_ptr<UnorderedAccessView> Resolve(Graphics& gfx, T& dataType, UINT width, UINT height, UINT slot = 0, UINT shaderIndex = 0b1u);
		static std::string GenerateUID(UINT slot, UINT shaderIndex);
		std::string GetUID() const noexcept override;
	private:
		unsigned int slot;
		UINT shaderIndex;
	protected:
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUnorderedAccessView;
	};
}
