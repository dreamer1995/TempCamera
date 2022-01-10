#pragma once
#include "Bindable.h"

namespace Bind
{
	class UnorderedAccessView : public Bindable
	{
	public:
		//struct DataPerCell
		//{
		//	float r;
		//	float g;
		//	float b;
		//	float a;
		//};

		UnorderedAccessView(Graphics& gfx, UINT width, UINT height, UINT slot);
		void BindAsBuffer(Graphics& gfx) noxnd;
	private:
		unsigned int slot;
	protected:
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> pUnorderedAccessView;
	};

	class ShaderInputUAV : public UnorderedAccessView
	{
	public:
		ShaderInputUAV(Graphics& gfx, UINT width, UINT height, UINT slot = 0, UINT shaderIndex = 0b1u);
		void Bind(Graphics& gfx) noxnd override;
		static std::shared_ptr<ShaderInputUAV> Resolve(Graphics& gfx, UINT width, UINT height, UINT slot = 0, UINT shaderIndex = 0b1u);
		static std::string GenerateUID(UINT width, UINT height, UINT slot, UINT shaderIndex);
		std::string GetUID() const noexcept override;
		void BanToBind() noxnd;
		void ReleaseToBind() noxnd;
	private:
		UINT width;
		UINT height;
		UINT slot;
		UINT shaderIndex;
		bool banToBind = false;
	public:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> pShaderResourceView;
	};
}