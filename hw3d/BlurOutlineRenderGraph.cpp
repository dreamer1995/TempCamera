#include "BlurOutlineRenderGraph.h"
#include "BufferClearPass.h"
#include "LambertianPass.h"
#include "OutlineDrawingPass.h"
#include "OutlineMaskGenerationPass.h"
#include "Source.h"
#include "HorizontalBlurPass.h"
#include "VerticalBlurPass.h"
#include "BlurOutlineDrawingPass.h"
#include "WireframePass.h"
#include "ShadowMappingPass.h"
#include "DynamicConstant.h"
#include "imgui/imgui.h"
#include "ChiliMath.h"
#include "ShadowSampler.h"
#include "ShadowRasterizer.h"
#include "EnvironmentPass.h"
#include "WaterPre.h"
#include "LambertianPass_Water.h"
#include "WaterCaustics.h"

namespace Rgph
{
	BlurOutlineRenderGraph::BlurOutlineRenderGraph( Graphics& gfx )
		:
		RenderGraph( gfx )
	{
		const std::filesystem::path envFile = "Images\\EpicQuadPanorama_CC+EV1.jpg";;
		unsigned char checkCode = PreCalculateRenderGraph::CheckPreMapAvailability(envFile);
		if (checkCode > 0)
			prg = std::make_unique<Rgph::PreCalculateRenderGraph>(gfx, envFile, checkCode);
		std::shared_ptr<Bind::TextureCube> pPreCalSimpleCube = TextureCube::Resolve(gfx, "Images\\PreCal\\" + envFile.stem().string() + "#0");
		std::shared_ptr<Bind::TextureCube> pPreCalBlurCube = TextureCube::Resolve(gfx, "Images\\PreCal\\" + envFile.stem().string() + "#1", 11u);
		std::shared_ptr<Bind::TextureCube> pPreCalMipCube = TextureCube::Resolve(gfx, "Images\\PreCal\\" + envFile.stem().string() + "#2", 12u, true);
		std::shared_ptr<Bind::Texture> pPreCalLUTPlane = Texture::Resolve(gfx, "Images\\PreCal\\IBLBRDFLUT.png", 13u);
		AddGlobalSource(DirectBindableSource<Bind::TextureCube>::Make("cubeMap",      pPreCalSimpleCube));
		AddGlobalSource(DirectBindableSource<Bind::TextureCube>::Make("cubeMapBlur",  pPreCalBlurCube));
		AddGlobalSource(DirectBindableSource<Bind::TextureCube>::Make("cubeMapMip",   pPreCalMipCube));
		AddGlobalSource(DirectBindableSource<Bind::Texture>::Make("planeBRDFLUT", pPreCalLUTPlane));
		{
			auto pass = std::make_unique<BufferClearPass>( "clearRT" );
			pass->SetSinkLinkage( "buffer","$.backbuffer" );
			AppendPass( std::move( pass ) );
		}
		{
			auto pass = std::make_unique<BufferClearPass>( "clearDS" );
			pass->SetSinkLinkage( "buffer","$.masterDepth" );
			AppendPass( std::move( pass ) );
		}

		// setup shadow rasterizer
		{
			shadowRasterizer = std::make_shared<Bind::ShadowRasterizer>( gfx,10000,0.0005f,1.0f );
			AddGlobalSource( DirectBindableSource<Bind::ShadowRasterizer>::Make( "shadowRasterizer",shadowRasterizer ) );
		}

		{
			auto pass = std::make_unique<ShadowMappingPass>( gfx,"shadowMap" );
			pass->SetSinkLinkage( "shadowRasterizer","$.shadowRasterizer" );
			AppendPass( std::move( pass ) );
		}

		// setup shadow control buffer and sampler
		{
			{
				Dcb::RawLayout l;
				l.Add<Dcb::Integer>( "pcfLevel" );
				l.Add<Dcb::Float>( "depthBias" );
				l.Add<Dcb::Bool>( "hwPcf" );
				Dcb::Buffer buf{ std::move( l ) };
				buf["pcfLevel"] = 0;
				buf["depthBias"] = 0.0005f;
				buf["hwPcf"] = true;
				shadowControl = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 6u);
				AddGlobalSource( DirectBindableSource<Bind::CachingPixelConstantBufferEx>::Make( "shadowControl",shadowControl ) );
			}
			{
				shadowSampler = std::make_shared<Bind::ShadowSampler>(gfx, 2u);
				AddGlobalSource( DirectBindableSource<Bind::ShadowSampler>::Make( "shadowSampler",shadowSampler ) );
			}
		}
		
