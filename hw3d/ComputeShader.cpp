#include "ComputeShader.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"
#include "ChiliUtil.h"

namespace Bind
{
	ComputeShader::ComputeShader(Graphics& gfx, const std::string& path)
		:
		path(path)
	{
		INFOMAN(gfx);

		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
		GFX_THROW_INFO(D3DReadFileToBlob(ToWide("ShaderBins\\" + path).c_str(), &pBlob));
		GFX_THROW_INFO(GetDevice(gfx)->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pComputeShader));
	}

	void ComputeShader::Bind(Graphics& gfx) noxnd
	{
		INFOMAN_NOHR(gfx);
		GFX_THROW_INFO_ONLY(GetContext(gfx)->CSSetShader(pComputeShader.Get(), nullptr, 0u));
	}
	std::shared_ptr<ComputeShader> ComputeShader::Resolve(Graphics& gfx, const std::string& path)
	{
		return Codex::Resolve<ComputeShader>(gfx, path);
	}
	std::string ComputeShader::GenerateUID(const std::string& path)
	{
		using namespace std::string_literals;
		return typeid(ComputeShader).name() + "#"s + path;
	}
	std::string ComputeShader::GetUID() const noexcept
	{
		return GenerateUID(path);
	}
}
