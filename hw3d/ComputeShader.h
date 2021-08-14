#pragma once
#include "Bindable.h"

namespace Bind
{
	class ComputeShader : public Bindable
	{
	public:
		ComputeShader(Graphics& gfx, const std::string& path);
		void Bind(Graphics& gfx) noxnd override;
		static std::shared_ptr<ComputeShader> Resolve(Graphics& gfx, const std::string& path);
		static std::string GenerateUID(const std::string& path);
		std::string GetUID() const noexcept override;
	protected:
		std::string path;
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> pComputeShader;
	};
}