		{
			auto pass = std::make_unique<LambertianPass>( gfx,"lambertian" );
			pass->SetSinkLinkage( "dShadowMap","shadowMap.dMap" );
			pass->SetSinkLinkage("cubeMapBlurIn", "$.cubeMapBlur");
			pass->SetSinkLinkage("cubeMapMipIn", "$.cubeMapMip");
			pass->SetSinkLinkage("planeBRDFLUTIn", "$.planeBRDFLUT");
			pass->SetSinkLinkage( "renderTarget","clearRT.buffer" );
			pass->SetSinkLinkage( "depthStencil","clearDS.buffer" );
			pass->SetSinkLinkage( "shadowControl","$.shadowControl" );
			pass->SetSinkLinkage( "shadowSampler","$.shadowSampler" );
			AppendPass( std::move( pass ) );
		}
		{
			auto pass = std::make_unique<EnvironmentPass>(gfx, "environment");
			pass->SetSinkLinkage("cubeMapIn", "$.cubeMap");
			pass->SetSinkLinkage("renderTarget", "lambertian.renderTarget");
			pass->SetSinkLinkage("depthStencil", "lambertian.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			namespace dx = DirectX;
			Dcb::RawLayout l;
			l.Add<Dcb::Float4>("amplitude");
			l.Add<Dcb::Float4>("wavespeed");
			l.Add<Dcb::Float4>("wavelength");
			l.Add<Dcb::Float4>("omega");
			l.Add<Dcb::Float4>("Q");
			l.Add<Dcb::Float4>("directionX");
			l.Add<Dcb::Float4>("directionZ");
			Dcb::Buffer buf{ std::move(l) };
			buf["amplitude"] = dx::XMFLOAT4{ 0.071f,0.032f,0.048f,0.063f };
			buf["wavespeed"] = dx::XMFLOAT4{ 0.097f,0.258f,0.179f,0.219f };
			dx::XMFLOAT4 wavelength = { 0.887f,0.774f,0.790f,0.844f };
			buf["wavelength"] = wavelength;
			buf["omega"] = dx::XMFLOAT4{ 2 * PI / wavelength.x,2 * PI / wavelength.y,2 * PI / wavelength.z,2 * PI / wavelength.w };
			buf["Q"] = dx::XMFLOAT4{ 1.0f,0.871f,0.935f,0.844f };
			buf["directionX"] = dx::XMFLOAT4{ 0.0f,0.113f,0.306f,0.281f };
			buf["directionZ"] = dx::XMFLOAT4{ 0.629f,0.081f,0.484f,0.156f };
			
			waterFlowVS = std::make_shared<Bind::CachingVertexConstantBufferEx>(gfx, buf, 10u);
			waterFlowDS = std::make_shared<Bind::CachingDomainConstantBufferEx>(gfx, buf, 10u);
			AddGlobalSource(DirectBindableSource<Bind::CachingVertexConstantBufferEx>::Make("waterFlowVS", waterFlowVS));
			AddGlobalSource(DirectBindableSource<Bind::CachingDomainConstantBufferEx>::Make("waterFlowDS", waterFlowDS));
		}
		{
			Dcb::RawLayout lay;
			lay.Add<Dcb::Float>("speed");
			lay.Add<Dcb::Float>("roughness");
			lay.Add<Dcb::Float>("flatten1");
			lay.Add<Dcb::Float>("flatten2");
			lay.Add<Dcb::Bool>("normalMappingEnabled");
			auto buf = Dcb::Buffer(std::move(lay));
			buf["speed"] = 0.25f;
			buf["roughness"] = 0.321f;
			buf["flatten1"] = 0.182f;
			buf["flatten2"] = 0.0f;
			buf["normalMappingEnabled"] = true;
			waterRipple = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
			AddGlobalSource(DirectBindableSource<Bind::CachingPixelConstantBufferEx>::Make("waterRipple", waterRipple));
		}
		{
			auto pass = std::make_unique<WaterPrePass>(gfx, "waterPre", gfx.GetWidth());
			pass->SetSinkLinkage("waterFlow", "$.waterFlowVS");
			pass->SetSinkLinkage("waterRipple", "$.waterRipple");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<WaterCaustics>(gfx, "waterCaustic", gfx.GetWidth());
			pass->SetSinkLinkage("waterPreMap", "waterPre.waterPreOut");
			pass->SetSinkLinkage("waterFlow", "$.waterFlowDS");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<LambertianPass_Water>(gfx, "water");
			pass->SetSinkLinkage("waterFlow", "$.waterFlowVS");
			pass->SetSinkLinkage("waterRipple", "$.waterRipple");
			pass->SetSinkLinkage("dShadowMap", "shadowMap.dMap");
			pass->SetSinkLinkage("cubeMapBlurIn", "$.cubeMapBlur");
			pass->SetSinkLinkage("cubeMapMipIn", "$.cubeMapMip");
			pass->SetSinkLinkage("planeBRDFLUTIn", "$.planeBRDFLUT");
			pass->SetSinkLinkage("waterCausticMap", "waterCaustic.waterCausticOut");
			pass->SetSinkLinkage("renderTarget", "environment.renderTarget");
			pass->SetSinkLinkage("depthStencil", "environment.depthStencil");
			pass->SetSinkLinkage("shadowControl", "$.shadowControl");
			pass->SetSinkLinkage("shadowSampler", "$.shadowSampler");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<OutlineMaskGenerationPass>( gfx,"outlineMask" );
			pass->SetSinkLinkage( "depthStencil","water.depthStencil" );
			AppendPass( std::move( pass ) );
		}

		// setup blur constant buffers
		{
			{
				Dcb::RawLayout l;
				l.Add<Dcb::Integer>( "nTaps" );
				l.Add<Dcb::Array>( "coefficients" );
				l["coefficients"].Set<Dcb::Float>( maxRadius * 2 + 1 );
				Dcb::Buffer buf{ std::move( l ) };
				blurKernel = std::make_shared<Bind::CachingPixelConstantBufferEx>( gfx,buf,10u );
				SetKernelGauss( radius,sigma );
				AddGlobalSource( DirectBindableSource<Bind::CachingPixelConstantBufferEx>::Make( "blurKernel",blurKernel ) );
			}
			{
				Dcb::RawLayout l;
				l.Add<Dcb::Bool>( "isHorizontal" );
				Dcb::Buffer buf{ std::move( l ) };
				blurDirection = std::make_shared<Bind::CachingPixelConstantBufferEx>( gfx,buf,11u );
				AddGlobalSource( DirectBindableSource<Bind::CachingPixelConstantBufferEx>::Make( "blurDirection",blurDirection ) );
			}
		}

		{
			auto pass = std::make_unique<BlurOutlineDrawingPass>( gfx,"outlineDraw",gfx.GetWidth(),gfx.GetHeight() );
			AppendPass( std::move( pass ) );
		}
		{
			auto pass = std::make_unique<HorizontalBlurPass>( "horizontal",gfx,gfx.GetWidth(),gfx.GetHeight() );
			pass->SetSinkLinkage( "scratchIn","outlineDraw.scratchOut" );
			pass->SetSinkLinkage( "kernel","$.blurKernel" );
			pass->SetSinkLinkage( "direction","$.blurDirection" );
			AppendPass( std::move( pass ) );
		}
		{
			auto pass = std::make_unique<VerticalBlurPass>( "vertical",gfx );
			pass->SetSinkLinkage( "renderTarget","water.renderTarget" );
			pass->SetSinkLinkage( "depthStencil","outlineMask.depthStencil" );
			pass->SetSinkLinkage( "scratchIn","horizontal.scratchOut" );
			pass->SetSinkLinkage( "kernel","$.blurKernel" );
			pass->SetSinkLinkage( "direction","$.blurDirection" );
			AppendPass( std::move( pass ) );
		}
		{
			auto pass = std::make_unique<WireframePass>( gfx,"wireframe" );
			pass->SetSinkLinkage( "renderTarget","vertical.renderTarget" );
			pass->SetSinkLinkage( "depthStencil","vertical.depthStencil" );
			AppendPass( std::move( pass ) );
		}
		SetSinkTarget( "backbuffer","wireframe.renderTarget" );
		Finalize();
	}

