#include "Texture.h"
#include "Surface.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace Bind
{
	namespace wrl = Microsoft::WRL;

	Texture::Texture(Graphics& gfx, const std::string& path, UINT slot, Type type)
		:
		path( path ),
		slot( slot ),
		type(type)
	{
		INFOMAN( gfx );

		// load surface
		const auto s = Surface::FromFile( path );
		hasAlpha = s.AlphaLoaded();

		// create texture resource
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = s.GetWidth();
		textureDesc.Height = s.GetHeight();
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		wrl::ComPtr<ID3D11Texture2D> pTexture;
		GFX_THROW_INFO( GetDevice( gfx )->CreateTexture2D(
			&textureDesc,nullptr,&pTexture
		) );

		// write image data into top mip level
		GetContext( gfx )->UpdateSubresource(
			pTexture.Get(),0u,nullptr,s.GetBufferPtrConst(),s.GetWidth() * sizeof( Surface::Color ),0u
		);

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = textureDesc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;
		GFX_THROW_INFO( GetDevice( gfx )->CreateShaderResourceView(
			pTexture.Get(),&srvDesc,&pTextureView
		) );

		// generate the mip chain using the gpu rendering pipeline
		GetContext( gfx )->GenerateMips( pTextureView.Get() );
	}

	Texture::Texture(Graphics& gfx, const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureViewIn, UINT slot)
		:
		pTextureView(pTextureViewIn)
	{
		GetContext(gfx)->GenerateMips(pTextureView.Get());
	}

	void Texture::Bind( Graphics& gfx ) noxnd
	{
		INFOMAN_NOHR( gfx );
		switch (type)
		{
		case Type::Vertex:
			GFX_THROW_INFO_ONLY(GetContext(gfx)->VSSetShaderResources(slot, 1u, pTextureView.GetAddressOf()));
			break;
		case Type::Hull:
			GFX_THROW_INFO_ONLY(GetContext(gfx)->HSSetShaderResources(slot, 1u, pTextureView.GetAddressOf()));
			break;
		case Type::Domin:
			GFX_THROW_INFO_ONLY(GetContext(gfx)->DSSetShaderResources(slot, 1u, pTextureView.GetAddressOf()));
			break;
		default:
			GFX_THROW_INFO_ONLY(GetContext(gfx)->PSSetShaderResources(slot, 1u, pTextureView.GetAddressOf()));
		}
		
	}
	std::shared_ptr<Texture> Texture::Resolve(Graphics& gfx, const std::string& path, UINT slot, Type type)
	{
		return Codex::Resolve<Texture>(gfx, path, slot, type);
	}
	std::string Texture::GenerateUID(const std::string& path, UINT slot, Type type)
	{
		using namespace std::string_literals;
		return typeid(Texture).name() + "#"s + path + "#" + std::to_string(slot) + std::to_string((int)type);
	}
	std::string Texture::GetUID() const noexcept
	{
		return GenerateUID( path,slot );
	}
	bool Texture::HasAlpha() const noexcept
	{
		return hasAlpha;
	}
	UINT Texture::CalculateNumberOfMipLevels( UINT width,UINT height ) noexcept
	{
		const float xSteps = std::ceil( log2( (float)width ) );
		const float ySteps = std::ceil( log2( (float)height ) );
		return (UINT)std::max( xSteps,ySteps );
	}
}
