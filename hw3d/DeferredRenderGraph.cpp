#include "DeferredRenderGraph.h"
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
#include "GBufferPass.h"
#include "DebugDeferredPass.h"
#include "DeferredSunLightPass.h"
#include "DeferredPointLightPass.h"
#include "DeferredTAAPass.h"
#include "DeferredHDRPass.h"
#include "DeferredHBAOPass.h"
#include "DeferredHBAOBlurPass.h"
#include "DeferredBloomCatchPass.h"
#include "DeferredBloomBlurPass.h"
#include "DeferredBloomMergePass.h"

namespace Rgph
{
	DeferredRenderGraph::DeferredRenderGraph(Graphics& gfx)
		:
		RenderGraph(gfx)
	{
		const std::filesystem::path envFile = "Images\\secluded_beach_4k.jpg";;
		unsigned char checkCode = PreCalculateRenderGraph::CheckPreMapAvailability(envFile);
		if (checkCode > 0)
			prg = std::make_unique<Rgph::PreCalculateRenderGraph>(gfx, envFile, checkCode);
		std::shared_ptr<Bind::TextureCube> pPreCalSimpleCube = TextureCube::Resolve(gfx, "Images\\PreCal\\" + envFile.stem().string() + "#0");
		std::shared_ptr<Bind::TextureCube> pPreCalBlurCube = TextureCube::Resolve(gfx, "Images\\PreCal\\" + envFile.stem().string() + "#1", 11u);
		std::shared_ptr<Bind::TextureCube> pPreCalMipCube = TextureCube::Resolve(gfx, "Images\\PreCal\\" + envFile.stem().string() + "#2", 12u, true);
		std::shared_ptr<Bind::Texture> pPreCalLUTPlane = Texture::Resolve(gfx, "Images\\PreCal\\IBLBRDFLUT.png", 13u);
		AddGlobalSource(DirectBindableSource<Bind::TextureCube>::Make("cubeMap", pPreCalSimpleCube));
		AddGlobalSource(DirectBindableSource<Bind::TextureCube>::Make("cubeMapBlur", pPreCalBlurCube));
		AddGlobalSource(DirectBindableSource<Bind::TextureCube>::Make("cubeMapMip", pPreCalMipCube));
		AddGlobalSource(DirectBindableSource<Bind::Texture>::Make("planeBRDFLUT", pPreCalLUTPlane));
		{
			auto pass = std::make_unique<BufferClearPass>("clearRT");
			pass->SetSinkLinkage("buffer", "$.backbuffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<BufferClearPass>("clearDS");
			pass->SetSinkLinkage("buffer", "$.masterDepth");
			AppendPass(std::move(pass));
		}

		// setup shadow rasterizer
		{
			shadowRasterizer = std::make_shared<Bind::ShadowRasterizer>(gfx, 10000, 5.0f, 1.0f);
			AddGlobalSource(DirectBindableSource<Bind::ShadowRasterizer>::Make("shadowRasterizer", shadowRasterizer));
		}

		{
			auto pass = std::make_unique<ShadowMappingPass>(gfx, "shadowMap");
			pass->SetSinkLinkage("shadowRasterizer", "$.shadowRasterizer");
			AppendPass(std::move(pass));
		}

		// setup shadow control buffer and sampler
		{
			{
				Dcb::RawLayout l;
				l.Add<Dcb::Integer>("pcfLevel");
				l.Add<Dcb::Float>("depthBias");
				l.Add<Dcb::Bool>("hwPcf");
				l.Add<Dcb::Float>("cubeShadowBaseOffset");
				Dcb::Buffer buf{ std::move(l) };
				buf["pcfLevel"] = 4;
				buf["depthBias"] = 0.0005f;
				buf["hwPcf"] = true;
				buf["cubeShadowBaseOffset"] = 0.036252f;
				shadowControl = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 6u);
				AddGlobalSource(DirectBindableSource<Bind::CachingPixelConstantBufferEx>::Make("shadowControl", shadowControl));
			}
			{
				shadowSampler = std::make_shared<Bind::ShadowSampler>(gfx, 2u);
				AddGlobalSource(DirectBindableSource<Bind::ShadowSampler>::Make("shadowSampler", shadowSampler));
			}
		}

		{
			Dcb::RawLayout l;
			l.Add<Dcb::Integer>("TAAIndex");
			Dcb::Buffer buf{ std::move(l) };
			TAAIndex = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
			AddGlobalSource(DirectBindableSource<Bind::CachingPixelConstantBufferEx>::Make("TAAIndex", TAAIndex));
		}
		{
			auto pass = std::make_unique<GbufferPass>(gfx, "gbuffer", gfx.GetWidth(), gfx.GetHeight());
			pass->SetSinkLinkage("TAAIndex", "$.TAAIndex");
			pass->SetSinkLinkage("depthStencil", "clearDS.buffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<DeferredSunLightPass>("deferredSunLighting", gfx, gfx.GetWidth(), gfx.GetHeight(), masterDepth);
			pass->SetSinkLinkage("dShadowMap", "shadowMap.dMap");
			pass->SetSinkLinkage("cubeMapBlurIn", "$.cubeMapBlur");
			pass->SetSinkLinkage("cubeMapMipIn", "$.cubeMapMip");
			pass->SetSinkLinkage("planeBRDFLUTIn", "$.planeBRDFLUT");
			pass->SetSinkLinkage("gbufferIn", "gbuffer.gbufferOut");
			pass->SetSinkLinkage("shadowControl", "$.shadowControl");
			pass->SetSinkLinkage("shadowSampler", "$.shadowSampler");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<DeferredPointLightPass>("deferredPointLighting", gfx, masterDepth);
			pass->SetSinkLinkage("pShadowMap0", "shadowMap.pMap0");
			pass->SetSinkLinkage("pShadowMap1", "shadowMap.pMap1");
			pass->SetSinkLinkage("pShadowMap2", "shadowMap.pMap2");
			pass->SetSinkLinkage("gbufferIn", "gbuffer.gbufferOut");
			pass->SetSinkLinkage("renderTarget", "deferredSunLighting.renderTarget");
			pass->SetSinkLinkage("shadowControl", "$.shadowControl");
			pass->SetSinkLinkage("shadowSampler", "$.shadowSampler");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<LambertianPass>(gfx, "lambertian");
			pass->SetSinkLinkage("dShadowMap", "shadowMap.dMap");
			pass->SetSinkLinkage("pShadowMap0", "shadowMap.pMap0");
			pass->SetSinkLinkage("pShadowMap1", "shadowMap.pMap1");
			pass->SetSinkLinkage("pShadowMap2", "shadowMap.pMap2");
			pass->SetSinkLinkage("cubeMapBlurIn", "$.cubeMapBlur");
			pass->SetSinkLinkage("cubeMapMipIn", "$.cubeMapMip");
			pass->SetSinkLinkage("planeBRDFLUTIn", "$.planeBRDFLUT");
			pass->SetSinkLinkage("renderTarget", "deferredSunLighting.renderTarget");
			pass->SetSinkLinkage("depthStencil", "gbuffer.depthStencil");
			pass->SetSinkLinkage("shadowControl", "$.shadowControl");
			pass->SetSinkLinkage("shadowSampler", "$.shadowSampler");
			AppendPass(std::move(pass));
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
			pass->SetSinkLinkage("pShadowMap0", "shadowMap.pMap0");
			pass->SetSinkLinkage("pShadowMap1", "shadowMap.pMap1");
			pass->SetSinkLinkage("pShadowMap2", "shadowMap.pMap2");
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
			Dcb::RawLayout l;
			l.Add<Dcb::Float>("ViewDepthThresholdNegInv");
			l.Add<Dcb::Float>("ViewDepthThresholdSharpness");
			l.Add<Dcb::Float>("NegInvR2");
			l.Add<Dcb::Float>("RadiusToScreen");
			l.Add<Dcb::Float>("BackgroundAORadiusPixels");
			l.Add<Dcb::Float>("ForegroundAORadiusPixels");
			l.Add<Dcb::Float>("NDotVBias");
			l.Add<Dcb::Float2>("AORes");
			l.Add<Dcb::Float2>("InvAORes");
			l.Add<Dcb::Float>("SmallScaleAOAmount");
			l.Add<Dcb::Float>("LargeScaleAOAmount");
			l.Add<Dcb::Float>("PowerExponent");
			l.Add<Dcb::Float>("BlurViewDepth0");
			l.Add<Dcb::Float>("BlurViewDepth1");
			l.Add<Dcb::Float>("BlurSharpness0");
			l.Add<Dcb::Float>("BlurSharpness1");
			Dcb::Buffer buf{ std::move(l) };
			AOParams = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
			AddGlobalSource(DirectBindableSource<Bind::CachingPixelConstantBufferEx>::Make("AOParams", AOParams));
		}
		{
			auto pass = std::make_unique<DeferredHBAOPass>("HBAO", gfx, gfx.GetWidth(), gfx.GetHeight(), masterDepth);
			pass->SetSinkLinkage("AOParams", "$.AOParams");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<DeferredHBAOBlurPass>("HBAOBlur", gfx, gfx.GetWidth(), gfx.GetHeight(), masterDepth);
			pass->SetSinkLinkage("AOParams", "$.AOParams");
			pass->SetSinkLinkage("scratchIn", "HBAO.scratchOut");
			pass->SetSinkLinkage("renderTarget", "water.renderTarget");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<DeferredTAAPass>("TAA", gfx, gfx.GetWidth(), gfx.GetHeight(), masterDepth);
			pass->SetSinkLinkage("scratchIn", "HBAOBlur.renderTarget");
			AppendPass(std::move(pass));
		}
		{
			Dcb::RawLayout l;
			l.Add<Dcb::Float>("bloomThreshold");
			Dcb::Buffer buf{ std::move(l) };
			buf["bloomThreshold"] = bloomThreshold;
			bloomParams = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
			AddGlobalSource(DirectBindableSource<Bind::CachingPixelConstantBufferEx>::Make("bloomParams", bloomParams));
		}
		{
			auto pass = std::make_unique<DeferredBloomCatchPass>("bloomCatch", gfx, gfx.GetWidth(), gfx.GetHeight());
			pass->SetSinkLinkage("TAA0", "TAA.renderTarget0");
			pass->SetSinkLinkage("TAA1", "TAA.renderTarget1");
			pass->SetSinkLinkage("bloomParams", "$.bloomParams");
			AppendPass(std::move(pass));
		}
		{
			Dcb::RawLayout l;
			l.Add<Dcb::Integer>("nTaps");
			l.Add<Dcb::Array>("coefficients");
			l["coefficients"].Set<Dcb::Float>(maxRadius * 2 + 1);
			Dcb::Buffer buf{ std::move(l) };
			blurKernel = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
			SetKernelGauss(radius, sigma);
			AddGlobalSource(DirectBindableSource<Bind::CachingPixelConstantBufferEx>::Make("blurKernel", blurKernel));
		}
		{
			auto pass = std::make_unique<DeferredBloomBlurPass>("bloomBlur", gfx, gfx.GetWidth(), gfx.GetHeight(), bloomQuality);
			pass->SetSinkLinkage("kernel", "$.blurKernel");
			pass->SetSinkLinkage("scratchIn", "bloomCatch.scratchOut");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<DeferredBloomMergePass>("bloomMerge", gfx);
			pass->SetSinkLinkage("renderTarget", "bloomBlur.scratchOut");
			pass->SetSinkLinkage("TAA0", "TAA.renderTarget0");
			pass->SetSinkLinkage("TAA1", "TAA.renderTarget1");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<DeferredHDRPass>("HDR", gfx);
			pass->SetSinkLinkage("scratchIn", "bloomMerge.renderTarget");
			pass->SetSinkLinkage("renderTarget", "clearRT.buffer");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<OutlineMaskGenerationPass>(gfx, "outlineMask");
			pass->SetSinkLinkage("depthStencil", "water.depthStencil");
			AppendPass(std::move(pass));
		}

		// setup blur constant buffers	
		{
			Dcb::RawLayout l;
			l.Add<Dcb::Bool>("isHorizontal");
			Dcb::Buffer buf{ std::move(l) };
			blurDirection = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 11u);
			AddGlobalSource(DirectBindableSource<Bind::CachingPixelConstantBufferEx>::Make("blurDirection", blurDirection));
		}

		{
			auto pass = std::make_unique<BlurOutlineDrawingPass>(gfx, "outlineDraw", gfx.GetWidth(), gfx.GetHeight());
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<HorizontalBlurPass>("horizontal", gfx, gfx.GetWidth(), gfx.GetHeight());
			pass->SetSinkLinkage("scratchIn", "outlineDraw.scratchOut");
			pass->SetSinkLinkage("kernel", "$.blurKernel");
			pass->SetSinkLinkage("direction", "$.blurDirection");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<VerticalBlurPass>("vertical", gfx);
			pass->SetSinkLinkage("renderTarget", "HDR.renderTarget");
			pass->SetSinkLinkage("depthStencil", "outlineMask.depthStencil");
			pass->SetSinkLinkage("scratchIn", "horizontal.scratchOut");
			pass->SetSinkLinkage("kernel", "$.blurKernel");
			pass->SetSinkLinkage("direction", "$.blurDirection");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<WireframePass>(gfx, "wireframe");
			pass->SetSinkLinkage("renderTarget", "vertical.renderTarget");
			pass->SetSinkLinkage("depthStencil", "vertical.depthStencil");
			AppendPass(std::move(pass));
		}
		{
			auto pass = std::make_unique<DebugDeferredPass>("debugDeferred", gfx, masterDepth);
			pass->SetSinkLinkage("gbufferIn", "gbuffer.gbufferOut");
			pass->SetSinkLinkage("renderTarget", "wireframe.renderTarget");
			AppendPass(std::move(pass));
		}
		SetSinkTarget("backbuffer", "debugDeferred.renderTarget");
		Finalize();
	}

