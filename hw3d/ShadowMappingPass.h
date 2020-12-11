#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include <vector>
#include "PixelShader.h"
#include "VertexShader.h"
#include "Stencil.h"
#include "Rasterizer.h"
#include "Source.h"
#include "RenderTarget.h"
#include "Blender.h"
#include "NullPixelShader.h"

class Graphics;

namespace Rgph
{
	class ShadowMappingPass : public RenderQueuePass
	{
	public:
		void BindShadowCamera(const Camera& dCam, std::vector<std::shared_ptr<Camera>> pCams) noexcept
		{
			pDShadowCamera = &dCam;
			for (unsigned char i = 0; i < pCams.size(); i++)
			{
				pPShadowCameras.emplace_back(pCams[i]);
			}
		}
		ShadowMappingPass( Graphics& gfx,std::string name )
			:
			RenderQueuePass( std::move( name ) )
		{
			using namespace Bind;
			depthStencil = std::make_unique<ShaderInputDepthStencil>(gfx, 14u, DepthStencil::Usage::ShadowDepth);
			AddBind( VertexShader::Resolve( gfx,"Solid_VS.cso" ) );
			AddBind( NullPixelShader::Resolve( gfx ) );
			AddBind( Stencil::Resolve( gfx,Stencil::Mode::Off ) );
			AddBindSink<Bind::Bindable>( "shadowRasterizer" );
			AddBind( Blender::Resolve( gfx,false ) );
			RegisterSource( DirectBindableSource<Bind::DepthStencil>::Make( "dMap",depthStencil ) );
		}
		void Execute( Graphics& gfx ) const noxnd override
		{
			depthStencil->Clear( gfx );
			pDShadowCamera->BindToGraphics( gfx );
			RenderQueuePass::Execute( gfx );
		}
		void DumpShadowMap( Graphics& gfx,const std::string& path ) const
		{
			depthStencil->ToSurface( gfx ).Save( path );
		}
	private:
		const Camera* pDShadowCamera = nullptr;
	public:
		std::vector<std::shared_ptr<Camera>> pPShadowCameras;
	};
}