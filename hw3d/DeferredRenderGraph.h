#pragma once
#include "RenderGraph.h"
#include <memory>
#include "ConstantBuffersEx.h"
#include "RenderTarget.h"
#include "PreCalculateRenderGraph.h"
#include "PointLight.h"
#include "SkyAtmosphereCommon.h"

class Graphics;
class Camera;
namespace Bind
{
	class Bindable;
	class RenderTarget;
	class ShadowSampler;
	class ShadowRasterizer;
}

namespace Rgph
{
	class DeferredRenderGraph : public RenderGraph
	{
	public:
		DeferredRenderGraph(Graphics& gfx);
		void RenderWindows(Graphics& gfx);
		void DumpShadowMap(Graphics& gfx, const std::string& path);
		void BindMainCamera(Camera& cam);
		void BindShadowCamera(Graphics& gfx, Camera& dCam, std::vector<std::shared_ptr<PointLight>> pCams);
		void StoreDepth(Graphics& gfx, const std::string& path);
	private:
		// private functions
		void RenderKernelWindow(Graphics& gfx);
		void RenderShadowWindow(Graphics& gfx);
		void SetKernelGauss(int radius, float sigma) noxnd;
		void SetKernelBox(int radius) noxnd;
		void RenderWaterWindow(Graphics& gfx);
		void RenderAOWindow(Graphics& gfx);
		void RenderBloomWindow(Graphics& gfx);
		//void RenderVolumeWindow(Graphics& gfx);
		void RenderSkyWindow(Graphics& gfx);
		// private data
		enum class KernelType
		{
			Gauss,
			Box,
		} kernelType = KernelType::Gauss;
		static constexpr int maxRadius = 7;
		int radius = 7;
		float sigma = 10.0f;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> blurKernel;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> blurDirection;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> shadowControl;
		std::shared_ptr<Bind::ShadowSampler> shadowSampler;
		std::shared_ptr<Bind::ShadowRasterizer> shadowRasterizer;
		std::unique_ptr<Rgph::PreCalculateRenderGraph> prg;
		std::shared_ptr<Bind::CachingVertexConstantBufferEx> waterFlowVS;
		std::shared_ptr<Bind::CachingDomainConstantBufferEx> waterFlowDS;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> waterRipple;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> TAAIndex;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> AOParams;
		float HAOMaxViewDepth = 1.0f;
		float HAOSharpness = 10.0f;
		float HAORadius = 0.95f;
		float HAOBackgroundViewDepth = -1.0f;
		float HAOForegroundViewDepth = 3.873f;
		float HAOBias = 0.01f;
		float HAOSmallScaleAO = 2.0f;
		float HAOLargeScaleAO = 2.0f;
		float HAOPowerExponent = 1.0f;
		float HAOForegroundSharpnessScale = 100.0f;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> bloomParams;
		float bloomThreshold = 0.5f;
		int bloomQuality = 6;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> volumeParams;
		std::unique_ptr<SkyAtmosphereCommon> skyAtmosphereParams;
	};
}
