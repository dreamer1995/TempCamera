#include "ShadowSampler.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"

namespace Bind
{
	ShadowSampler::ShadowSampler(Graphics& gfx, UINT slot, UINT shaderIndex)
		:
		slot(slot),
		shaderIndex(shaderIndex)
	{
		for( size_t i = 0; i < 4; i++ )
		{
			curSampler = i;
			samplers[i] = MakeSampler( gfx,GetBilinear(),GetHwPcf() );
		}
		// pre-bind something to both sampler slots by default so that no slot is unbound during drawing
		SetBilinear( true );
		SetHwPcf( false );
		Bind( gfx );
		SetHwPcf( true );
		Bind( gfx );
	}

	void ShadowSampler::SetBilinear( bool bilin )
	{
		curSampler = (curSampler & ~0b01) | (bilin ? 0b01 : 0);
	}

	void ShadowSampler::SetHwPcf( bool hwPcf )
	{
		curSampler = (curSampler & ~0b10) | (hwPcf ? 0b10 : 0);
	}

	bool ShadowSampler::GetBilinear() const
	{
		return curSampler & 0b01;
	}

	bool ShadowSampler::GetHwPcf() const
	{
		return curSampler & 0b10;
	}

	UINT ShadowSampler::GetCurrentSlot() const
	{
		return GetHwPcf() ? slot + 0 : slot + 1;
	}

	size_t ShadowSampler::ShadowSamplerIndex( bool bilin,bool hwPcf )
	{
		return (bilin ? 0b01 : 0) + (hwPcf ? 0b10 : 0);
	}

	Microsoft::WRL::ComPtr<ID3D11SamplerState> ShadowSampler::MakeSampler( Graphics& gfx,bool bilin,bool hwPcf )
	{
		INFOMAN( gfx );

		D3D11_SAMPLER_DESC samplerDesc = CD3D11_SAMPLER_DESC{ CD3D11_DEFAULT{} };

		samplerDesc.BorderColor[0] = 1.0f;
		samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;

		if( hwPcf )
		{
			samplerDesc.Filter = bilin ? D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
			samplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
		}
		else
		{
			samplerDesc.Filter = bilin ? D3D11_FILTER_MIN_MAG_MIP_LINEAR : D3D11_FILTER_MIN_MAG_MIP_POINT;
		}

		Microsoft::WRL::ComPtr<ID3D11SamplerState> pSampler;
		GFX_THROW_INFO( GetDevice( gfx )->CreateSamplerState( &samplerDesc,&pSampler ) );
		return std::move( pSampler );
	}

	void ShadowSampler::Bind( Graphics& gfx ) noxnd
	{		
		assert(shaderIndex & 0b00111111);
		INFOMAN_NOHR(gfx);
		if (shaderIndex & 0b00010000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->VSSetSamplers(GetCurrentSlot(), 1, samplers[curSampler].GetAddressOf()));
		}
		if (shaderIndex & 0b00001000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->HSSetSamplers(GetCurrentSlot(), 1, samplers[curSampler].GetAddressOf()));
		}
		if (shaderIndex & 0b00000100)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->DSSetSamplers(GetCurrentSlot(), 1, samplers[curSampler].GetAddressOf()));
		}
		if (shaderIndex & 0b00000010)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->GSSetSamplers(GetCurrentSlot(), 1, samplers[curSampler].GetAddressOf()));
		}
		if (shaderIndex & 0b00000001)
		{
			GFX_THROW_INFO_ONLY( GetContext( gfx )->PSSetSamplers( GetCurrentSlot(),1,samplers[curSampler].GetAddressOf() ) );
		}
		if (shaderIndex & 0b00100000)
		{
			GFX_THROW_INFO_ONLY(GetContext(gfx)->CSSetSamplers(GetCurrentSlot(), 1, samplers[curSampler].GetAddressOf()));
		}
	}
} 