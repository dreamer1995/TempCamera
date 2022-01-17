#include "FullscreenPass.h"
#include "BindableCommon.h"


namespace Rgph
{
	namespace dx = DirectX;

	FullscreenPass::FullscreenPass(const std::string name, Graphics& gfx, D3D_PRIMITIVE_TOPOLOGY topology) noxnd
		:
	BindingPassWithRTDS( std::move( name ) )
	{
		// setup fullscreen geometry
		Dvtx::VertexLayout lay;
		lay.Append( Dvtx::VertexLayout::Position2D );
		Dvtx::VertexBuffer bufFull{ lay };
		bufFull.EmplaceBack( dx::XMFLOAT2{ -1,1 } );
		bufFull.EmplaceBack( dx::XMFLOAT2{ 1,1 } );
		bufFull.EmplaceBack( dx::XMFLOAT2{ -1,-1 } );
		bufFull.EmplaceBack( dx::XMFLOAT2{ 1,-1 } );
		AddBind( Bind::VertexBuffer::Resolve( gfx,"$Full",std::move( bufFull ) ) );
		if (topology == D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
		{
			std::vector<unsigned short> indices = { 0,1,2,1,3,2 };
			AddBind(Bind::IndexBuffer::Resolve(gfx, "$Full", std::move(indices)));
		}	
		// setup other common fullscreen bindables
		auto vs = Bind::VertexShader::Resolve( gfx,"Fullscreen_VS.cso" );
		AddBind( Bind::InputLayout::Resolve( gfx,lay,*vs ) );
		AddBind( std::move( vs ) );
		AddBind(Bind::Topology::Resolve(gfx, topology));
		AddBind( Bind::Rasterizer::Resolve( gfx,false ) );
	}

	void FullscreenPass::Execute( Graphics& gfx ) const noxnd
	{
		BindAll( gfx );
		gfx.DrawIndexed( 6u );
	}
}
