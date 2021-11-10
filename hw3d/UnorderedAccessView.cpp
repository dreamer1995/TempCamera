#include "UnorderedAccessView.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace Bind
{
	namespace wrl = Microsoft::WRL;

	template <typename T>
	UnorderedAccessView<T>::UnorderedAccessView(Graphics& gfx, T& dataType, UINT width, UINT height, UINT slot)
		:
		slot(slot)
	{
		INFOMAN(GraphicsResource::gfx);

		// The compute shader will need to output to some buffer so here we create a GPU buffer for that.
		D3D11_BUFFER_DESC uabd;
		uabd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;

		DataPerCell dataCell;

		uabd.ByteWidth = sizeof(dataCell) * width * height;
		uabd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		uabd.StructureByteStride = 4;	// We assume the output data is in the RGBA format, 8 bits per channel

		ID3D11Buffer* UAGPUBuffer;
		GFX_THROW_INFO(GraphicsResource::GetDevice(gfx)->CreateBuffer(&uabd, NULL, &UAGPUBuffer));

		D3D11_UNORDERED_ACCESS_VIEW_DESC uavd;
		uavd.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		uavd.Buffer.FirstElement = 0;

		uavd.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
		uavd.Buffer.NumElements = uabd.ByteWidth / uabd.StructureByteStride;

		GFX_THROW_INFO(GraphicsResource::GetDevice(gfx)->CreateUnorderedAccessView(UAGPUBuffer, &uavd, &pUnorderedAccessView));
	}

	template <typename T>
	void UnorderedAccessView<T>::Bind(Graphics& gfx) noxnd
	{
		INFOMAN_NOHR(GraphicsResource::gfx);

		UINT initCounts = 0;
		GFX_THROW_INFO_ONLY(GraphicsResource::GetContext(gfx)->CSSetUnorderedAccessViews(0, 1, pUnorderedAccessView.GetAddressOf(), &initCounts));
	}

	template <typename T>
	ShaderInputUAV<T>::ShaderInputUAV(Graphics& gfx, T& dataType, UINT width, UINT height, UINT slot, UINT shaderIndex)
		:
		UnorderedAccessView<T>(gfx, dataType, width, height, slot),
		slot(slot),
		shaderIndex(shaderIndex)
	{
		INFOMAN(GraphicsResource::gfx);

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
		wrl::ComPtr<ID3D11Resource> pRes;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
		
		GFX_THROW_INFO(GraphicsResource::GetDevice(gfx)->CreateShaderResourceView(
			pRes.Get(), &srvDesc, &pShaderResourceView
		));
	}

	template <typename T>
	void ShaderInputUAV<T>::Bind(Graphics& gfx) noxnd
	{
		INFOMAN_NOHR(GraphicsResource::gfx);

		assert(shaderIndex & 0b00111111);
		if (shaderIndex & 0b00010000)
		{
			GFX_THROW_INFO_ONLY(GraphicsResource::GetContext(gfx)->VSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
		if (shaderIndex & 0b00001000)
		{
			GFX_THROW_INFO_ONLY(GraphicsResource::GetContext(gfx)->HSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
		if (shaderIndex & 0b00000100)
		{
			GFX_THROW_INFO_ONLY(GraphicsResource::GetContext(gfx)->DSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
		if (shaderIndex & 0b00000010)
		{
			GFX_THROW_INFO_ONLY(GraphicsResource::GetContext(gfx)->GSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
		if (shaderIndex & 0b00000001)
		{
			GFX_THROW_INFO_ONLY(GraphicsResource::GetContext(gfx)->PSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
		if (shaderIndex & 0b00100000)
		{
			GFX_THROW_INFO_ONLY(GraphicsResource::GetContext(gfx)->CSSetShaderResources(slot, 1, pShaderResourceView.GetAddressOf()));
		}
	}

	template<typename T>
	std::shared_ptr<ShaderInputUAV<T>> ShaderInputUAV<T>::Resolve(Graphics& gfx, T& dataType, UINT width, UINT height, UINT slot, UINT shaderIndex)
	{
		return Codex::Resolve<UnorderedAccessView>(gfx, dataType, width, height, slot, shaderIndex);
	}

	template<typename T>
	std::string ShaderInputUAV<T>::GenerateUID(UINT slot, UINT shaderIndex)
	{
		using namespace std::string_literals;
		return typeid(UnorderedAccessView).name() + "#"s + "#" + std::to_string(slot) + "#" + std::to_string(shaderIndex);
	}

	template<typename T>
	std::string ShaderInputUAV<T>::GetUID() const noexcept
	{
		return GenerateUID(slot, shaderIndex);
	}
}
