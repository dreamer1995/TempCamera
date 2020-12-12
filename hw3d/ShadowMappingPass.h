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
		void BindShadowCamera(Graphics& gfx, const Camera& dCam, std::vector<std::shared_ptr<Camera>> pCams) noexcept
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
			shadowDepthStencil = std::make_unique<ShaderInputDepthStencil>(gfx, 4096u, 4096u, 14u, DepthStencil::Usage::ShadowDepth);
			depthStencil = shadowDepthStencil;
			AddBind( VertexShader::Resolve( gfx,"Solid_VS.cso" ) );
			AddBind( NullPixelShader::Resolve( gfx ) );
			AddBind( Stencil::Resolve( gfx,Stencil::Mode::Off ) );
			AddBindSink<Bind::Bindable>( "shadowRasterizer" );
			AddBind( Blender::Resolve( gfx,false ) );
			RegisterSource(DirectBindableSource<Bind::ShaderInputDepthStencil>::Make("dMap", shadowDepthStencil));
			for (unsigned char i = 0; i < 3; i++)
			{
				if (i == 0)
					shadowDepthStencils[i] = std::make_unique<Bind::ShaderInputDepthStencil>
					(gfx, 15u + i, Bind::DepthStencil::Usage::ShadowDepth);
				else
					shadowDepthStencils[i] = std::make_unique<Bind::ShaderInputDepthStencil>
						(gfx, 1u, 1u, 15u + i, Bind::DepthStencil::Usage::ShadowDepth);
				SetDepthBuffer(shadowDepthStencils[i]);
				RegisterSource(DirectBindableSource<Bind::ShaderInputDepthStencil>::Make("pMap" + std::to_string(i), shadowDepthStencils[i]));
			}
		}
		void Execute( Graphics& gfx ) const noxnd override
		{
			SetDepthBuffer(shadowDepthStencil);
			depthStencil->Clear( gfx );
			pDShadowCamera->BindToGraphics( gfx );
			RenderQueuePass::Execute( gfx );
			
			for (unsigned char i = 0; i < pPShadowCameras.size(); i++)
			{
				SetDepthBuffer(shadowDepthStencils[i]);
				depthStencil->Clear(gfx);
				pPShadowCameras[i]->BindToGraphics(gfx);
				RenderQueuePass::Execute(gfx);
			}
			//RegisterSource(DirectBindableSource<Bind::DepthStencil>::Make("dMap", depthStencil));
		}
		void DumpShadowMap( Graphics& gfx,const std::string& path ) const
		{
			depthStencil->ToSurface( gfx ).Save( path );
		}
	private:
		const Camera* pDShadowCamera = nullptr;
		void SetDepthBuffer( std::shared_ptr<Bind::DepthStencil> ds ) const
		{
			const_cast<ShadowMappingPass*>(this)->depthStencil = std::move( ds );
		}
	public:
		std::vector<std::shared_ptr<Camera>> pPShadowCameras;
		std::shared_ptr<Bind::ShaderInputDepthStencil> shadowDepthStencil;
		std::shared_ptr<Bind::ShaderInputDepthStencil> shadowDepthStencils[3];
	};
}