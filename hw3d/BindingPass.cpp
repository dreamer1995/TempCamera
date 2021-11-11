#include "BindingPass.h"
#include "Bindable.h"
#include "RenderGraphCompileException.h"


namespace Rgph
{
	BindingPass::BindingPass( std::string name,std::vector<std::shared_ptr<Bind::Bindable>> binds )
		:
		Pass( std::move( name ) ),
		binds( std::move( binds ) )
	{}

	void BindingPass::AddBind( std::shared_ptr<Bind::Bindable> bind ) noexcept
	{
		binds.push_back( std::move( bind ) );
	}

	void BindingPass::BindAll( Graphics& gfx ) const noxnd
	{
		for( auto& bind : binds )
		{
			bind->Bind( gfx );
		}
	}

	void BindingPass::Finalize()
	{
		Pass::Finalize();
	}
}