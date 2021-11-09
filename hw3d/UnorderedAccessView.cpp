#include "UnorderedAccessView.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace Bind
{
	namespace wrl = Microsoft::WRL;
	
	UnorderedAccessView::UnorderedAccessView(Graphics& gfx, UINT slot, UINT shaderIndex)
		:
		slot(slot),
		shaderIndex(shaderIndex)
	{
		INFOMAN(gfx);

		// The compute shader will need to output to some buffer so here we create a GPU buffer for that.
		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		cbd.ByteWidth = sizeof(consts);
		cbd.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		cbd.StructureByteStride = 4;	// We assume the output data is in the RGBA format, 8 bits per channel

		ID3D11Buffer* UAGPUBuffer;
		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &consts;
		GFX_THROW_INFO(GetDevice(gfx)->CreateBuffer(&cbd, &csd, &UAGPUBuffer));

		// The view we need for the output is an unordered access view. This is to allow the compute shader to write anywhere in the buffer.
		D3D11_BUFFER_DESC descBuf;
		ZeroMemory(&descBuf, sizeof(descBuf));
		m_destDataGPUBuffer->GetDesc(&descBuf);

		D3D11_UNORDERED_ACCESS_VIEW_DESC descView;
		ZeroMemory(&descView, sizeof(descView));
		descView.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
		descView.Buffer.FirstElement = 0;

		descView.Format = DXGI_FORMAT_UNKNOWN;      // Format must be must be DXGI_FORMAT_UNKNOWN, when creating a View of a Structured Buffer
		descView.Buffer.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;

		if (FAILED(m_pd3dDevice->CreateUnorderedAccessView(m_destDataGPUBuffer, &descView, &m_destDataGPUBufferView)))
			return false;

		return true;
	}
}
