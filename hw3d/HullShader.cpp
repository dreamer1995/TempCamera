#include "HullShader.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"
#include "ChiliUtil.h"

namespace Bind
{
	HullShader::HullShader(Graphics& gfx, const std::string& path)
		:
		path(path)
	{
		INFOMAN(gfx);

		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
		GFX_THROW_INFO(D3DReadFileToBlob(ToWide("ShaderBins\\" + path).c_str(), &pBlob));
		GFX_THROW_INFO(GetDevice(gfx)->CreateHullShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pHullShader));
	}

	void HullShader::Bind(Graphics& gfx) noxnd
	{
		INFOMAN_NOHR(gfx);
		GFX_THROW_INFO_ONLY(GetContext(gfx)->HSSetShader(pHullShader.Get(), nullptr, 0u));
	}
	std::shared_ptr<HullShader> HullShader::Resolve(Graphics& gfx, const std::string& path)
	{
		return Codex::Resolve<HullShader>(gfx, path);
	}
	std::string HullShader::GenerateUID(const std::string& path)
	{
		using namespace std::string_literals;
		return typeid(HullShader).name() + "#"s + path;
	}
	std::string HullShader::GetUID() const noexcept
	{
		return GenerateUID(path);
	}
}
