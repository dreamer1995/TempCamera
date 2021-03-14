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
#include "ChiliMath.h"

namespace dx = DirectX;

class Graphics;

namespace Rgph
{
	class ShadowMappingPass : public RenderQueuePass
	{
	public:
		void BindShadowCamera(Graphics& gfx, const Camera& dCam, std::vector<std::shared_ptr<PointLight>> pCams) noexcept
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
			AddBindSink<Bindable>( "shadowRasterizer" );
			AddBind( Blender::Resolve( gfx,false ) );
			RegisterSource(DirectBindableSource<ShaderInputDepthStencil>::Make("dMap", shadowDepthStencil));
			for (unsigned char i = 0; i < 3; i++)
			{
				if (i < 1)
					shadowDepthStencils[i] = std::make_unique<ShaderInputDepthStencil>
					(gfx, 1000u, 1000u, 15u + i, DepthStencil::Usage::ShadowDepth, DepthStencil::Type::Cube);
				else
					shadowDepthStencils[i] = std::make_unique<ShaderInputDepthStencil>
						(gfx, 1u, 1u, 15u + i, DepthStencil::Usage::ShadowDepth, DepthStencil::Type::Cube);
				SetDepthBuffer(shadowDepthStencils[i]);
				RegisterSource(DirectBindableSource<ShaderInputDepthStencil>::Make("pMap" + std::to_string(i), shadowDepthStencils[i]));
			}
		}
		void Execute( Graphics& gfx ) const noxnd override
		{
			SetDepthBuffer(shadowDepthStencil);
			depthStencil->Clear( gfx );
			pDShadowCamera->BindToGraphics( gfx );
			RenderQueuePass::Execute( gfx );
			
			using namespace DirectX;
			gfx.SetProjection(projmatrix);
			for (unsigned char i = 0; i < pPShadowCameras.size(); i++)
			{
				const auto _pos = pPShadowCameras[i]->GetPos();
				const auto pos = XMLoadFloat3(&_pos);
				SetDepthBuffer(shadowDepthStencils[i]);
				depthStencil->Clear(gfx);
				for (unsigned char j = 0; j < 6; j++)
				{
					depthStencil->targetIndex = j;
					const auto lookAt = pos + cameraDirections[j];
					gfx.SetCamera(XMMatrixLookAtLH(pos, lookAt, cameraUps[j]));
					RenderQueuePass::Execute(gfx);
				}
			}
			//RegisterSource(DirectBindableSource<Bind::DepthStencil>::Make("dMap", depthStencil));
		}
		void DumpShadowMap( Graphics& gfx,const std::string& path ) const
		{
			//for( size_t i = 0; i < 6; i++ )
			//{
			//	auto d = pDepthCube->GetDepthBuffer( i );
			//	d->ToSurface( gfx ).Save( path + std::to_string( i ) + ".png" );
			//}
			depthStencil->ToSurface(gfx).Save(path);
		}
	private:
		const Camera* pDShadowCamera = nullptr;
		void SetDepthBuffer( std::shared_ptr<Bind::DepthStencil> ds ) const
		{
			const_cast<ShadowMappingPass*>(this)->depthStencil = std::move( ds );
		}
		std::vector<std::shared_ptr<PointLight>> pPShadowCameras;
		std::shared_ptr<Bind::ShaderInputDepthStencil> shadowDepthStencil;
		std::shared_ptr<Bind::ShaderInputDepthStencil> shadowDepthStencils[3];
		dx::XMVECTOR cameraDirections[6] =
		{
			{ 1.0f,0.0f,0.0f },
			{ -1.0f,0.0f,0.0f },
			{ 0.0f,1.0f,0.0f },
			{ 0.0f,-1.0f,0.0f },
			{ 0.0f,0.0f,1.0f },
			{ 0.0f,0.0f,-1.0f }
		};
		dx::XMVECTOR cameraUps[6] =
		{
			{ 0.0f,1.0f,0.0f },
			{ 0.0f,1.0f,0.0f },
			{ 0.0f,0.0f,-1.0f },
			{ 0.0f,0.0f,1.0f },
			{ 0.0f,1.0f,0.0f },
			{ 0.0f,1.0f,0.0f }
		};
		dx::XMMATRIX projmatrix = dx::XMMatrixPerspectiveFovLH(PI / 2.0f, 1.0f, 0.5f, 100.0f);
	};
}