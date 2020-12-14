#pragma once
#include "Bindable.h"
#include "BufferResource.h"
#include "Surface.h"

class Graphics;

namespace Bind
{
	class RenderTarget;

	class DepthStencil : public Bindable, public BufferResource
	{
		friend RenderTarget;
	public:
		enum class Usage
		{
			DepthStencil,
			ShadowDepth,
		};
	public:
		enum class Type
		{
			Default,
			Cube
		};
	public:
		void BindAsBuffer( Graphics& gfx ) noxnd override;
		void BindAsBuffer( Graphics& gfx,BufferResource* renderTarget ) noxnd override;
		void BindAsBuffer( Graphics& gfx,RenderTarget* rt ) noxnd;
		void Clear( Graphics& gfx ) noxnd override;
		Surface ToSurface( Graphics& gfx,bool linearlize = true ) const;
		void Dumpy( Graphics& gfx,const std::string& path ) const;
		unsigned int GetWidth() const;
		unsigned int GetHeight() const;
	private:
		std::pair<Microsoft::WRL::ComPtr<ID3D11Texture2D>,D3D11_TEXTURE2D_DESC> MakeStaging( Graphics& gfx ) const;
	protected:
		DepthStencil(Graphics& gfx, UINT width, UINT height, bool canBindShaderInput, Usage usage, Type type = Type::Default);
		DepthStencil( Graphics& gfx,Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture,UINT face );
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> pDepthStencilCubeView[6];
		unsigned int width;
		unsigned int height;
		Type type;
	public:
		UINT targetIndex = 0;
	};

	class ShaderInputDepthStencil : public DepthStencil
	{
	public:
		ShaderInputDepthStencil(Graphics& gfx, UINT slot, Usage usage = Usage::DepthStencil, Type type = Type::Default);
		ShaderInputDepthStencil(Graphics& gfx, UINT width, UINT height, UINT slot,
			Usage usage = Usage::DepthStencil, Type type = Type::Default);
		void Bind( Graphics& gfx ) noxnd override;
	private:
		UINT slot;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
	};

	class OutputOnlyDepthStencil : public DepthStencil
	{
	public:
		OutputOnlyDepthStencil( Graphics& gfx );
		OutputOnlyDepthStencil( Graphics& gfx,UINT width,UINT height );
		OutputOnlyDepthStencil( Graphics& gfx,Microsoft::WRL::ComPtr<ID3D11Texture2D> pTexture,UINT face );
		void Bind( Graphics& gfx ) noxnd override;
	};
}
