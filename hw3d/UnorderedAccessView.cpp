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
		D3D11_BUFFER_DESC uabd;
		uabd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		uabd.CPUAccessFlags = 0;

		DataPerCell dataCell;

		uabd.ByteWidth = sizeof(dataCell) * width * height;
		uabd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		uabd.StructureByteStride = sizeof(dataCell);	// We assume the output data is in the RGBA format, 8 bits per channel

		uabd.Usage = D3D11_USAGE_DEFAULT;

		ID3D11Buffer* UAGPUBuffer;
		D3D11_SUBRESOURCE_DATA uasd = {};
		uasd.pSysMem = &(dataCell);

		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&uabd, NULL, &UAGPUBuffer));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
		uavd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavd.Buffer.FirstElement = 0;
		uavd.Buffer.Flags = 0;
		uavd.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
		uavd.Buffer.NumElements = uabd.ByteWidth / uabd.StructureByteStride;

		GFX_THROW_INFO(GetDevice(gfx)->CreateUnorderedAccessView(UAGPUBuffer, &uavd, &pUnorderedAccessView));
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

		UnorderedAccessView::DataPerCell dataCell;

		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.ElementWidth = sizeof(dataCell);
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = width * height;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		
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