	void BlurOutlineRenderGraph::SetKernelGauss( int radius,float sigma ) noxnd
	{
		assert( radius <= maxRadius );
		auto k = blurKernel->GetBuffer();
		const int nTaps = radius * 2 + 1;
		k["nTaps"] = nTaps;
		float sum = 0.0f;
		for( int i = 0; i < nTaps; i++ )
		{
			const auto x = float( i - radius );
			const auto g = gauss( x,sigma );
			sum += g;
			k["coefficients"][i] = g;
		}
		for( int i = 0; i < nTaps; i++ )
		{
			k["coefficients"][i] = (float)k["coefficients"][i] / sum;
		}
		blurKernel->SetBuffer( k );
	}
	
	void BlurOutlineRenderGraph::SetKernelBox( int radius ) noxnd
	{
		assert( radius <= maxRadius );
		auto k = blurKernel->GetBuffer();
		const int nTaps = radius * 2 + 1;
		k["nTaps"] = nTaps;
		const float c = 1.0f / nTaps;
		for( int i = 0; i < nTaps; i++ )
		{
			k["coefficients"][i] = c;
		}
		blurKernel->SetBuffer( k );
	}
	
	void BlurOutlineRenderGraph::RenderWindows( Graphics& gfx )
	{
		RenderShadowWindow( gfx );
		RenderKernelWindow( gfx );
		// RenderWaterWindow(gfx);
	}

