#pragma once
#include "Bindable.h"

class Surface;

namespace Bind
{
	class Texture : public Bindable
	{
	public:
		enum class Type
		{
			Vertex,
			Hull,
			Domin,
			Pixel
		};
		Texture(Graphics& gfx, const std::string& path, UINT slot = 0, Type type = Type::Pixel);
		Texture(Graphics& gfx, const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureViewIn, UINT slot = 0);
		void Bind( Graphics& gfx ) noxnd override;
		static std::shared_ptr<Texture> Resolve(Graphics& gfx, const std::string& path, UINT slot = 0, Type type = Type::Pixel);
		static std::string GenerateUID(const std::string& path, UINT slot = 0, Type type = Type::Pixel);
		std::string GetUID() const noexcept override;
		bool HasAlpha() const noexcept;
	private:
		static UINT CalculateNumberOfMipLevels( UINT width,UINT height ) noexcept;
	private:
		unsigned int slot;
	protected:
		bool hasAlpha = false;
		std::string path;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		Type type;
	};
}