	void DeferredRenderGraph::SetKernelGauss(int radius, float sigma) noxnd
	{
		assert(radius <= maxRadius);
		auto k = blurKernel->GetBuffer();
		const int nTaps = radius * 2 + 1;
		k["nTaps"] = nTaps;
		float sum = 0.0f;
		for (int i = 0; i < nTaps; i++)
		{
			const auto x = float(i - radius);
			const auto g = gauss(x, sigma);
			sum += g;
			k["coefficients"][i] = g;
		}
		for (int i = 0; i < nTaps; i++)
		{
			k["coefficients"][i] = (float)k["coefficients"][i] / sum;
		}
		blurKernel->SetBuffer(k);
	}

	void DeferredRenderGraph::SetKernelBox(int radius) noxnd
	{
		assert(radius <= maxRadius);
		auto k = blurKernel->GetBuffer();
		const int nTaps = radius * 2 + 1;
		k["nTaps"] = nTaps;
		const float c = 1.0f / nTaps;
		for (int i = 0; i < nTaps; i++)
		{
			k["coefficients"][i] = c;
		}
		blurKernel->SetBuffer(k);
	}

	void DeferredRenderGraph::RenderWindows(Graphics& gfx)
	{
		RenderShadowWindow(gfx);
		RenderKernelWindow(gfx);
		// RenderWaterWindow(gfx);
		RenderAOWindow(gfx);
		RenderBloomWindow(gfx);
	}

