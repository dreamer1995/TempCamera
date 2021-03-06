#pragma once
#include "RenderGraph.h"
#include <memory>
#include "ConstantBuffersEx.h"
#include "RenderTarget.h"
#include "PreCalculateRenderGraph.h"
#include "PointLight.h"

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
	class BlurOutlineRenderGraph : public RenderGraph
	{
	public:
		BlurOutlineRenderGraph( Graphics& gfx );
		void RenderWindows( Graphics& gfx );
		void DumpShadowMap( Graphics& gfx,const std::string& path );
		void BindMainCamera( Camera& cam );
		void BindShadowCamera(Graphics& gfx, Camera& dCam, std::vector<std::shared_ptr<PointLight>> pCams);
		void StoreDepth( Graphics& gfx,const std::string& path );
	private:
		// private functions
		void RenderKernelWindow( Graphics& gfx );
		void RenderShadowWindow( Graphics& gfx );
		void SetKernelGauss( int radius,float sigma ) noxnd;
		void SetKernelBox( int radius ) noxnd;
		void RenderWaterWindow(Graphics& gfx);
		// private data
		enum class KernelType
		{
			Gauss,
			Box,
		} kernelType = KernelType::Gauss;
		static constexpr int maxRadius = 7;
		int radius = 4;
		float sigma = 2.0f;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> blurKernel;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> blurDirection;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> shadowControl;
		std::shared_ptr<Bind::ShadowSampler> shadowSampler;
		std::shared_ptr<Bind::ShadowRasterizer> shadowRasterizer;
		std::unique_ptr<Rgph::PreCalculateRenderGraph> prg;
		std::shared_ptr<Bind::CachingVertexConstantBufferEx> waterFlowVS;
		std::shared_ptr<Bind::CachingDomainConstantBufferEx> waterFlowDS;
		std::shared_ptr<Bind::CachingPixelConstantBufferEx> waterRipple;
	};
}
