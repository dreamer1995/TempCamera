#include "TextureCube.h"
#include "Surface.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"
#include <vector>
#include "DepthStencil.h"
#include "RenderTarget.h"

namespace Bind
{
	namespace wrl = Microsoft::WRL;

	TextureCube::TextureCube(Graphics& gfx, const std::string& path, UINT slot, bool manuallyGenerateMips, UINT shaderIndex)
		:
		path( path ),
		slot( slot ),
		manuallyGenerateMips(manuallyGenerateMips),
		shaderIndex(shaderIndex)
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
		assert(shaderIndex & 0b00111111);
		INFOMAN_NOHR( gfx );
		if (shaderIndex & 0b00010000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->VSSetShaderResources(slot, 1u, pTextureView.GetAddressOf()));
		}
		if (shaderIndex & 0b00001000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->HSSetShaderResources(slot, 1u, pTextureView.GetAddressOf()));
		}
		if (shaderIndex & 0b00000100)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->DSSetShaderResources(slot, 1u, pTextureView.GetAddressOf()));
		}
		if (shaderIndex & 0b00000010)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->GSSetShaderResources(slot, 1u, pTextureView.GetAddressOf()));
		}
		if (shaderIndex & 0b00000001)
		{
			GFX_THROW_INFO_ONLY( GetContext( gfx )->PSSetShaderResources( slot,1u,pTextureView.GetAddressOf() ) );
		}
		if (shaderIndex & 0b00100000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->CSSetShaderResources(slot, 1u, pTextureView.GetAddressOf()));
		}
	}

	std::shared_ptr<TextureCube> TextureCube::Resolve(Graphics& gfx, const std::string& path, UINT slot, bool manuallyGenerateMips, UINT shaderIndex)
	{
		return Codex::Resolve<TextureCube>(gfx, path, slot, manuallyGenerateMips, shaderIndex);
	}
	std::string TextureCube::GenerateUID(const std::string& path, UINT slot, bool manuallyGenerateMips, UINT shaderIndex)
	{
		using namespace std::string_literals;
		return typeid(TextureCube).name() + "#"s + path + "#" + std::to_string(slot) + std::to_string(manuallyGenerateMips) + std::to_string(shaderIndex);
	}
	std::string TextureCube::GetUID() const noexcept
	{
		return GenerateUID(path, slot, manuallyGenerateMips, shaderIndex);
	}


	CubeTargetTexture::CubeTargetTexture( Graphics& gfx,UINT width,UINT height,UINT slot,DXGI_FORMAT format )
		:
		slot( slot )
	{
		INFOMAN( gfx );

		// texture descriptor
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 6;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		// create the texture resource
		wrl::ComPtr<ID3D11Texture2D> pTexture;
		GFX_THROW_INFO( GetDevice( gfx )->CreateTexture2D(
			&textureDesc,nullptr,&pTexture
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

		// make render target resources for capturing shadow map
		for( UINT face = 0; face < 6; face++ )
		{
			renderTargets.push_back( std::make_shared<OutputOnlyRenderTarget>( gfx,pTexture.Get(),face ) );
		}
	}

	void CubeTargetTexture::Bind( Graphics& gfx ) noxnd
	{
		INFOMAN_NOHR( gfx );
		GFX_THROW_INFO_ONLY( GetContext( gfx )->PSSetShaderResources( slot,1u,pTextureView.GetAddressOf() ) );
	}

	std::shared_ptr<OutputOnlyRenderTarget> Bind::CubeTargetTexture::GetRenderTarget( size_t index ) const
	{
		return renderTargets[index];
	}



	DepthCubeTexture::DepthCubeTexture( Graphics& gfx,UINT size,UINT slot )
		:
		slot( slot )
	{
		INFOMAN( gfx );

		// texture descriptor
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = size;
		textureDesc.Height = size;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 6;
		textureDesc.Format = DXGI_FORMAT::DXGI_FORMAT_R32_TYPELESS;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
		textureDesc.CPUAccessFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		// create the texture resource
		wrl::ComPtr<ID3D11Texture2D> pTexture;
		GFX_THROW_INFO( GetDevice( gfx )->CreateTexture2D(
			&textureDesc,nullptr,&pTexture
		) );

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = 1;
		GFX_THROW_INFO( GetDevice( gfx )->CreateShaderResourceView(
			pTexture.Get(),&srvDesc,&pTextureView
		) );

		// make depth buffer resources for capturing shadow map
		for( UINT face = 0; face < 6; face++ )
		{
			depthBuffers.push_back( std::make_shared<OutputOnlyDepthStencil>( gfx,pTexture,face ) );
		}
	}

	std::shared_ptr<OutputOnlyDepthStencil> Bind::DepthCubeTexture::GetDepthBuffer( size_t index ) const
	{
		return depthBuffers[index];
	}

	void DepthCubeTexture::Bind( Graphics& gfx ) noxnd
	{
		INFOMAN_NOHR( gfx );
		GFX_THROW_INFO_ONLY( GetContext( gfx )->PSSetShaderResources( slot,1u,pTextureView.GetAddressOf() ) );
	}
}