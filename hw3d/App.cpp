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
	dLight(wnd.Gfx(), { 10.0f,9.0f,2.5f }, 63.0f * PI / 180.0f, 84.0f * PI / 180.0f)
{
	pointLight = std::make_unique<PointLight>(wnd.Gfx(), dx::XMFLOAT3{ 16.5f, 9.0f, 1.5f }, 1.0f, 4u);
	//pointLight = std::make_unique<PointLight>(wnd.Gfx(), dx::XMFLOAT3{  27.f + 9 * 0.666666f, 20.0f, 1.7f }, 1.0f, 4u);
	//pointLight2 = std::make_unique<PointLight>(wnd.Gfx(), dx::XMFLOAT3{ 27.f - 9 * 0.333333f, 20.0f, 1.7f + 9 * 0.577350f }, 1.0f, 7u);
	//pointLight3 = std::make_unique<PointLight>(wnd.Gfx(), dx::XMFLOAT3{ 27.f - 9 * 0.333333f, 20.0f, 1.7f - 9 * 0.577350f }, 1.0f, 8u);

	time = 0;
	cVBuf = std::make_unique<Bind::VertexConstantBuffer<CommonVar>>(wnd.Gfx(), 2u);
	cPBuf = std::make_unique<Bind::PixelConstantBuffer<CommonVar>>(wnd.Gfx(), 2u);
	cDBuf = std::make_unique<Bind::DomainConstantBuffer<CommonVar>>(wnd.Gfx(), 2u);
	cameras.AddCamera(std::make_unique<Camera>(wnd.Gfx(), "A", dx::XMFLOAT3{ -7.2f,6.2f,5.9f }, -9.0f * PI / 180.0f, 105.0f * PI / 180.0f));
	cameras.AddCamera( std::make_unique<Camera>( wnd.Gfx(),"B",dx::XMFLOAT3{ -13.5f,28.8f,-6.4f },PI / 180.0f * 13.0f,PI / 180.0f * 61.0f ) );
	//pCam = std::make_unique<Camera>(wnd.Gfx(), "B", dx::XMFLOAT3{ -13.5f,28.8f,-6.4f }, PI / 180.0f * 13.0f, PI / 180.0f * 61.0f);
	//cameras.AddCamera(pCam);
	//cameras.AddCamera( pointLight.ShareCamera() );
	cameras.AddCamera(dLight.ShareCamera());

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

	skybox.LinkTechniques(rg);
	cameras.LinkTechniques(rg);
	pointLight->LinkTechniques( rg );
	//pointLight2->LinkTechniques(rg);
	//pointLight3->LinkTechniques(rg);
	dLight.LinkTechniques(rg);

	sponza.LinkTechniques( rg );
	gobber.LinkTechniques( rg );
	nano.LinkTechniques( rg );
	cube.LinkTechniques(rg);
	cube2.LinkTechniques(rg);
	sphere.LinkTechniques(rg);
	// water.LinkTechniquesEX(rg);

	//wnd.Gfx().SetProjection( dx::XMMatrixPerspectiveLH( 1.0f,9.0f / 16.0f,0.5f,400.0f ) );
	pCams.emplace_back(pointLight);
	//pCams.emplace_back(pointLight2);
	//pCams.emplace_back(pointLight3);
	rg.BindShadowCamera(wnd.Gfx(), *dLight.ShareCamera(), pCams);
}

