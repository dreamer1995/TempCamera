#include "App.h"
#include <algorithm>
#include "ChiliMath.h"
#include "imgui/imgui.h"
#include "ChiliUtil.h"
#include "Testing.h"
#include "PerfLog.h"
#include "TestModelProbe.h"
#include "Testing.h"
#include "Camera.h"
#include "Channels.h"

namespace dx = DirectX;

App::App( const std::string& commandLine )
	:
	commandLine( commandLine ),
	wnd( 1280,720,"The Donkey Fart Box" ),
	scriptCommander( TokenizeQuoted( commandLine ) ),
	dLight(wnd.Gfx()),
	pointLight( wnd.Gfx(),{ 10.0f,5.0f,0.0f } )
{
	cameras.AddCamera( std::make_unique<Camera>( wnd.Gfx(),"A",dx::XMFLOAT3{ -13.5f,6.0f,3.5f },0.0f,PI / 2.0f ) );
	cameras.AddCamera( std::make_unique<Camera>( wnd.Gfx(),"B",dx::XMFLOAT3{ -13.5f,28.8f,-6.4f },PI / 180.0f * 13.0f,PI / 180.0f * 61.0f ) );
	cameras.AddCamera( pointLight.ShareCamera() );

	//D3DTestScratchPad( wnd );
	cube.SetPos( { 10.0f,5.0f,6.0f } );
	cube2.SetPos( { 10.0f,5.0f,14.0f } );
	nano.SetRootTransform(
		dx::XMMatrixRotationY( PI / 2.f ) *
		dx::XMMatrixTranslation( 27.f,-0.56f,1.7f )
	);
	gobber.SetRootTransform(
		dx::XMMatrixRotationY( -PI / 2.f ) *
		dx::XMMatrixTranslation( -8.f,10.f,0.f )
	);

	cube.LinkTechniques( rg );
	cube2.LinkTechniques( rg );
	pointLight.LinkTechniques( rg );
	dLight.LinkTechniques(rg);
	sponza.LinkTechniques( rg );
	gobber.LinkTechniques( rg );
	nano.LinkTechniques( rg );
	skybox.LinkTechniques(rg);
	cameras.LinkTechniques(rg);

	//wnd.Gfx().SetProjection( dx::XMMatrixPerspectiveLH( 1.0f,9.0f / 16.0f,0.5f,400.0f ) );
	rg.BindShadowCamera(*pointLight.ShareCamera());
}

void App::HandleInput( float dt )
{
	static bool isWireframe = false;

	static bool isRotate = true;

	while( const auto e = wnd.kbd.ReadKey() )
	{
		if( !e->IsPress() )
		{
			continue;
		}

		switch( e->GetCode() )
		{
		//case VK_ESCAPE:
		//	if( wnd.CursorEnabled() )
		//	{
		//		wnd.DisableCursor();
		//		wnd.mouse.EnableRaw();
		//	}
		//	else
		//	{
		//		wnd.EnableCursor();
		//		wnd.mouse.DisableRaw();
		//	}
		//	break;
		case VK_F1:
			showDemoWindow = true;
			break;
		case VK_F3:
			isWireframe = true;
			break;
		case VK_F4:
			isWireframe = false;
			break;
		//case VK_F5:
		//	if (uvPannel->showUV)
		//	{
		//		uvPannel->showUV = false;
		//	}
		//	else
		//	{
		//		uvPannel->showUV = true;
		//	}
		//	break;
		case VK_SPACE:
			if (isRotate)
			{
				isRotate = false;
			}
			else
			{
				isRotate = true;
			}
			break;
		case VK_RETURN:
			savingDepth = true;
			break;
		}
	}

	static float cameraSpeed = 1.0f;
	while (!wnd.mouse.IsEmpty())
	{
		const auto e = wnd.mouse.Read();

		switch (e->GetType())
		{
		case Mouse::Event::Type::RPress:
		{
			wnd.DisableCursor();
			wnd.mouse.EnableRaw();
			break;
		}
		case Mouse::Event::Type::RRelease:
		{
			wnd.EnableCursor();
			wnd.mouse.DisableRaw();
			break;
		}

		case Mouse::Event::Type::LRelease:
		{
			wnd.EnableCursor();
			wnd.mouse.DisableRaw();
			break;
		}
		case Mouse::Event::Type::LPress:
		{
			if (wnd.kbd.KeyIsPressed(VK_MENU))
			{
				wnd.DisableCursor();
				wnd.mouse.EnableRaw();
				break;
			}

			if (wnd.kbd.KeyIsPressed('L') || wnd.kbd.KeyIsPressed(VK_SHIFT))
			{
				wnd.DisableCursor();
				wnd.mouse.EnableRaw();
				break;
			}
			break;
		}
		case Mouse::Event::Type::WheelUp:
		{
			if (wnd.mouse.RightIsPressed())
			{
				cameraSpeed += 0.3;
			}
			else
			{
				cameras->Translate({ 0.0f,0.0f,10.0f * dt });
			}
			break;
		}
		case Mouse::Event::Type::WheelDown:
		{
			if (wnd.mouse.RightIsPressed())
			{
				cameraSpeed -= 0.3;
			}
			else
			{
				cameras->Translate({ 0.0f,0.0f,10.0f * -dt });
			}
			break;
		}
		case Mouse::Event::Type::WheelPress:
		{
			wnd.DisableCursor();
			wnd.mouse.EnableRaw();
			break;
		}
		case Mouse::Event::Type::WheelRelease:
		{
			wnd.EnableCursor();
			wnd.mouse.DisableRaw();
			break;
		}
		}
		cameraSpeed = std::clamp(cameraSpeed, 0.3f, 9.9f);
	}

	if (!wnd.CursorEnabled())
	{
		if (wnd.kbd.KeyIsPressed('W'))
		{
			cameras->Translate({ 0.0f,0.0f,dt * cameraSpeed });
		}
		if (wnd.kbd.KeyIsPressed('A'))
		{
			cameras->Translate({ -dt * cameraSpeed,0.0f,0.0f });
		}
		if (wnd.kbd.KeyIsPressed('S'))
		{
			cameras->Translate({ 0.0f,0.0f,-dt * cameraSpeed });
		}
		if (wnd.kbd.KeyIsPressed('D'))
		{
			cameras->Translate({ dt * cameraSpeed,0.0f,0.0f });
		}
		if (wnd.kbd.KeyIsPressed('E'))
		{
			cameras->Translate({ 0.0f,dt * cameraSpeed,0.0f });
		}
		if (wnd.kbd.KeyIsPressed('Q'))
		{
			cameras->Translate({ 0.0f,-dt * cameraSpeed,0.0f });
		}
	}

	if (wnd.kbd.KeyIsPressed('F'))
	{
		cameras->LookZero({ pointLight.GetPos().x, pointLight.GetPos().y, pointLight.GetPos().z });
	}

	while (const auto delta = wnd.mouse.ReadRawDelta())
	{
		if (!wnd.CursorEnabled() && wnd.mouse.RightIsPressed())
		{
			cameras->Rotate((float)delta->x, (float)delta->y);
		}
		else if (!wnd.CursorEnabled() && wnd.kbd.KeyIsPressed(VK_MENU) && wnd.mouse.LeftIsPressed())
		{
			cameras->RotateAround((float)delta->x, (float)delta->y, { pointLight.GetPos().x, pointLight.GetPos().y, pointLight.GetPos().z });
		}
		else if (!wnd.CursorEnabled() && wnd.mouse.WheelIsPressed())
		{
			static float mKeyMoveSpeed = 0.1f;
			cameras->Translate({ -(float)delta->x * dt * mKeyMoveSpeed,(float)delta->y * dt * mKeyMoveSpeed,0.0f });
		}
		else if (!wnd.CursorEnabled() && (wnd.kbd.KeyIsPressed('L') || wnd.kbd.KeyIsPressed(VK_SHIFT)) && wnd.mouse.LeftIsPressed())
		{
			dLight.Rotate((float)delta->x, (float)delta->y);
		}
	}
}

