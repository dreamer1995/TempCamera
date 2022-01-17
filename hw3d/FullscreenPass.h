#pragma once
#include "BindingPassWithRTDS.h"

namespace Bind
{
	class IndexBuffer;
	class VertexBuffer;
	class VertexShader;
	class InputLayout;
}

namespace Rgph
{
	class FullscreenPass : public BindingPassWithRTDS
	{
	public:
		FullscreenPass( const std::string name,Graphics& gfx, D3D_PRIMITIVE_TOPOLOGY topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) noxnd;
		void Execute( Graphics& gfx ) const noxnd override;
	};
}
