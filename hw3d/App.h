#pragma once
#include "Window.h"
#include "ChiliTimer.h"
#include "ImguiManager.h"
#include "CameraContainer.h"
#include "PointLight.h"
#include "TestCube.h"
#include "Model.h"
#include "ScriptCommander.h"
#include "BlurOutlineRenderGraph.h"
#include "ChiliMath.h"
#include "SkyBox.h"
#include "DirectionalLight.h"
#include "TestSphere.h"
#include "ConstantBuffers.h"
#include "PlaneWater.h"

class App
{
public:
	App( const std::string& commandLine = "" );
	// master frame / message loop
	int Go();
	~App();
private:
	struct CommonVar
	{
		float time;
		DirectX::XMMATRIX EVRotation;
		float padding[3];
	};
	float time;
	static std::unique_ptr<Bind::VertexConstantBuffer<CommonVar>> cVBuf;
	static std::unique_ptr<Bind::PixelConstantBuffer<CommonVar>> cPBuf;
	static std::unique_ptr<Bind::DomainConstantBuffer<CommonVar>> cDBuf;
private:
	void DoFrame( float dt );
	void HandleInput( float dt );
	void ShowImguiDemoWindow();
	void UpdateCommonVar(Graphics& gfx, const CommonVar& cvar) noxnd;
private:
	std::string commandLine;
	bool showDemoWindow = false;
	ImguiManager imgui;
	Window wnd;
	ScriptCommander scriptCommander;
	Rgph::BlurOutlineRenderGraph rg{ wnd.Gfx() };
	ChiliTimer timer;
	float speed_factor = 1.0f;
	CameraContainer cameras;
	PointLight pointLight;
	TestCube cube{ wnd.Gfx(),4.0f };
	TestCube cube2{ wnd.Gfx(),4.0f };
	Model sponza{ wnd.Gfx(),"Models\\sponza\\sponza.obj",1.0f / 20.0f };
	Model gobber{ wnd.Gfx(),"Models\\gobber\\GoblinX.obj",4.0f };
	Model nano{ wnd.Gfx(),"Models\\nano_textured\\nanosuit.obj",2.0f };
	SkyBox skybox{ wnd.Gfx(),4.0f };
	DirectionalLight dLight;
	bool savingDepth = false;
	std::shared_ptr<Camera> pCam;
	TestSphere sphere{ wnd.Gfx(),4.0f };
	// PlaneWater water{ wnd.Gfx() ,5.0f };
};