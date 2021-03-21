#pragma once
#include "Window.h"
#include "ChiliTimer.h"
#include "ImguiManager.h"
#include "CameraContainer.h"
#include "PointLight.h"
#include "TestCube.h"
#include "Model.h"
#include "ScriptCommander.h"
#ifdef USE_DEFERRED
	#include "DeferredRenderGraph.h"
#else
	#include "BlurOutlineRenderGraph.h"
#endif
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
		double time;
		DirectX::XMMATRIX EVRotation;
		unsigned int lightCount;
		alignas(16) DirectX::XMFLOAT4 screenInfo;
		BOOL TAA;
		BOOL HBAO;
		BOOL HDR;
		float GIScale;
		BOOL volumetricRendering;
		float padding[1];
	};
	double time;
	static std::unique_ptr<Bind::VertexConstantBuffer<CommonVar>> cVBuf;
	static std::unique_ptr<Bind::PixelConstantBuffer<CommonVar>> cPBuf;
	static std::unique_ptr<Bind::DomainConstantBuffer<CommonVar>> cDBuf;
private:
	void DoFrame( float dt );
	void HandleInput( float dt );
	void ShowImguiDemoWindow();
	void UpdateCommonVar(Graphics& gfx, const CommonVar& cvar) noxnd;
	void GameLogic(float dt, float time);
	void RenderMainWindows(Graphics& gfx);
private:
	std::string commandLine;
	bool showDemoWindow = false;
	ImguiManager imgui;
	Window wnd;
	ScriptCommander scriptCommander;
#ifdef USE_DEFERRED
	Rgph::DeferredRenderGraph rg{ wnd.Gfx() };
#else
	Rgph::BlurOutlineRenderGraph rg{ wnd.Gfx() };
#endif
	ChiliTimer timer;
	float speed_factor = 1.0f;
	CameraContainer cameras;
	std::shared_ptr<PointLight> pointLight;
	//std::shared_ptr<PointLight> pointLight2;
	//std::shared_ptr<PointLight> pointLight3;
	//TestCube cube{ wnd.Gfx(),4.0f };
	//TestCube cube2{ wnd.Gfx(),4.0f };
	Model sponza{ wnd.Gfx(),"Models\\sponza\\sponza.obj",1.0f / 20.0f, true};
	//Model gobber{ wnd.Gfx(),"Models\\gobber\\GoblinX.obj",4.0f };
	Model nano{ wnd.Gfx(),"Models\\nano_textured\\nanosuit.obj",2.0f };
	SkyBox skybox{ wnd.Gfx(),4.0f };
	DirectionalLight dLight;
	//bool savingDepth = false;
	//std::shared_ptr<Camera> pCam;
	TestSphere sphere{ wnd.Gfx(),4.0f };
	//PlaneWater water{ wnd.Gfx() ,5.0f };
	//float rotateSpeed = 2.5f;
	//float flickerSpeed = 0.4f;
	//float scanSpeed = 0.4f;
	//float extentSpeed = 1.7f;
	std::vector<std::shared_ptr<PointLight>> pCams;
	bool TAA = true;
	bool HBAO = true;
	bool HDR = true;
	float GIScale = 0.353f;
	bool volumetricRendering = true;
};