#include "TransformCbufScaling.h"
#include "TechniqueProbe.h"

namespace dx = DirectX;

Bind::TransformCbufScaling::TransformCbufScaling(Graphics& gfx, float scale, UINT otherShaderIndex)
	:
	TransformCbuf(gfx, otherShaderIndex),
	buf( MakeLayout() )
{
	buf["scale"] = scale;
}

void Bind::TransformCbufScaling::Accept( TechniqueProbe& probe )
{
	probe.VisitBuffer( buf );
}

void Bind::TransformCbufScaling::Bind( Graphics& gfx ) noxnd
{
	const float scale = buf["scale"];
	const auto scaleMatrix = dx::XMMatrixScaling( scale,scale,scale );
	auto xf = GetTransforms( gfx );
	xf.matrix_W2M = scaleMatrix * xf.matrix_W2M;
	xf.matrix_MV = xf.matrix_MV * scaleMatrix;
	xf.matrix_M2W = xf.matrix_M2W * scaleMatrix;
	xf.matrix_MVP = xf.matrix_MVP * scaleMatrix;
	xf.matrix_T_MV = scaleMatrix * xf.matrix_T_MV;
	xf.matrix_IT_MV = DirectX::XMMatrixTranspose(DirectX::XMMatrixInverse(nullptr, xf.matrix_T_MV));
	UpdateBindImpl( gfx,xf );
}

std::unique_ptr<Bind::CloningBindable> Bind::TransformCbufScaling::Clone() const noexcept
{
	return std::make_unique<TransformCbufScaling>( *this );
}

Dcb::RawLayout Bind::TransformCbufScaling::MakeLayout()
{
	Dcb::RawLayout layout;
	layout.Add<Dcb::Float>( "scale" );
	return layout;
}