void App::DoFrame( float dt )
{
	time += dt;
	UpdateCommonVar(wnd.Gfx(), { time,DirectX::XMMatrixRotationRollPitchYaw(skybox.pitch, skybox.yaw, skybox.roll),(unsigned int)pCams.size() });
	//wnd.Gfx().BeginFrame( 0.07f,0.0f,0.12f );
	wnd.Gfx().BeginFrame(0.1f, 0.1f, 0.1f);
	//wnd.Gfx().SetCamera(cameras->GetMatrix() );
	rg.BindMainCamera(cameras.GetActiveCamera());
	cameras->Bind(wnd.Gfx());

	GameLogic(dt, time);

	//skybox.SetPos({ cameras->pos });
	skybox.Submit(Chan::main);

	pointLight->Submit(Chan::main);
	pointLight->Bind(wnd.Gfx());
	//pointLight2->Submit(Chan::main);
	//pointLight2->Bind(wnd.Gfx());
	//pointLight3->Submit(Chan::main);
	//pointLight3->Bind(wnd.Gfx());

	dLight.Submit(Chan::main);
	dLight.Bind(wnd.Gfx());

	cameras.Submit(Chan::main);
	cube.Submit(Chan::main);
	cube2.Submit(Chan::main);
	sponza.Submit(Chan::main);
	gobber.Submit(Chan::main);
	nano.Submit(Chan::main);
	sphere.Submit(Chan::main);
	//water.SubmitEX(Chan::waterPre, Chan::main);

	sponza.Submit(Chan::shadow);
	gobber.Submit(Chan::shadow);
	nano.Submit(Chan::shadow);
	cube.Submit(Chan::shadow);
	cube2.Submit(Chan::shadow);
	sphere.Submit(Chan::shadow);

	rg.Execute( wnd.Gfx() );
	
	//if (savingDepth)
	//{
	//	rg.DumpShadowMap(wnd.Gfx(), "Images\\shadow.png");
	//	savingDepth = false;
	//}

	// imgui windows
	cameras.SpawnWindow( wnd.Gfx() );
	pointLight->SpawnControlWindow("PointLight");
	//pointLight2->SpawnControlWindow("PointLight2");
	//pointLight3->SpawnControlWindow("PointLight3");
	dLight.SpawnControlWindow();
	ShowImguiDemoWindow();
	skybox.SpawnControlWindow(wnd.Gfx(), "SkyBox");

	static MP sponzeProbe{ "Sponza" };
	static MP gobberProbe{ "Gobber" };
	static MP nanoProbe{ "Nano" };
	sponzeProbe.SpawnWindow(sponza);
	gobberProbe.SpawnWindow(gobber);
	nanoProbe.SpawnWindow(nano);
	cube.SpawnControlWindow(wnd.Gfx(), "Cube 1");
	cube2.SpawnControlWindow(wnd.Gfx(), "Cube 2");
	sphere.SpawnControlWindow(wnd.Gfx(), "Sphere");
	//water.SpawnControlWindow(wnd.Gfx(), "Water");
	rg.RenderWindows( wnd.Gfx() );
	//RenderMainWindows(wnd.Gfx());

	//if (ImGui::Begin("Delete"))
	//{
	//	if (ImGui::Button("deletecam"))
	//	{
	//		cameras.DeleteCamera(pCam);
	//	}
	//}
	//ImGui::End();

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

void App::HandleInput( float dt )
{
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
		//case VK_F3:
		//	isWireframe = true;
		//	break;
		case VK_F4:
			wnd.Gfx().isWireFrame = !wnd.Gfx().isWireFrame;
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
		/*case VK_RETURN:
			savingDepth = true;
			break;*/
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
		cameras->LookZero({ pointLight->GetPos().x, pointLight->GetPos().y, pointLight->GetPos().z });
	}

	while (const auto delta = wnd.mouse.ReadRawDelta())
	{
		if (!wnd.CursorEnabled() && wnd.mouse.RightIsPressed())
		{
			cameras->Rotate((float)delta->x, (float)delta->y);
		}
		else if (!wnd.CursorEnabled() && wnd.kbd.KeyIsPressed(VK_MENU) && wnd.mouse.LeftIsPressed())
		{
			cameras->RotateAround((float)delta->x, (float)delta->y, { pointLight->GetPos().x, pointLight->GetPos().y, pointLight->GetPos().z });
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

void App::UpdateCommonVar(Graphics& gfx, const CommonVar& cvar) noxnd
{
	cVBuf->Update(gfx, cvar);
	cVBuf->Bind(gfx);
	cPBuf->Update(gfx, cvar);
	cPBuf->Bind(gfx);
	cDBuf->Update(gfx, cvar);
	cDBuf->Bind(gfx);
}
std::unique_ptr<Bind::VertexConstantBuffer<App::CommonVar>> App::cVBuf;
std::unique_ptr<Bind::PixelConstantBuffer<App::CommonVar>> App::cPBuf;
std::unique_ptr<Bind::DomainConstantBuffer<App::CommonVar>> App::cDBuf;

//void App::RenderMainWindows(Graphics& gfx)
//{
//	if (ImGui::Begin("Íæ"))
//	{
//		ImGui::SliderFloat("X", &rotateSpeed, 0.0f, 5.0f, "%.001f");
//		ImGui::SliderFloat("Y", &flickerSpeed, 0.0f, 5.0f, "%.1f");
//		ImGui::SliderFloat("Z", &scanSpeed, 0.0f, 1.0f, "%.1f");
//		ImGui::SliderFloat("W", &extentSpeed, 0.0f, 2.0f, "%.1f");
//	}
//	ImGui::End();
//}

void App::GameLogic(float dt, float time)
{
	//pointLight->RotateAround(dt, 0.0f, { 27.f, -0.56f, 1.7f }, rotateSpeed);
	//pointLight2->RotateAround(dt, 0.0f, { 27.f, -0.56f, 1.7f }, rotateSpeed);
	//pointLight3->RotateAround(dt, 0.0f, { 27.f, -0.56f, 1.7f }, rotateSpeed);

	//auto r = std::lerp(5.0, 12.0, std::sin(time * extentSpeed) / 2 + 0.5);
	//auto x = pointLight->cbData.pos.x - 27.f;
	//auto y = pointLight->cbData.pos.z - 1.7f;
	//auto length = std::sqrt(x * x + y * y);
	//pointLight->cbData.pos.x = 27.f + x / length * r;
	//pointLight->cbData.pos.z = 1.7f + y / length * r;
	//x = pointLight2->cbData.pos.x - 27.f;
	//y = pointLight2->cbData.pos.z - 1.7f;
	//length = std::sqrt(x * x + y * y);
	//pointLight2->cbData.pos.x = 27.f + x / length * r;
	//pointLight2->cbData.pos.z = 1.7f + y / length * r;
	//x = pointLight3->cbData.pos.x - 27.f;
	//y = pointLight3->cbData.pos.z - 1.7f;
	//length = std::sqrt(x * x + y * y);
	//pointLight3->cbData.pos.x = 27.f + x / length * r;
	//pointLight3->cbData.pos.z = 1.7f + y / length * r;

	//auto b = std::lerp(0.0, 0.2, std::sin(time * flickerSpeed) / 2 + 0.5);
	//pointLight->cbData.diffuseIntensity = b;
	//pointLight2->cbData.diffuseIntensity = b;
	//pointLight3->cbData.diffuseIntensity = b;
	//dLight.pitch = std::lerp(60.0 * PI / 180.0, 120.0 * PI / 180.0, std::sin(time * scanSpeed) / 2 + 0.5);
	//dLight.pCamera->SetRotation(dLight.pitch, dLight.yaw);
}