	void BlurOutlineRenderGraph::RenderKernelWindow( Graphics& gfx )
	{
		if( ImGui::Begin( "Kernel" ) )
		{
			bool filterChanged = false;
			{
				const char* items[] = { "Gauss","Box" };
				static const char* curItem = items[0];
				if( ImGui::BeginCombo( "Filter Type",curItem ) )
				{
					for( int n = 0; n < std::size( items ); n++ )
					{
						const bool isSelected = (curItem == items[n]);
						if( ImGui::Selectable( items[n],isSelected ) )
						{
							filterChanged = true;
							curItem = items[n];
							if( curItem == items[0] )
							{
								kernelType = KernelType::Gauss;
							}
							else if( curItem == items[1] )
							{
								kernelType = KernelType::Box;
							}
						}
						if( isSelected )
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			bool radChange = ImGui::SliderInt( "Radius",&radius,0,maxRadius );
			bool sigChange = ImGui::SliderFloat( "Sigma",&sigma,0.1f,10.0f );
			if( radChange || sigChange || filterChanged )
			{
				if( kernelType == KernelType::Gauss )
				{
					SetKernelGauss( radius,sigma );
				}
				else if( kernelType == KernelType::Box )
				{
					SetKernelBox( radius );
				}
			}
		}
		ImGui::End();
	}

	void BlurOutlineRenderGraph::RenderShadowWindow( Graphics& gfx )
	{
		if( ImGui::Begin( "Shadows" ) )
		{
			auto ctrl = shadowControl->GetBuffer();
			bool bilin = shadowSampler->GetBilinear();

			bool pcfChange = ImGui::SliderInt( "PCF Level",&ctrl["pcfLevel"],0,4 );
			bool biasChange = ImGui::SliderFloat( "Post Bias",&ctrl["depthBias"],0.0f,0.1f,"%.6f",3.6f );
			bool hwPcfChange = ImGui::Checkbox( "HW PCF",&ctrl["hwPcf"] );
			ImGui::Checkbox( "Bilinear",&bilin );

			if( pcfChange || biasChange || hwPcfChange )
			{
				shadowControl->SetBuffer( ctrl );
			}

			shadowSampler->SetHwPcf( ctrl["hwPcf"] );
			shadowSampler->SetBilinear( bilin );

			{
				auto bias = shadowRasterizer->GetDepthBias();
				auto slope = shadowRasterizer->GetSlopeBias();
				auto clamp = shadowRasterizer->GetClamp();

				bool biasChange = ImGui::SliderInt( "Pre Bias",&bias,0,100000 );
				bool slopeChange = ImGui::SliderFloat( "Slope Bias",&slope,0.0f,100.0f,"%.4f",4.0f );
				bool clampChange = ImGui::SliderFloat( "Clamp",&clamp,0.0001f,0.5f,"%.4f",2.5f );

				if( biasChange || slopeChange || clampChange )
				{
					shadowRasterizer->ChangeDepthBiasParameters( gfx,bias,slope,clamp );
				}
			}
		}
		ImGui::End();
	}

	void BlurOutlineRenderGraph::RenderWaterWindow(Graphics& gfx)
	{
		if (ImGui::Begin("WaterWave"))
		{
			auto buf = waterFlowVS->GetBuffer();
			namespace dx = DirectX;
			float dirty = false;
			const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };
			bool lDirty = false;

			if (auto v = buf["amplitude"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat4("Amplitude", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT4&>(v)), 0.0f, 1.0f, "%.3f", 1.0f));
			}
			if (auto v = buf["wavespeed"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat4("Wavespeed", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT4&>(v)), 0.0f, 1.0f, "%.3f", 1.0f));
			}
			if (auto v = buf["wavelength"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat4("Wavelength", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT4&>(v)), 0.0f, 1.0f, "%.3f", 1.0f));
			}
			if (auto v = buf["omega"]; v.Exists())
			{
				lDirty = ImGui::SliderFloat4("Omega", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT4&>(v)), 0.0f, 1.0f, "%.3f", 1.0f);
				dcheck(lDirty);
			}
			if (auto v = buf["Q"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat4("Q", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT4&>(v)), 0.0f, 1.0f, "%.3f", 1.0f));
			}
			if (auto v = buf["directionX"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat4("DirectionX", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT4&>(v)), 0.0f, 1.0f, "%.3f", 1.0f));
			}
			if (auto v = buf["directionZ"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat4("DirectionY", reinterpret_cast<float*>(&static_cast<dx::XMFLOAT4&>(v)), 0.0f, 1.0f, "%.3f", 1.0f));
			}

			if (dirty)
			{
				if (lDirty)
				{
					dx::XMFLOAT4 wavelength = buf["wavelength"];
					buf["omega"] = dx::XMFLOAT4{ 2 * PI / wavelength.x,2 * PI / wavelength.y,2 * PI / wavelength.z,2 * PI / wavelength.w };
				}
				waterFlowVS->SetBuffer(buf);
				waterFlowDS->SetBuffer(buf);
			}
		}
		ImGui::End();
		if (ImGui::Begin("WaterRipple"))
		{
			auto buf = waterRipple->GetBuffer();
			namespace dx = DirectX;
			float dirty = false;
			const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };

			if (auto v = buf["speed"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat("Speed", &v, 0.0f, 10.0f, "%.3f", 1.0f));
			}
			if (auto v = buf["roughness"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat("Roughness", &v, 0.0f, 1.0f, "%.3f", 1.0f));
			}
			if (auto v = buf["flatten1"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat("Flatten1", &v, 0.0f, 1.0f, "%.3f", 1.0f));
			}
			if (auto v = buf["flatten2"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat("Flatten2", &v, 0.0f, 1.0f, "%.3f", 1.0f));
			}
			if (auto v = buf["normalMappingEnabled"]; v.Exists())
			{
				dcheck(ImGui::Checkbox("Normal Map Enable", &v));
			}

			if (dirty)
			{
				waterRipple->SetBuffer(buf);
			}
		}
		ImGui::End();
	}

	void Rgph::BlurOutlineRenderGraph::DumpShadowMap( Graphics & gfx,const std::string & path )
	{
		dynamic_cast<ShadowMappingPass&>(FindPassByName( "shadowMap" )).DumpShadowMap( gfx,path );
	}
	void Rgph::BlurOutlineRenderGraph::BindMainCamera( Camera& cam )
	{
		dynamic_cast<EnvironmentPass&>(FindPassByName("environment")).BindMainCamera(cam);
		dynamic_cast<LambertianPass&>(FindPassByName( "lambertian" )).BindMainCamera( cam );
		dynamic_cast<LambertianPass_Water&>(FindPassByName("water")).BindMainCamera(cam);
	}
	void Rgph::BlurOutlineRenderGraph::BindShadowCamera(Graphics& gfx, Camera& dCam, std::vector<std::shared_ptr<Camera>> pCams)
	{
		dynamic_cast<ShadowMappingPass&>(FindPassByName( "shadowMap" )).BindShadowCamera(dCam, pCams);
		dynamic_cast<LambertianPass&>(FindPassByName( "lambertian" )).BindShadowCamera(gfx, dCam, pCams);
		dynamic_cast<LambertianPass_Water&>(FindPassByName("water")).BindShadowCamera(gfx, dCam, pCams);
	}
}