void App::DoFrame( float dt )
{
	//wnd.Gfx().BeginFrame( 0.07f,0.0f,0.12f );
	wnd.Gfx().BeginFrame(0.1f, 0.1f, 0.1f);
	//wnd.Gfx().SetCamera(cameras->GetMatrix() );
	rg.BindMainCamera(cameras.GetActiveCamera());
	cameras->Bind(wnd.Gfx());

	skybox.SetPos({ cameras.GetActiveCamera().pos });
	skybox.Submit(Chan::main);

	pointLight.Submit(Chan::main);
	pointLight.Bind(wnd.Gfx());

	dLight.Submit(Chan::main);
	dLight.Bind(wnd.Gfx());

	cameras.Submit(Chan::main);
	cube.Submit(Chan::main);
	cube2.Submit(Chan::main);
	sponza.Submit(Chan::main);
	gobber.Submit(Chan::main);
	nano.Submit(Chan::main);
	
	sponza.Submit(Chan::shadow);
	gobber.Submit(Chan::shadow);
	nano.Submit(Chan::shadow);
	cube.Submit(Chan::shadow);
	cube2.Submit(Chan::shadow);

	rg.Execute( wnd.Gfx() );
	
	if (savingDepth)
	{
		rg.DumpShadowMap(wnd.Gfx(), "shadow.png");
		savingDepth = false;
	}

	// imgui windows
	static MP sponzeProbe{ "Sponza" };
	static MP gobberProbe{ "Gobber" };
	static MP nanoProbe{ "Nano" };
	sponzeProbe.SpawnWindow( sponza );
	gobberProbe.SpawnWindow( gobber );
	nanoProbe.SpawnWindow( nano );
	cameras.SpawnWindow( wnd.Gfx() );
	pointLight.SpawnControlWindow();
	dLight.SpawnControlWindow();
	ShowImguiDemoWindow();
	cube.SpawnControlWindow( wnd.Gfx(),"Cube 1" );
	cube2.SpawnControlWindow( wnd.Gfx(),"Cube 2" );
	skybox.SpawnControlWindow(wnd.Gfx(), "SkyBox");

	rg.RenderWidgets( wnd.Gfx() );

	// present
	wnd.Gfx().EndFrame();
	rg.Reset();
}

void App::ShowImguiDemoWindow()
{
	if( showDemoWindow )
	{
		ImGui::ShowDemoWindow( &showDemoWindow );
	}
}

App::~App()
{}

int App::Go()
{
	while( true )
	{
		// process all messages pending, but to not block for new messages
		if( const auto ecode = Window::ProcessMessages() )
		{
			// if return optional has value, means we're quitting so return exit code
			return *ecode;
		}
		// execute the game logic
		const auto dt = timer.Mark() * speed_factor;
		HandleInput( dt );
		DoFrame( dt );
	}
}