#pragma once
#include "Bindable.h"

class Surface;

namespace Bind
{
	class OutputOnlyDepthStencil;
	class OutputOnlyRenderTarget;

	class TextureCube : public Bindable
	{
	public:
		TextureCube(Graphics& gfx, const std::string& path, UINT slot, bool manuallyGenerateMips, UINT shaderIndex);
		void Bind( Graphics& gfx ) noxnd override;
		static std::shared_ptr<TextureCube> Resolve(Graphics& gfx, const std::string& path, UINT slot = 0, bool manuallyGenerateMips = false, UINT shaderIndex = 0b1);
		static std::string GenerateUID(const std::string& path, UINT slot, bool manuallyGenerateMips, UINT shaderIndex);
		std::string GetUID() const noexcept override;
	private:
		unsigned int slot;
		UINT shaderIndex;
	protected:
		std::string path;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		bool manuallyGenerateMips;
	};


	class CubeTargetTexture : public Bindable
	{
	public:
		CubeTargetTexture( Graphics& gfx,UINT width,UINT height,UINT slot = 0,DXGI_FORMAT format = DXGI_FORMAT::DXGI_FORMAT_B8G8R8A8_UNORM );
		void Bind( Graphics& gfx ) noxnd override;
		std::shared_ptr<OutputOnlyRenderTarget> GetRenderTarget( size_t index ) const;
	private:
		unsigned int slot;
	protected:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		std::vector<std::shared_ptr<OutputOnlyRenderTarget>> renderTargets;
	};

	class DepthCubeTexture : public Bindable
	{
	public:
		DepthCubeTexture( Graphics& gfx,UINT size,UINT slot = 0 );
		void Bind( Graphics& gfx ) noxnd override;
		std::shared_ptr<OutputOnlyDepthStencil> GetDepthBuffer( size_t index ) const;
	private:
		unsigned int slot;
		UINT shaderIndex;
	protected:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pTextureView;
		std::vector<std::shared_ptr<OutputOnlyDepthStencil>> depthBuffers;
	};
}