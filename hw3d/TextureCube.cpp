#include "TextureCube.h"
#include "Surface.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"
#include <vector>

namespace Bind
{
	namespace wrl = Microsoft::WRL;

	TextureCube::TextureCube(Graphics& gfx, const std::string& path, UINT slot, bool manuallyGenerateMips)
		:
		path( path ),
		slot( slot ),
		manuallyGenerateMips(manuallyGenerateMips)
	{
		INFOMAN( gfx );

		if (!manuallyGenerateMips)
		{
			// load 6 surfaces for cube faces
			std::vector<Surface> surfaces;
			for( int i = 0; i < 6; i++ )
			{
				surfaces.push_back(Surface::FromFile(path + "#" + std::to_string(i) + ".jpg"));
			}

			// texture descriptor
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = surfaces[0].GetWidth();
			textureDesc.Height = surfaces[0].GetHeight();
			textureDesc.MipLevels = 1;
			textureDesc.ArraySize = 6;
			textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
			// subresource data
			D3D11_SUBRESOURCE_DATA data[6];
			for( int i = 0; i < 6; i++ )
			{
				data[i].pSysMem = surfaces[i].GetBufferPtrConst();
				data[i].SysMemPitch = surfaces[i].GetBytePitch();
				data[i].SysMemSlicePitch = 0;
			}
			// create the texture resource
			wrl::ComPtr<ID3D11Texture2D> pTexture;
			GFX_THROW_INFO( GetDevice( gfx )->CreateTexture2D(
				&textureDesc,data,&pTexture
			) );

			// create the resource view on the texture
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			GFX_THROW_INFO( GetDevice( gfx )->CreateShaderResourceView(
				pTexture.Get(),&srvDesc,&pTextureView
			) );
		}
		else
		{
			// load 6 surfaces for cube faces
			std::vector<Surface> surfaces;

			for (unsigned int i = 0; i < 6; i++)
			{
				for (unsigned int j = 0; j < 5; j++)
				{
				surfaces.push_back(Surface::FromFile(path + "#" + std::to_string(i) + "#" + std::to_string(j) + ".jpg"));
				}
			}

			// texture descriptor
			D3D11_TEXTURE2D_DESC textureDesc = {};
			textureDesc.Width = surfaces[0].GetWidth();
			textureDesc.Height = surfaces[0].GetHeight();
			textureDesc.MipLevels = 5;
			textureDesc.ArraySize = 6;
			textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
			textureDesc.SampleDesc.Count = 1;
			textureDesc.SampleDesc.Quality = 0;
			textureDesc.Usage = D3D11_USAGE_DEFAULT;
			textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			textureDesc.CPUAccessFlags = 0;
			textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;;
			// subresource data
			D3D11_SUBRESOURCE_DATA data[30];
			for (int i = 0; i < 6; i++)
			{
				for (unsigned int j = 0; j < 5; j++)
				{
					data[j + i * 5].pSysMem = surfaces[j + i * 5].GetBufferPtrConst();
					data[j + i * 5].SysMemPitch = surfaces[j + i * 5].GetBytePitch();
					data[j + i * 5].SysMemSlicePitch = 0;
				}
			}
			// create the texture resource
			wrl::ComPtr<ID3D11Texture2D> pTexture;
			GFX_THROW_INFO( GetDevice( gfx )->CreateTexture2D(
				&textureDesc,data,&pTexture
			) );

			// create the resource view on the texture
			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
			srvDesc.Format = textureDesc.Format;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 5;
			GFX_THROW_INFO( GetDevice( gfx )->CreateShaderResourceView(
				pTexture.Get(),&srvDesc,&pTextureView
			) );
		}		
	}

	void TextureCube::Bind( Graphics& gfx ) noxnd
	{
		INFOMAN_NOHR( gfx );
		GFX_THROW_INFO_ONLY( GetContext( gfx )->PSSetShaderResources( slot,1u,pTextureView.GetAddressOf() ) );
	}

	std::shared_ptr<TextureCube> TextureCube::Resolve(Graphics& gfx, const std::string& path, UINT slot, bool manuallyGenerateMips)
	{
		return Codex::Resolve<TextureCube>(gfx, path, slot, manuallyGenerateMips);
	}
	std::string TextureCube::GenerateUID(const std::string& path, UINT slot, bool manuallyGenerateMips)
	{
		using namespace std::string_literals;
		return typeid(TextureCube).name() + "#"s + path + "#" + std::to_string(slot) + std::to_string(manuallyGenerateMips);
	}
	std::string TextureCube::GetUID() const noexcept
	{
		return GenerateUID(path, slot, manuallyGenerateMips);
	}
}