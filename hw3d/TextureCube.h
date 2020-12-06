#pragma once
#include "Bindable.h"

class Surface;

namespace Bind
{
	class TextureCube : public Bindable
	{
	public:
		TextureCube(Graphics& gfx, const std::string& path, UINT slot, bool manuallyGenerateMips);
		void Bind( Graphics& gfx ) noxnd override;
		static std::shared_ptr<TextureCube> Resolve(Graphics& gfx, const std::string& path, UINT slot = 0, bool manuallyGenerateMips = false);
		static std::string GenerateUID(const std::string& path, UINT slot, bool manuallyGenerateMips);
		std::string GetUID() const noexcept override;
	private:
		unsigned int slot;
	protected:
		std::string path;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		bool manuallyGenerateMips;
	};
}