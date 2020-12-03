#pragma once
#include "Bindable.h"
#include "BufferResource.h"

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
			PreBRDFPlane
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
	private:
		void BindAsBuffer( Graphics& gfx,ID3D11DepthStencilView* pDepthStencilView ) noxnd;
	protected:
		RenderTarget( Graphics& gfx,ID3D11Texture2D* pTexture );
		RenderTarget(Graphics& gfx, UINT width, UINT height, Type type = Type::Default);
		UINT width;
		UINT height;
		Type type;
	public:
		UINT targetIndex = 0;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTargetView;
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> pTargetCubeView[6];
		float _width = 256.0f;
		float _height = 256.0f;
	};

	class ShaderInputRenderTarget : public RenderTarget
	{
	public:
		ShaderInputRenderTarget(Graphics& gfx, UINT width, UINT height, UINT slot, Type type = Type::Default, UINT shaderIndex = 0b1u);
		void Bind( Graphics& gfx ) noxnd override;
		Surface ToSurface( Graphics& gfx ) const;
		void ToCube(Graphics& gfx, const std::string& path) const;
	private:
		UINT slot;
		UINT shaderIndex;
	public:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
	};

	// RT for Graphics to create RenderTarget for the back buffer
	class OutputOnlyRenderTarget : public RenderTarget
	{
		friend Graphics;
	public:
		void Bind( Graphics& gfx ) noxnd override;
	private:
		OutputOnlyRenderTarget( Graphics& gfx,ID3D11Texture2D* pTexture );
	};
}