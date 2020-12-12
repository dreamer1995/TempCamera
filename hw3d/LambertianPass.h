#pragma once
#include "RenderQueuePass.h"
#include "Job.h"
#include <vector>
#include "Sink.h"
#include "Source.h"
#include "Stencil.h"
#include "Camera.h"
#include "DepthStencil.h"
#include "ShadowCameraCBuf.h"
#include "ShadowSampler.h"
#include "Blender.h"

class Graphics;

namespace Rgph
{
	class LambertianPass : public RenderQueuePass
	{
	public:
		LambertianPass( Graphics& gfx,std::string name )
			:
			RenderQueuePass( std::move( name ) )
		{
			using namespace Bind;
			pDShadowCBuf = std::make_shared<Bind::ShadowCameraCBuf>(gfx, 5u);
			AddBind(pDShadowCBuf);
			AddBindSink<Bindable>("dShadowMap");
			AddBindSink<Bindable>("pShadowMap0");
			AddBindSink<Bindable>("pShadowMap1");
			AddBindSink<Bindable>("pShadowMap2");
			RegisterSink( DirectBufferSink<RenderTarget>::Make( "renderTarget",renderTarget ) );
			RegisterSink( DirectBufferSink<DepthStencil>::Make( "depthStencil",depthStencil ) );
			AddBindSink<Bindable>( "shadowControl" );
			AddBindSink<Bindable>( "shadowSampler" );
			AddBindSink<Bindable>("cubeMapBlurIn");
			AddBindSink<Bindable>("cubeMapMipIn");
			AddBindSink<Bindable>("planeBRDFLUTIn");
			RegisterSource( DirectBufferSource<RenderTarget>::Make( "renderTarget",renderTarget ) );
			RegisterSource( DirectBufferSource<DepthStencil>::Make( "depthStencil",depthStencil ) );
			AddBind( Stencil::Resolve( gfx,Stencil::Mode::Off ) );
			AddBind(Blender::Resolve(gfx, false));
		}
		void BindMainCamera( const Camera& cam ) noexcept
		{
			pMainCamera = &cam;
		}
		void BindShadowCamera(Graphics& gfx, const Camera& dCam, std::vector<std::shared_ptr<Camera>> pCams) noexcept
		{
			pDShadowCBuf->SetCamera(&dCam);
			for (unsigned char i = 0; i < pCams.size(); i++)
			{
				//pPShadowCBufs.emplace_back(std::make_shared<Bind::ShadowCameraCBuf>(gfx, 7u + i));
				//pPShadowCBufs[i]->SetCamera(&pCams[i]);
				//AddBind(pPShadowCBufs[i]);
				//AddBindSink<Bindable>("pShadowMap");
			}
			
		}
		void Execute( Graphics& gfx ) const noxnd override
		{
			assert( pMainCamera );
			pMainCamera->BindToGraphics( gfx );
			pDShadowCBuf->Update(gfx);
			for (unsigned char i = 0; i < pPShadowCBufs.size(); i++)
				pPShadowCBufs[i]->Update(gfx);	
			RenderQueuePass::Execute( gfx );
		}
	private:
		std::shared_ptr<Bind::ShadowCameraCBuf> pDShadowCBuf;
		std::vector<std::shared_ptr<Bind::ShadowCameraCBuf>> pPShadowCBufs;
		const Camera* pMainCamera = nullptr;
	};
}