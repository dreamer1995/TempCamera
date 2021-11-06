#pragma once
#include "Bindable.h"
#include "BufferResource.h"
#include <optional>

class Graphics;
class Surface;

namespace Bind
{
	class DepthStencil;

	class RenderTarget : public Bindable, public BufferResource
	{
	public:
		enum class Type
		{
			Default,
			PreCalSimpleCube,
			PreCalMipCube,
			PreBRDFPlane,
			GBuffer,
			UVABuffer
		};
	public:
		void BindAsBuffer( Graphics& gfx ) noxnd override;
		void BindAsBuffer( Graphics& gfx,BufferResource* depthStencil ) noxnd override;
		void BindAsBuffer( Graphics& gfx,DepthStencil* depthStencil ) noxnd;
		void Clear( Graphics& gfx ) noxnd override;
		void Clear( Graphics& gfx,const std::array<float,4>& color ) noxnd;
		UINT GetWidth() const noexcept;
		UINT GetHeight() const noexcept;
		void ChangeMipSlice(Graphics& gfx, UINT i) noxnd;
		Surface ToSurface( Graphics& gfx ) const;
		void Dumpy( Graphics& gfx,const std::string& path ) const;
	private:
		std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>,D3D11_TEXTURE2D_DESC> MakeStaging( Graphics& gfx ) const;
		void BindAsBuffer( Graphics& gfx,ID3D11DepthStencilView* pDepthStencilView ) noxnd;
	protected:
		RenderTarget( Graphics& gfx,ID3D11Texture2D* pTexture,std::optional<UINT> face );
		RenderTarget(Graphics& gfx, UINT width, UINT height, Type type = Type::Default, DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM);
		UINT width;
		UINT height;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTargetView;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTargetCubeView[6];
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTargetGBufferView[8];
		Type type;
	public:
		UINT targetIndex = 0;
		float _width = 256.0f;
		float _height = 256.0f;
	};

	class ShaderInputRenderTarget : public RenderTarget
	{
	public:
		ShaderInputRenderTarget(Graphics& gfx, UINT width, UINT height, UINT slot, Type type = Type::Default, UINT shaderIndex = 0b1u, DXGI_FORMAT format = DXGI_FORMAT_B8G8R8A8_UNORM);
		void Bind( Graphics& gfx ) noxnd override;
		Surface ToSurface( Graphics& gfx ) const;
		void ToCube(Graphics& gfx, const std::string& path) const;
		void ToMipCube(Graphics& gfx, const std::string& path) const;
		void BanToBind() noxnd;
		void ReleaseToBind() noxnd;
	private:
		UINT slot;
		UINT shaderIndex;
		bool banToBind = false;
	public:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceGBufferViews[8];
	};

	// RT for Graphics to create RenderTarget for the back buffer
	class OutputOnlyRenderTarget : public RenderTarget
	{
	public:
		void Bind( Graphics& gfx ) noxnd override;
		OutputOnlyRenderTarget( Graphics& gfx,ID3D11Texture2D* pTexture,std::optional<UINT> face = {} );
	};
}