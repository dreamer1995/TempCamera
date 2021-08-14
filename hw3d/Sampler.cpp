#include "Sampler.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace Bind
{
	Sampler::Sampler(Graphics& gfx, Filter filter, Address address, UINT slot, UINT shaderIndex, float LODRange)
		:
		filter(filter),
		address(address),
		slot(slot),
		shaderIndex(shaderIndex),
		LODRange(LODRange)
	{
		INFOMAN( gfx );

		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };
		samplerDesc.Filter = [filter]() {
			switch(filter)
			{
			case Filter::Anisotropic: return D3D11_FILTER_ANISOTROPIC;
			case Filter::Point: return D3D11_FILTER_MIN_MAG_MIP_POINT;
			default:
			return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			}
		}();
		switch (address)
		{
		case Address::Clamp:
		{
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		}
		case Address::Mirror:
		{
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_MIRROR;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_MIRROR;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_MIRROR;
			break;
		}
		default:
		{
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.MaxAnisotropy = D3D11_REQ_MAXANISOTROPY;
		}
		}

		samplerDesc.MinLOD = -LODRange;
		samplerDesc.MaxLOD = LODRange;

		GFX_THROW_INFO( GetDevice( gfx )->CreateSamplerState( &samplerDesc,&pSampler ) );
	}

	void Sampler::Bind( Graphics& gfx ) noxnd
	{
		assert(shaderIndex & 0b00111111);
		INFOMAN_NOHR( gfx );
		if (shaderIndex & 0b00010000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->VSSetSamplers(slot, 1, pSampler.GetAddressOf()));
		}
		if (shaderIndex & 0b00001000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->HSSetSamplers(slot, 1, pSampler.GetAddressOf()));
		}
		if (shaderIndex & 0b00000100)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->DSSetSamplers(slot, 1, pSampler.GetAddressOf()));
		}
		if (shaderIndex & 0b00000010)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->GSSetSamplers(slot, 1, pSampler.GetAddressOf()));
		}
		if (shaderIndex & 0b00000001)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->PSSetSamplers(slot, 1, pSampler.GetAddressOf()));
		}
		if (shaderIndex & 0b00100000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->CSSetSamplers(slot, 1, pSampler.GetAddressOf()));
		}
	}
	std::shared_ptr<Sampler> Sampler::Resolve(Graphics& gfx, Filter filter, Address address, UINT slot, UINT shaderIndex, float LODRange)
	{
		return Codex::Resolve<Sampler>(gfx, filter, address, slot, shaderIndex, LODRange);
	}
	std::string Sampler::GenerateUID(Filter filter, Address address, UINT slot, UINT shaderIndex, float LODRange)
	{
		using namespace std::string_literals;
		return typeid(Sampler).name() + "#"s + std::to_string( (int)filter) + std::to_string((int)address) + "@"s + std::to_string((int)slot) +
			std::to_string(shaderIndex) + std::to_string(LODRange);
	}
	std::string Sampler::GetUID() const noexcept
	{
		return GenerateUID(filter, address, slot, shaderIndex, LODRange);
	}
}