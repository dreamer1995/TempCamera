#include "UnorderedAccessView.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace Bind
{
	namespace wrl = Microsoft::WRL;

	UnorderedAccessView::UnorderedAccessView(Graphics& gfx, UINT width, UINT height, UINT slot)
		:
		slot(slot)
	{
		INFOMAN(gfx);

		// The compute shader will need to output to some buffer so here we create a GPU buffer for that.
		D3D11_TEXTURE2D_DESC uatd;
		//DataPerCell dataCell;

		uatd.Width = width;
		uatd.Height = height;
		uatd.MipLevels = 1;
		uatd.ArraySize = 1;
		uatd.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		uatd.SampleDesc.Count = 1;
		uatd.SampleDesc.Quality = 0;
		uatd.Usage = D3D11_USAGE_DEFAULT;
		uatd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE | D3D10_BIND_RENDER_TARGET;
		uatd.CPUAccessFlags = 0;
		uatd.MiscFlags = 0;

		ID3D11Texture2D* UAVTex;
		D3D11_SUBRESOURCE_DATA uasd = {};

		GFX_THROW_INFO(GetDevice(gfx)->CreateTexture2D(&uatd, NULL, &UAVTex));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
		uavd.Format = uatd.Format;
		uavd.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
		uavd.Texture2D.MipSlice = 0;

		GFX_THROW_INFO(GetDevice(gfx)->CreateUnorderedAccessView(UAVTex, &uavd, &pUnorderedAccessView));
	}

	void UnorderedAccessView::BindAsBuffer(Graphics& gfx) noxnd
	{
		INFOMAN_NOHR(gfx);

		UINT initCounts = 0;
		GFX_THROW_INFO_ONLY(GetContext(gfx)->CSSetUnorderedAccessViews(0, 1, pUnorderedAccessView.GetAddressOf(), &initCounts));
	}

	ShaderInputUAV::ShaderInputUAV(Graphics& gfx, UINT width, UINT height, UINT slot, UINT shaderIndex)
		:
		UnorderedAccessView(gfx, width, height, slot),
		width(width),
		height(height),
		slot(slot),
		shaderIndex(shaderIndex)
	{
		INFOMAN(gfx);

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
		UnorderedAccessView::pUnorderedAccessView->GetDesc(&uavd);
		wrl::ComPtr<ID3D11Resource> pRes;
		UnorderedAccessView::pUnorderedAccessView->GetResource(&pRes);
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};

		//UnorderedAccessView::DataPerCell dataCell;

		srvDesc.Format = uavd.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = UINT32_MAX;
		
		GFX_THROW_INFO(GetDevice(gfx)->CreateShaderResourceView(
			pRes.Get(), &srvDesc, &pShaderResourceView
		));
	}

	void ShaderInputUAV::Bind(Graphics& gfx) noxnd
	{
		if (banToBind)
			return;

		assert(shaderIndex & 0b00111111);
		INFOMAN_NOHR(gfx);
		if (shaderIndex & 0b00010000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->VSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
		if (shaderIndex & 0b00001000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->HSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
		if (shaderIndex & 0b00000100)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->DSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
		if (shaderIndex & 0b00000010)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->GSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
		if (shaderIndex & 0b00000001)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->PSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
		if (shaderIndex & 0b00100000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->CSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
	}

	std::shared_ptr<ShaderInputUAV> ShaderInputUAV::Resolve(Graphics& gfx, UINT width, UINT height, UINT slot, UINT shaderIndex)
	{
		return Codex::Resolve<ShaderInputUAV>(gfx, width, height, slot, shaderIndex);
	}

	std::string ShaderInputUAV::GenerateUID(UINT width, UINT height, UINT slot, UINT shaderIndex)
	{
		using namespace std::string_literals;
		return typeid(ShaderInputUAV).name() + "#"s + "#" + std::to_string(width) + "#" + std::to_string(height) + "#" + std::to_string(slot) + "#" + std::to_string(shaderIndex);
	}

	std::string ShaderInputUAV::GetUID() const noexcept
	{
		return GenerateUID(width, height, slot, shaderIndex);
	}

	void ShaderInputUAV::BanToBind() noxnd
	{
		banToBind = true;
	}

	void ShaderInputUAV::ReleaseToBind() noxnd
	{
		banToBind = false;
	}
}
