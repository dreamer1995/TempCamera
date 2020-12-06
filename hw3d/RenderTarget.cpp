#include "RenderTarget.h"
#include "GraphicsThrowMacros.h"
#include "DepthStencil.h"
#include "Surface.h"
#include <array>

namespace wrl = Microsoft::WRL;

namespace Bind
{
	RenderTarget::RenderTarget(Graphics& gfx, UINT width, UINT height, Type type)
		:
		width( width ),
		height( height ),
		type(type)
	{
		INFOMAN( gfx );

		// create texture resource
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = width;
		textureDesc.Height = height;
		textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE; // never do we not want to bind offscreen RTs as inputs
		textureDesc.CPUAccessFlags = 0;

		switch (type)
		{
		case Type::PreCalSimpleCube:
		{
			textureDesc.ArraySize = 6;
			textureDesc.MipLevels = 1;
			textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
			break;
		}
		case Type::PreCalMipCube:
		{
			textureDesc.ArraySize = 6;
			textureDesc.MipLevels = 5;
			textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS | D3D11_RESOURCE_MISC_TEXTURECUBE;
			break;
		}
		case Type::PreBRDFPlane:
		{
			textureDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		}
		default:
		{
			textureDesc.ArraySize = 1;
			textureDesc.MipLevels = 1;
			textureDesc.MiscFlags = 0;
		}
		}

		wrl::ComPtr<ID3D11Texture2D> pTexture;
		GFX_THROW_INFO( GetDevice( gfx )->CreateTexture2D(
			&textureDesc,nullptr,&pTexture
		) );

		// create the target view on the texture
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = textureDesc.Format;
		rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };

