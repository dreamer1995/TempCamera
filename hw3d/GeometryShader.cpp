#include "GeometryShader.h"
#include "GraphicsThrowMacros.h"
#include "BindableCodex.h"
#include "ChiliUtil.h"

namespace Bind
{
	GeometryShader::GeometryShader(Graphics& gfx, const std::string& path)
		:
		path(path)
	{
		INFOMAN(gfx);

		Microsoft::WRL::ComPtr<ID3DBlob> pBlob;
		GFX_THROW_INFO(D3DReadFileToBlob(ToWide("ShaderBins\\" + path).c_str(), &pBlob));
		GFX_THROW_INFO(GetDevice(gfx)->CreateGeometryShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &pGeometryShader));
	}

	void GeometryShader::Bind(Graphics& gfx) noxnd
	{
		INFOMAN_NOHR(gfx);
		GFX_THROW_INFO_ONLY(GetContext(gfx)->GSSetShader(pGeometryShader.Get(), nullptr, 0u));
	}
	std::shared_ptr<GeometryShader> GeometryShader::Resolve(Graphics& gfx, const std::string& path)
	{
		return Codex::Resolve<GeometryShader>(gfx, path);
	}
	std::string GeometryShader::GenerateUID(const std::string& path)
	{
		using namespace std::string_literals;
		return typeid(GeometryShader).name() + "#"s + path;
	}
	std::string GeometryShader::GetUID() const noexcept
	{
		return GenerateUID(path);
	}
}
