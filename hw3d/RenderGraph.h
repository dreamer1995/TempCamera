#pragma once
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include "ConditionalNoexcept.h"

class Graphics;

namespace Bind
{
	class RenderTarget;
	class DepthStencil;
	class OutputOnlyDepthStencil;
}

namespace Rgph
{
	class Pass;
	class RenderQueuePass;
	class Source;
	class Sink;

	class RenderGraph
	{
	public:
		enum class Type
		{
			Default,
			PreCal,
		};
	public:
		RenderGraph(Graphics& gfx, Type type = Type::Default);
		~RenderGraph();
		void Execute( Graphics& gfx ) noxnd;
		void Reset() noexcept;
		RenderQueuePass& GetRenderQueue( const std::string& passName );
		void StoreDepth( Graphics& gfx,const std::string& path );
	protected:
		void SetSinkTarget( const std::string& sinkName,const std::string& target );
		void AddGlobalSource( std::unique_ptr<Source> );
		void AddGlobalSink( std::unique_ptr<Sink> );
		void Finalize();
		void AppendPass( std::unique_ptr<Pass> pass );
		Pass& FindPassByName( const std::string& name );
		std::shared_ptr<Bind::RenderTarget> backBufferTarget;
		std::shared_ptr<Bind::OutputOnlyDepthStencil> masterDepth;
	private:
		void LinkSinks( Pass& pass );
		void LinkGlobalSinks();
	private:
		std::vector<std::unique_ptr<Pass>> passes;
		std::vector<std::unique_ptr<Source>> globalSources;
		std::vector<std::unique_ptr<Sink>> globalSinks;
		bool finalized = false;
	};
}