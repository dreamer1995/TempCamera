#include "SolidSphere.h"
#include "BindableCommon.h"
#include "GraphicsThrowMacros.h"
#include "Vertex.h"
#include "Sphere.h"
#include "Stencil.h"
#include "Channels.h"
#include "imgui/imgui.h"
#include "TransformCbufScaling.h"

SolidSphere::SolidSphere( Graphics& gfx,float radius )
{
	using namespace Bind;
	namespace dx = DirectX;
	
	auto model = Sphere::Make();
	model.Transform( dx::XMMatrixScaling( radius,radius,radius ) );
	const auto geometryTag = "$sphere." + std::to_string( radius );
	pVertices = VertexBuffer::Resolve( gfx,geometryTag,model.vertices );
	pIndices = IndexBuffer::Resolve( gfx,geometryTag,model.indices );
	pTopology = Topology::Resolve( gfx,D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST );

	auto tcb = std::make_shared<TransformCbufScaling>(gfx);

	{
		Technique solid{ Chan::main };
		Step only( "lambertian" );

		auto pvs = VertexShader::Resolve( gfx,"Solid_VS.cso" );
		only.AddBindable( InputLayout::Resolve( gfx,model.vertices.GetLayout(),*pvs ) );
		only.AddBindable( std::move( pvs ) );

		only.AddBindable( PixelShader::Resolve( gfx,"Solid_PS.cso" ) );

		Dcb::RawLayout lay;
		lay.Add<Dcb::Float3>("color");
		auto buf = Dcb::Buffer(std::move(lay));
		cbuf = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
		only.AddBindable(cbuf);

		only.AddBindable(tcb);

		only.AddBindable( Rasterizer::Resolve( gfx,false ) );

		solid.AddStep( std::move( only ) );
		AddTechnique( std::move( solid ) );
	}
}

void SolidSphere::SetPos( DirectX::XMFLOAT3 pos ) noexcept
{
	this->pos = pos;
}

DirectX::XMMATRIX SolidSphere::GetTransformXM() const noexcept
{
	return DirectX::XMMatrixTranslation( pos.x,pos.y,pos.z );
}

void SolidSphere::ChangeSphereMaterialState() noexcept
{
	class Probe : public TechniqueProbe
	{
	public:
		void OnSetTechnique() override
		{
			using namespace std::string_literals;
			ImGui::TextColored({ 0.4f,1.0f,0.6f,1.0f }, pTech->GetName().c_str());
			bool active = pTech->IsActive();
			ImGui::Checkbox(("Tech Active##"s + std::to_string(techIdx)).c_str(), &active);
			pTech->SetActiveState(active);
		}
		bool OnVisitBuffer(Dcb::Buffer& buf) override
		{
			namespace dx = DirectX;
			float dirty = false;
			const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };
			auto tag = [tagScratch = std::string{}, tagString = "##" + std::to_string(bufIdx)]
			(const char* label) mutable
			{
				tagScratch = label + tagString;
				return tagScratch.c_str();
			};

			if (auto v = buf["scale"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat(tag("Scale"), &v, 0.0f, 2.0f, "%.3f", 1.0f));
			}

			return dirty;
		}
	} probe;

	Accept(probe);
}

void SolidSphere::SetColor(DirectX::XMFLOAT3 diffuseColor) noexcept
{
	auto buf = cbuf->GetBuffer();
	buf["color"] = diffuseColor;
	cbuf->SetBuffer(buf);
}