		switch (type)
		{

		case Type::PreCalMipCube:
			rtvDesc.Texture2DArray.MipSlice = 0;
		case Type::PreCalSimpleCube:
		{
			rtvDesc.Texture2DArray.ArraySize = 1;
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
			for (unsigned char i = 0; i < 6; ++i)
			{
				// Create a render target view to the ith element.
				rtvDesc.Texture2DArray.FirstArraySlice = i;
				GFX_THROW_INFO(GetDevice(gfx)->CreateRenderTargetView(
					pTexture.Get(), &rtvDesc, &pTargetCubeView[i]));
			}
			break;
		}
		case Type::PreBRDFPlane:
		default:
		{
			rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			GFX_THROW_INFO(GetDevice(gfx)->CreateRenderTargetView(
				pTexture.Get(), &rtvDesc, &pTargetView
			));
		}
		}
	}

	RenderTarget::RenderTarget( Graphics& gfx,ID3D11Texture2D* pTexture )
	{
		INFOMAN( gfx );

		// get information from texture about dimensions
		D3D11_TEXTURE2D_DESC textureDesc;
		pTexture->GetDesc( &textureDesc );
		width = textureDesc.Width;
		height = textureDesc.Height;

		// create the target view on the texture
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
		rtvDesc.Format = textureDesc.Format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D = D3D11_TEX2D_RTV{ 0 };
		GFX_THROW_INFO( GetDevice( gfx )->CreateRenderTargetView(
			pTexture,&rtvDesc,&pTargetView
		) );
	}

	void RenderTarget::BindAsBuffer( Graphics& gfx ) noxnd
	{
		ID3D11DepthStencilView* const null = nullptr;
		BindAsBuffer( gfx,null );
	}

	void RenderTarget::BindAsBuffer( Graphics& gfx,BufferResource* depthStencil ) noxnd
	{
		assert( dynamic_cast<DepthStencil*>(depthStencil) != nullptr );
		BindAsBuffer( gfx,static_cast<DepthStencil*>(depthStencil) );
	}

	void RenderTarget::BindAsBuffer( Graphics& gfx,DepthStencil* depthStencil ) noxnd
	{
		BindAsBuffer(gfx,depthStencil ? depthStencil->pDepthStencilView.Get() : nullptr );
	}

	void RenderTarget::BindAsBuffer( Graphics& gfx,ID3D11DepthStencilView* pDepthStencilView ) noxnd
	{
		INFOMAN_NOHR( gfx );
		D3D11_VIEWPORT vp;
		switch (type)
		{
		case Type::PreCalSimpleCube:
		{
			vp.Width = (float)width;
			vp.Height = (float)height;
			GFX_THROW_INFO_ONLY(GetContext(gfx)->OMSetRenderTargets(1, pTargetCubeView[targetIndex].GetAddressOf(), pDepthStencilView));
			break;
		}
		case Type::PreCalMipCube:
		{
			vp.Width = _width;
			vp.Height = _height;
			GFX_THROW_INFO_ONLY(GetContext(gfx)->OMSetRenderTargets(1, pTargetCubeView[targetIndex].GetAddressOf(), pDepthStencilView));
			break;
		}
		case Type::PreBRDFPlane:
		default:
		{
			vp.Width = (float)width;
			vp.Height = (float)height;
			GFX_THROW_INFO_ONLY(GetContext(gfx)->OMSetRenderTargets(1, pTargetView.GetAddressOf(), pDepthStencilView));
		}
		}

		// configure viewport
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0.0f;
		vp.TopLeftY = 0.0f;
		GFX_THROW_INFO_ONLY( GetContext( gfx )->RSSetViewports( 1u,&vp ) );
	}

	void RenderTarget::Clear( Graphics& gfx,const std::array<float,4>& color ) noxnd
	{
		INFOMAN_NOHR( gfx );
		GFX_THROW_INFO_ONLY( GetContext( gfx )->ClearRenderTargetView( pTargetView.Get(),color.data() ) );
	}

	void RenderTarget::Clear( Graphics& gfx ) noxnd
	{
		Clear( gfx,{ 0.0f,0.0f,0.0f,0.0f } );
	}

	UINT RenderTarget::GetWidth() const noexcept
	{
		return width;
	}

	UINT RenderTarget::GetHeight() const noexcept
	{
		return height;
	}

	void RenderTarget::ChangeMipSlice(Graphics& gfx, UINT i) noxnd
	{
		INFOMAN(gfx);
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		pTargetCubeView[0]->GetDesc(&rtvDesc);
		rtvDesc.Texture2DArray.MipSlice = i;
		wrl::ComPtr<ID3D11Resource> pRes;
		pTargetCubeView[0]->GetResource(&pRes);
		for (unsigned char j = 0; j < 6; ++j)
		{
			// Create a render target view to the ith element.
			rtvDesc.Texture2DArray.FirstArraySlice = j;
			GFX_THROW_INFO(GetDevice(gfx)->CreateRenderTargetView(
				pRes.Get(), &rtvDesc, &pTargetCubeView[j]));
		}
	}

	ShaderInputRenderTarget::ShaderInputRenderTarget(Graphics& gfx, UINT width, UINT height, UINT slot, Type type, UINT shaderIndex)
		:
		RenderTarget(gfx, width, height, type),
		slot( slot ),
		shaderIndex(shaderIndex) 
	{
		INFOMAN( gfx );

		// create the resource view on the texture
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		srvDesc.Texture2D.MostDetailedMip = 0;

		switch (type)
		{
		case Type::PreCalSimpleCube:
			srvDesc.TextureCube.MipLevels = 1;
			break;
		case Type::PreCalMipCube:
			srvDesc.TextureCube.MipLevels = 5;
			break;
		default:
			srvDesc.Texture2D.MipLevels = 1;
		}

		wrl::ComPtr<ID3D11Resource> pRes;

		switch (type)
		{
		case Type::PreCalSimpleCube:
		case Type::PreCalMipCube:
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			pTargetCubeView[0]->GetResource(&pRes);
			GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
				pRes.Get(), &srvDesc, &pShaderResourceView
			));
			break;
		}
		case Type::PreBRDFPlane:
			srvDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
		default:
		{
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			pTargetView->GetResource(&pRes);
			GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
				pRes.Get(), &srvDesc, &pShaderResourceView
			));
		}
		}
	}

	Surface Bind::ShaderInputRenderTarget::ToSurface( Graphics& gfx ) const
	{
		INFOMAN( gfx );
		namespace wrl = Microsoft::WRL;

		// creating a temp texture compatible with the source, but with CPU read access
		wrl::ComPtr<ID3D11Resource> pResSource;
		pShaderResourceView->GetResource( &pResSource );
		wrl::ComPtr<ID3D11Texture2D> pTexSource;
		pResSource.As( &pTexSource );
		D3D11_TEXTURE2D_DESC textureDesc;
		pTexSource->GetDesc( &textureDesc );
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDesc.Usage = D3D11_USAGE_STAGING;
		textureDesc.BindFlags = 0;
		wrl::ComPtr<ID3D11Texture2D> pTexTemp;
		GFX_THROW_INFO( GetDevice( gfx )->CreateTexture2D(
			&textureDesc,nullptr,&pTexTemp
		) );

		// copy texture contents
		GFX_THROW_INFO_ONLY( GetContext( gfx )->CopyResource( pTexTemp.Get(),pTexSource.Get() ) );

		// create Surface and copy from temp texture to it
		const auto width = GetWidth();
		const auto height = GetHeight();
		Surface s{ width,height };
		D3D11_MAPPED_SUBRESOURCE msr = {};
		GFX_THROW_INFO( GetContext( gfx )->Map( pTexTemp.Get(),0,D3D11_MAP::D3D11_MAP_READ,0,&msr ) );
		auto pSrcBytes = static_cast<const char*>(msr.pData);
		for( unsigned int y = 0; y < height; y++ )
		{
			auto pSrcRow = reinterpret_cast<const Surface::Color*>(pSrcBytes + msr.RowPitch * size_t( y ));
			for( unsigned int x = 0; x < width; x++ )
			{
				const auto raw = *reinterpret_cast<const float*>(pSrcRow + x);
				if (textureDesc.Format == DXGI_FORMAT::DXGI_FORMAT_R32G32_FLOAT)
				{
					const auto channel = unsigned char(raw * 255.0f);
					s.PutPixel(x, y, { channel,channel,channel });
				}
				else
				{
					s.PutPixel( x,y,*(pSrcRow + x) );
				}
			}
		}
		GFX_THROW_INFO_ONLY( GetContext( gfx )->Unmap( pTexTemp.Get(),0 ) );

		return s;
	}

	void Bind::ShaderInputRenderTarget::ToCube(Graphics& gfx, const std::string& path) const
	{
		INFOMAN(gfx);
		namespace wrl = Microsoft::WRL;

		// creating a temp texture compatible with the source, but with CPU read access
		wrl::ComPtr<ID3D11Resource> pResSource;
		pShaderResourceView->GetResource(&pResSource);
		wrl::ComPtr<ID3D11Texture2D> pTexSource;
		pResSource.As(&pTexSource);
		D3D11_TEXTURE2D_DESC textureDesc;
		pTexSource->GetDesc(&textureDesc);
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDesc.Usage = D3D11_USAGE_STAGING;
		textureDesc.BindFlags = 0;
		wrl::ComPtr<ID3D11Texture2D> pTexTemp;
		GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
			&textureDesc, nullptr, &pTexTemp
		));

		// copy texture contents
		GFX_THROW_INFO_ONLY(GetContext(gfx)->CopyResource(pTexTemp.Get(), pTexSource.Get()));

		// create Surface and copy from temp texture to it
		const auto width = GetWidth();
		const auto height = GetHeight();
		Surface s{ width,height };
		D3D11_MAPPED_SUBRESOURCE msr = {};
		GFX_THROW_INFO(GetContext(gfx)->Map(pTexTemp.Get(), 0, D3D11_MAP::D3D11_MAP_READ, 0, &msr));
		auto pSrcBytes = static_cast<const char*>(msr.pData);
		for (unsigned char z = 0; z < 6; z++)
		{
			for (unsigned int y = 0; y < height; y++)
			{
				auto pSrcRow = reinterpret_cast<const Surface::Color*>(pSrcBytes + msr.RowPitch * size_t(y) + width * msr.RowPitch * z);
				for (unsigned int x = 0; x < width; x++)
				{
					s.PutPixel(x, y, *(pSrcRow + x));
				}
			}
			s.Save(path.c_str(), "#" + std::to_string(z));
		}
		GFX_THROW_INFO_ONLY(GetContext(gfx)->Unmap(pTexTemp.Get(), 0));
	}

	void Bind::ShaderInputRenderTarget::ToMipCube(Graphics& gfx, const std::string& path) const
	{
		INFOMAN(gfx);
		namespace wrl = Microsoft::WRL;

		// creating a temp texture compatible with the source, but with CPU read access
		wrl::ComPtr<ID3D11Resource> pResSource;
		pShaderResourceView->GetResource(&pResSource);
		wrl::ComPtr<ID3D11Texture2D> pTexSource;
		pResSource.As(&pTexSource);
		D3D11_TEXTURE2D_DESC textureDesc;
		pTexSource->GetDesc(&textureDesc);
		textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
		textureDesc.Usage = D3D11_USAGE_STAGING;
		textureDesc.BindFlags = 0;
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
		wrl::ComPtr<ID3D11Texture2D> pTexTemp;
		GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(
			&textureDesc, nullptr, &pTexTemp
		));

		// copy texture contents
		GFX_THROW_INFO_ONLY(GetContext(gfx)->CopyResource(pTexTemp.Get(), pTexSource.Get()));

		// create Surface and copy from temp texture to it
		const auto width = GetWidth();
		const auto height = GetHeight();
		D3D11_MAPPED_SUBRESOURCE msr = {};
		GFX_THROW_INFO(GetContext(gfx)->Map(pTexTemp.Get(), 0, D3D11_MAP::D3D11_MAP_READ, 0, &msr));
		auto pSrcBytes = static_cast<const char*>(msr.pData);
		unsigned int pRowBase = 0;
		for (unsigned char z = 0; z < 6; z++)
		{
			for (unsigned char i = 0; i < 5; i++)
			{
				unsigned char scale = std::powf(2, i);
				Surface s{ width / scale,height / scale };
				unsigned int _width = width / scale;
				unsigned int _height = height / scale;
				UINT rowPitch = 0u;
				if (i < 4)
					rowPitch = msr.RowPitch / scale;
				else
					rowPitch = msr.RowPitch / scale * 2; // nimade weishenme???
				for (unsigned int y = 0; y < _height; y++)
				{
					auto pSrcRow = reinterpret_cast<const Surface::Color*>(pSrcBytes + pRowBase + rowPitch * size_t(y));
					for (unsigned int x = 0; x < _width; x++)
					{
						s.PutPixel(x, y, *(pSrcRow + x));
					}
				}
				pRowBase += _height * rowPitch;
				s.Save(path.c_str(), "#" + std::to_string(z) + "#" + std::to_string(i));
			}
		}
		GFX_THROW_INFO_ONLY(GetContext(gfx)->Unmap(pTexTemp.Get(), 0));
	}

	void ShaderInputRenderTarget::Bind( Graphics& gfx ) noxnd
	{
		INFOMAN_NOHR( gfx );
		assert(shaderIndex & 0b00001111);
		for (unsigned char i = 0; i < 5; i++)
		{
			if (shaderIndex & 0b00001000)
			{
				GFX_THROW_INFO_ONLY(GetContext(gfx)->VSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
			}
			if (shaderIndex & 0b00000100)
			{
				GFX_THROW_INFO_ONLY(GetContext(gfx)->HSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
			}
			if (shaderIndex & 0b00000010)
			{
				GFX_THROW_INFO_ONLY(GetContext(gfx)->DSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
			}
			if (shaderIndex & 0b00000001)
			{
				GFX_THROW_INFO_ONLY(GetContext(gfx)->PSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
			}
		}	
	}
	

	void OutputOnlyRenderTarget::Bind( Graphics& gfx ) noxnd
	{
		assert( "Cannot bind OuputOnlyRenderTarget as shader input" && false );
	}

	OutputOnlyRenderTarget::OutputOnlyRenderTarget( Graphics& gfx,ID3D11Texture2D* pTexture )
		:
		RenderTarget( gfx,pTexture )
	{}
}