	void DeferredRenderGraph::RenderKernelWindow(Graphics& gfx)
	{
		if (ImGui::Begin("Kernel"))
		{
			bool filterChanged = false;
			{
				const char* items[] = { "Gauss","Box" };
				static const char* curItem = items[0];
				if (ImGui::BeginCombo("Filter Type", curItem))
				{
					for (int n = 0; n < std::size(items); n++)
					{
						const bool isSelected = (curItem == items[n]);
						if (ImGui::Selectable(items[n], isSelected))
						{
							filterChanged = true;
							curItem = items[n];
							if (curItem == items[0])
							{
								kernelType = KernelType::Gauss;
							}
							else if (curItem == items[1])
							{
								kernelType = KernelType::Box;
							}
						}
						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}

			bool radChange = ImGui::SliderInt("Radius", &radius, 0, maxRadius);
			bool sigChange = ImGui::SliderFloat("Sigma", &sigma, 0.1f, 10.0f);
			if (radChange || sigChange || filterChanged)
			{
				if (kernelType == KernelType::Gauss)
				{
					SetKernelGauss(radius, sigma);
				}
				else if (kernelType == KernelType::Box)
				{
					SetKernelBox(radius);
				}
			}
		}
		ImGui::End();
	}

	void DeferredRenderGraph::RenderShadowWindow(Graphics& gfx)
	{
		if (ImGui::Begin("Shadows"))
		{
			auto ctrl = shadowControl->GetBuffer();
			bool bilin = shadowSampler->GetBilinear();

			bool pcfChange = ImGui::SliderInt("PCF Level", &ctrl["pcfLevel"], 0, 4);
			bool biasChange = ImGui::SliderFloat("Post Bias", &ctrl["depthBias"], 0.0f, 0.1f, "%.6f", 3.6f);
			bool hwPcfChange = ImGui::Checkbox("HW PCF", &ctrl["hwPcf"]);
			bool offsetChange = ImGui::SliderFloat("Base Offset", &ctrl["cubeShadowBaseOffset"], 0.0f, 0.1f, "%.6f", 3.6f);
			ImGui::Checkbox("Bilinear", &bilin);

			if (pcfChange || biasChange || hwPcfChange || offsetChange)
			{
				shadowControl->SetBuffer(ctrl);
			}

			shadowSampler->SetHwPcf(ctrl["hwPcf"]);
			shadowSampler->SetBilinear(bilin);

			{
				auto bias = shadowRasterizer->GetDepthBias();
				auto slope = shadowRasterizer->GetSlopeBias();
				auto clamp = shadowRasterizer->GetClamp();

				bool biasChange = ImGui::SliderInt("Pre Bias", &bias, 0, 100000);
				bool slopeChange = ImGui::SliderFloat("Slope Bias", &slope, 0.0f, 100.0f, "%.4f", 4.0f);
				bool clampChange = ImGui::SliderFloat("Clamp", &clamp, 0.0001f, 0.5f, "%.4f", 2.5f);

				if (biasChange || slopeChange || clampChange)
				{
					shadowRasterizer->ChangeDepthBiasParameters(gfx, bias, slope, clamp);
				}
			}

			if (ImGui::Button("Dump Cubemap"))
			{
				DumpShadowMap(gfx, "Dumps\\shadow_");
			}
		}
		ImGui::End();
	}

	void DeferredRenderGraph::RenderWaterWindow(Graphics& gfx)
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

	void DeferredRenderGraph::RenderAOWindow(Graphics& gfx)
	{
		if (!gfx.isHBAO)
			return;
		if (ImGui::Begin("HBAO+"))
		{
			auto buf = AOParams->GetBuffer();

			bool con1 = ImGui::SliderFloat("Max View Depth", &HAOMaxViewDepth, 0.001f, 1.0f);
			bool con2 = ImGui::SliderFloat("Sharpness", &HAOSharpness, 0.0f, 10.0f);
			bool con3 = ImGui::SliderFloat("Radius", &HAORadius, 0.001f, 50.0f);
			bool con4 = ImGui::SliderFloat("Background View Depth", &HAOBackgroundViewDepth, -1.0f, 200.0f);
			bool con5 = ImGui::SliderFloat("Foreground View Depth", &HAOForegroundViewDepth, -1.0f, 200.0f);
			bool con6 = ImGui::SliderFloat("Bias", &HAOBias, 0.01f, 1.0f);
			bool con7 = ImGui::SliderFloat("Small Scale AO", &HAOSmallScaleAO, 0.0f, 2.0f);
			bool con8 = ImGui::SliderFloat("Large Scale AO", &HAOLargeScaleAO, 0.0f, 2.0f);
			bool con9 = ImGui::SliderFloat("Power Exponent", &HAOPowerExponent, 0.0f, 10.0f);
			bool con10 = ImGui::SliderFloat("Foreground Sharpness Scale", &HAOForegroundSharpnessScale, 0.0f, 100.0f);

			static bool firstFrame = true;
			if (con1 || con2 || con3 || con4 || con5 || con6 || con7 || con8 || con9 || con10 || firstFrame)
			{
				if (firstFrame)
					firstFrame = false;
				buf["ViewDepthThresholdNegInv"] = -1.f / std::max(HAOMaxViewDepth, 1.e-6f);
				buf["ViewDepthThresholdSharpness"] = std::max(HAOSharpness, 0.f);
				const float R = std::max(HAORadius, 1.e-6f);
				buf["NegInvR2"] = -1.f / (R * R);
				float RadiusToScreen = R * 0.5f / gfx.GetFOV() * gfx.GetHeight();
				buf["RadiusToScreen"] = RadiusToScreen;
				buf["BackgroundAORadiusPixels"] = RadiusToScreen / HAOBackgroundViewDepth;
				buf["ForegroundAORadiusPixels"] = RadiusToScreen / HAOForegroundViewDepth;
				buf["NDotVBias"] = std::clamp(HAOBias, 0.0f, 1.0f);
				buf["AORes"] = DirectX::XMFLOAT2{ (float)gfx.GetWidth(),(float)gfx.GetHeight() };
				buf["InvAORes"] = DirectX::XMFLOAT2{ 1.0f / gfx.GetWidth(),1.0f / gfx.GetHeight() };
				const float AOAmountScaleFactor = 1.0f / (1.0f - HAOBias);
				buf["SmallScaleAOAmount"] = std::clamp(HAOSmallScaleAO, 0.0f, 2.0f) * AOAmountScaleFactor * 2.0f;
				buf["LargeScaleAOAmount"] = std::clamp(HAOLargeScaleAO, 0.0f, 2.0f) * AOAmountScaleFactor;
				buf["PowerExponent"] = HAOPowerExponent;
				const float fBlurViewDepth0 = std::max(HAOForegroundViewDepth, 0.f);
				buf["BlurViewDepth0"] = fBlurViewDepth0;
				buf["BlurViewDepth1"] = std::max(HAOBackgroundViewDepth, fBlurViewDepth0 + 1.e-6f);
				const float baseSharpness = std::max(HAOSharpness, 0.f);
				buf["BlurSharpness0"] = baseSharpness * std::max(HAOForegroundSharpnessScale, 0.f);
				buf["BlurSharpness1"] = baseSharpness;
				AOParams->SetBuffer(buf);
			}
		}
		ImGui::End();
	}

	void DeferredRenderGraph::RenderBloomWindow(Graphics& gfx)
	{
		if (!gfx.isHDR)
			return;
		if (ImGui::Begin("HDR"))
		{
			auto buf = bloomParams->GetBuffer();
			namespace dx = DirectX;
			float dirty = false;
			const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };

			if (auto v = buf["bloomThreshold"]; v.Exists())
			{
				dcheck(ImGui::SliderFloat("Bloom Threshold", &v, 0.0f, 10.0f, "%.3f", 1.0f));
			}
			ImGui::SliderInt("Bloom Quality", &bloomQuality, 1, 6);

			if (dirty)
			{
				bloomParams->SetBuffer(buf);
			}
		}
		ImGui::End();
	}

	void Rgph::DeferredRenderGraph::DumpShadowMap(Graphics& gfx, const std::string& path)
	{
		dynamic_cast<ShadowMappingPass&>(FindPassByName("shadowMap")).DumpShadowMap(gfx, path);
	}
	void Rgph::DeferredRenderGraph::BindMainCamera(Camera& cam)
	{
		dynamic_cast<EnvironmentPass&>(FindPassByName("environment")).BindMainCamera(cam);
		dynamic_cast<LambertianPass&>(FindPassByName("lambertian")).BindMainCamera(cam);
		dynamic_cast<LambertianPass_Water&>(FindPassByName("water")).BindMainCamera(cam);
		dynamic_cast<GbufferPass&>(FindPassByName("gbuffer")).BindMainCamera(cam);
		dynamic_cast<DeferredSunLightPass&>(FindPassByName("deferredSunLighting")).BindMainCamera(cam);
		dynamic_cast<DeferredPointLightPass&>(FindPassByName("deferredPointLighting")).BindMainCamera(cam);
		dynamic_cast<DeferredTAAPass&>(FindPassByName("TAA")).BindMainCamera(cam);
	}
	void Rgph::DeferredRenderGraph::BindShadowCamera(Graphics& gfx, Camera& dCam, std::vector<std::shared_ptr<PointLight>> pCams)
	{
		dynamic_cast<ShadowMappingPass&>(FindPassByName("shadowMap")).BindShadowCamera(gfx, dCam, pCams);
		dynamic_cast<LambertianPass&>(FindPassByName("lambertian")).BindShadowCamera(gfx, dCam, pCams);
		dynamic_cast<LambertianPass_Water&>(FindPassByName("water")).BindShadowCamera(gfx, dCam, pCams);
		dynamic_cast<DeferredSunLightPass&>(FindPassByName("deferredSunLighting")).BindShadowCamera(gfx, dCam);
		dynamic_cast<DeferredPointLightPass&>(FindPassByName("deferredPointLighting")).BindShadowCamera(gfx, pCams);
	}
}
