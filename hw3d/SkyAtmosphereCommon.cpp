#include "SkyAtmosphereCommon.h"
#include "ChiliMath.h"
#include "imgui/imgui.h"
#include "DynamicConstant.h"

auto EqualFloat3 = [](const dx::XMFLOAT3& a, const dx::XMFLOAT3& b) {return a.x == b.x && a.y == b.y && a.z == b.z; };
auto length3 = [](dx::XMFLOAT3& v) {return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z); };
auto normalize3 = [](dx::XMFLOAT3& v, float l) {dx::XMFLOAT3 r; r.x = v.x / l, r.y = v.y / l, r.z = v.z / l; return r; };
auto scale3 = [](dx::XMFLOAT3& v, float l) {dx::XMFLOAT3 r; r.x = v.x * l, r.y = v.y * l, r.z = v.z * l; return r; };
auto sub3 = [](dx::XMFLOAT3& a, dx::XMFLOAT3& b) {dx::XMFLOAT3 r; r.x = a.x - b.x; r.y = a.y - b.y; r.z = a.z - b.z; return r; };
auto add3 = [](dx::XMFLOAT3& a, dx::XMFLOAT3& b) {dx::XMFLOAT3 r; r.x = a.x + b.x; r.y = a.y + b.y; r.z = a.z + b.z; return r; };
auto MaxZero3 = [](dx::XMFLOAT3& a) {dx::XMFLOAT3 r; r.x = a.x > 0.0f ? a.x : 0.0f; r.y = a.y > 0.0f ? a.y : 0.0f; r.z = a.z > 0.0f ? a.z : 0.0f; return r; };

SkyAtmosphereCommon::SkyAtmosphereCommon(Graphics& gfx)
{
	SetupEarthAtmosphere(atmosphereInfos);
	dx::XMFLOAT3 RayleighScattering = atmosphereInfos.rayleigh_scattering;
	dx::XMFLOAT3 MieScattering = atmosphereInfos.mie_scattering;
	dx::XMFLOAT3 MieAbsorption = sub3(atmosphereInfos.mie_extinction, atmosphereInfos.mie_scattering);
	MieAbsorption = MaxZero3(MieAbsorption);
	MieScatteringLength = length3(MieScattering);
	MieScatteringColor = MieScatteringLength == 0.0f ? dx::XMFLOAT3{ 0.0f,0.0f,0.0f } : normalize3(MieScattering, MieScatteringLength);
	MieAbsLength = length3(MieAbsorption);
	MieAbsColor = MieAbsLength == 0.0f ? dx::XMFLOAT3{ 0.0f,0.0f,0.0f } : normalize3(MieAbsorption, MieAbsLength);
	RayleighScatteringLength = length3(RayleighScattering);
	RayleighScatteringColor = RayleighScatteringLength == 0.0f ? dx::XMFLOAT3{ 0.0f,0.0f,0.0f } : normalize3(RayleighScattering, RayleighScatteringLength);
	AtmosphereHeight = atmosphereInfos.top_radius - atmosphereInfos.bottom_radius;
	MieScaleHeight = -1.0f / atmosphereInfos.mie_density.layers[1].exp_scale;
	RayleighScaleHeight = -1.0f / atmosphereInfos.rayleigh_density.layers[1].exp_scale;

	AbsorptionLength = length3(atmosphereInfos.absorption_extinction);
	AbsorptionColor = AbsorptionLength == 0.0f ? dx::XMFLOAT3{ 0.0f,0.0f,0.0f } : normalize3(atmosphereInfos.absorption_extinction, AbsorptionLength);
	
	Dcb::RawLayout l;
	l.Add<Dcb::Float3>("solar_irradiance");
	l.Add<Dcb::Float>("sun_angular_radius");
	l.Add<Dcb::Float3>("absorption_extinction");
	l.Add<Dcb::Float>("mu_s_min");
	l.Add<Dcb::Float3>("rayleigh_scattering");
	l.Add<Dcb::Float>("mie_phase_function_g");
	l.Add<Dcb::Float3>("mie_scattering");
	l.Add<Dcb::Float>("bottom_radius");
	l.Add<Dcb::Float3>("mie_extinction");
	l.Add<Dcb::Float>("top_radius");
	l.Add<Dcb::Float3>("mie_absorption");
	l.Add<Dcb::Float3>("ground_albedo");
	l.Add<Dcb::Integer>("TRANSMITTANCE_TEXTURE_WIDTH");
	l.Add<Dcb::Integer>("TRANSMITTANCE_TEXTURE_HEIGHT");
	l.Add<Dcb::Integer>("IRRADIANCE_TEXTURE_WIDTH");
	l.Add<Dcb::Integer>("IRRADIANCE_TEXTURE_HEIGHT");
	l.Add<Dcb::Integer>("SCATTERING_TEXTURE_R_SIZE");
	l.Add<Dcb::Integer>("SCATTERING_TEXTURE_MU_SIZE");
	l.Add<Dcb::Integer>("SCATTERING_TEXTURE_MU_S_SIZE");
	l.Add<Dcb::Integer>("SCATTERING_TEXTURE_NU_SIZE");
	l.Add<Dcb::Float3>("SKY_SPECTRAL_RADIANCE_TO_LUMINANCE");
	l.Add<Dcb::Float3>("SUN_SPECTRAL_RADIANCE_TO_LUMINANCE");
	//l.Add<Dcb::Matrix>("gSkyViewProjMat");
	//l.Add<Dcb::Matrix>("gSkyInvViewProjMat");
	//l.Add<Dcb::Matrix>("gShadowmapViewProjMat");
	//l.Add<Dcb::Float3>("camerac");
	//l.Add<Dcb::Float3>("sun_direction");
	//l.Add<Dcb::Float3>("view_ray");
	l.Add<Dcb::Float>("MultipleScatteringFactor");
	l.Add<Dcb::Float>("MultiScatteringLUTRes");
	l.Add<Dcb::Float2>("RayMarchMinMaxSPP");
	l.Add<Dcb::Integer>("gScatteringMaxPathDepth");

	l.Add<Dcb::Array>("rayleigh_density");
	l["rayleigh_density"].Set<Dcb::Float>(10);
	l.Add<Dcb::Array>("mie_density");
	l["mie_density"].Set<Dcb::Float>(10);
	l.Add<Dcb::Array>("absorption_density");
	l["absorption_density"].Set<Dcb::Float>(10);
	Dcb::Buffer buf{ std::move(l) };
	AtmosphereSkyParamsPS = std::make_shared<Bind::CachingPixelConstantBufferEx>(gfx, buf, 10u);
	AtmosphereSkyParamsCS = std::make_shared<Bind::CachingComputeConstantBufferEx>(gfx, buf, 10u);
}

void SkyAtmosphereCommon::SetupEarthAtmosphere(AtmosphereParameters& info)
{
	// Values shown here are the result of integration over wavelength power spectrum integrated with paricular function.
	// Refer to https://github.com/ebruneton/precomputed_atmospheric_scattering for details.

	// All units in kilometers
	const float EarthBottomRadius = 6360.0f;
	const float EarthTopRadius = 6460.0f;   // 100km atmosphere radius, less edge visible and it contain 99.99% of the atmosphere medium https://en.wikipedia.org/wiki/K%C3%A1rm%C3%A1n_line
	const float EarthRayleighScaleHeight = 8.0f;
	const float EarthMieScaleHeight = 1.2f;

	// Sun - This should not be part of the sky model...
	//info.solar_irradiance = { 1.474000f, 1.850400f, 1.911980f };
	info.solar_irradiance = { 1.0f, 1.0f, 1.0f };	// Using a normalise sun illuminance. This is to make sure the LUTs acts as a transfert factor to apply the runtime computed sun irradiance over.
	info.sun_angular_radius = 0.004675f;

	// Earth
	info.bottom_radius = EarthBottomRadius;
	info.top_radius = EarthTopRadius;
	info.ground_albedo = { 0.0f, 0.0f, 0.0f };

	// Raleigh scattering
	info.rayleigh_density.layers[0] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	info.rayleigh_density.layers[1] = { 0.0f, 1.0f, -1.0f / EarthRayleighScaleHeight, 0.0f, 0.0f };
	info.rayleigh_scattering = { 0.005802f, 0.013558f, 0.033100f };		// 1/km

	// Mie scattering
	info.mie_density.layers[0] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
	info.mie_density.layers[1] = { 0.0f, 1.0f, -1.0f / EarthMieScaleHeight, 0.0f, 0.0f };
	info.mie_scattering = { 0.003996f, 0.003996f, 0.003996f };			// 1/km
	info.mie_extinction = { 0.004440f, 0.004440f, 0.004440f };			// 1/km
	info.mie_phase_function_g = 0.8f;

	// Ozone absorption
	info.absorption_density.layers[0] = { 25.0f, 0.0f, 0.0f, 1.0f / 15.0f, -2.0f / 3.0f };
	info.absorption_density.layers[1] = { 0.0f, 0.0f, 0.0f, -1.0f / 15.0f, 8.0f / 3.0f };
	info.absorption_extinction = { 0.000650f, 0.001881f, 0.000085f };	// 1/km

	const double max_sun_zenith_angle = PI * 120.0 / 180.0; // (use_half_precision_ ? 102.0 : 120.0) / 180.0 * kPi;
	info.mu_s_min = (float)cos(max_sun_zenith_angle);
}

void SkyAtmosphereCommon::ConvertUItoPhysical()
{
	// Convert UI to physical data
	atmosphereInfos.mie_scattering = scale3(MieScatteringColor, MieScatteringLength);
	dx::XMFLOAT3 scaledF3 = scale3(MieAbsColor, MieAbsLength);
	atmosphereInfos.mie_extinction = add3(atmosphereInfos.mie_scattering, scaledF3);
	atmosphereInfos.rayleigh_scattering = scale3(RayleighScatteringColor, RayleighScatteringLength);
	atmosphereInfos.absorption_extinction = scale3(AbsorptionColor, AbsorptionLength);
	atmosphereInfos.top_radius = atmosphereInfos.bottom_radius + AtmosphereHeight;
	atmosphereInfos.mie_density.layers[1].exp_scale = -1.0f / MieScaleHeight;
	atmosphereInfos.rayleigh_density.layers[1].exp_scale = -1.0f / RayleighScaleHeight;
	atmosphereInfos.ground_albedo = uiGroundAbledo;
}

bool SkyAtmosphereCommon::RenderUI()
{
	bool dirty = false;
	if (ImGui::Begin("AtmosphereSky"))
	{
		const auto dcheck = [&dirty](bool changed) {dirty = dirty || changed; };

		dcheck(ImGui::SliderFloat("Mie phase", &atmosphereInfos.mie_phase_function_g, 0.0f, 0.999f, "%.3f", 1.0f / 3.0f));
		dcheck(ImGui::SliderInt("Scatt Order", &NumScatteringOrder, 1, 50));

		dcheck(ImGui::ColorEdit3("MieScattCoeff", &MieScatteringColor.x));
		dcheck(ImGui::SliderFloat("MieScattScale", &MieScatteringLength, 0.00001f, 0.1f, "%.5f", 3.0f));
		dcheck(ImGui::ColorEdit3("MieAbsorCoeff", &MieAbsColor.x));
		dcheck(ImGui::SliderFloat("MieAbsorScale", &MieAbsLength, 0.00001f, 10.0f, "%.5f", 3.0f));
		dcheck(ImGui::ColorEdit3("RayScattCoeff", &RayleighScatteringColor.x));
		dcheck(ImGui::SliderFloat("RayScattScale", &RayleighScatteringLength, 0.00001f, 10.0f, "%.5f", 3.0f));
		dcheck(ImGui::ColorEdit3("AbsorptiCoeff", &AbsorptionColor.x));
		dcheck(ImGui::SliderFloat("AbsorptiScale", &AbsorptionLength, 0.00001f, 10.0f, "%.5f", 3.0f));

		dcheck(ImGui::SliderFloat("Planet radius", &atmosphereInfos.bottom_radius, 100.0f, 8000.0f));
		dcheck(ImGui::SliderFloat("Atmos height", &AtmosphereHeight, 10.0f, 150.0f));
		dcheck(ImGui::SliderFloat("MieScaleHeight", &MieScaleHeight, 0.5f, 20.0f));
		dcheck(ImGui::SliderFloat("RayScaleHeight", &RayleighScaleHeight, 0.5f, 20.0f));

		dcheck(ImGui::ColorEdit3("Ground albedo", &uiGroundAbledo.x));

		dcheck(ImGui::SliderInt("Min SPP", &uiViewRayMarchMinSPP, 1, 30));
		dcheck(ImGui::SliderInt("Max SPP", &uiViewRayMarchMaxSPP, 2, 31));
	}
	ImGui::End();

	return dirty;
}

void SkyAtmosphereCommon::UpdateConstants()
{
	auto buf = AtmosphereSkyParamsPS->GetBuffer();
	buf["solar_irradiance"] = atmosphereInfos.solar_irradiance;
	buf["sun_angular_radius"] = atmosphereInfos.sun_angular_radius;
	buf["absorption_extinction"] = atmosphereInfos.absorption_extinction;
	buf["mu_s_min"] = atmosphereInfos.mu_s_min;
	
	for (unsigned char i = 0; i < 2; i++)
	{
		buf["rayleigh_density"][i * 5 + 0] = atmosphereInfos.rayleigh_density.layers[i].width;
		buf["rayleigh_density"][i * 5 + 1] = atmosphereInfos.rayleigh_density.layers[i].exp_term;
		buf["rayleigh_density"][i * 5 + 2] = atmosphereInfos.rayleigh_density.layers[i].exp_scale;
		buf["rayleigh_density"][i * 5 + 3] = atmosphereInfos.rayleigh_density.layers[i].linear_term;
		buf["rayleigh_density"][i * 5 + 4] = atmosphereInfos.rayleigh_density.layers[i].constant_term;
	}
	for (unsigned char i = 0; i < 2; i++)
	{
		buf["mie_density"][i * 5 + 0] = atmosphereInfos.mie_density.layers[i].width;
		buf["mie_density"][i * 5 + 1] = atmosphereInfos.mie_density.layers[i].exp_term;
		buf["mie_density"][i * 5 + 2] = atmosphereInfos.mie_density.layers[i].exp_scale;
		buf["mie_density"][i * 5 + 3] = atmosphereInfos.mie_density.layers[i].linear_term;
		buf["mie_density"][i * 5 + 4] = atmosphereInfos.mie_density.layers[i].constant_term;
	}
	for (unsigned char i = 0; i < 2; i++)
	{
		buf["absorption_density"][i * 5 + 0] = atmosphereInfos.absorption_density.layers[i].width;
		buf["absorption_density"][i * 5 + 1] = atmosphereInfos.absorption_density.layers[i].exp_term;
		buf["absorption_density"][i * 5 + 2] = atmosphereInfos.absorption_density.layers[i].exp_scale;
		buf["absorption_density"][i * 5 + 3] = atmosphereInfos.absorption_density.layers[i].linear_term;
		buf["absorption_density"][i * 5 + 4] = atmosphereInfos.absorption_density.layers[i].constant_term;
	}

	buf["mie_phase_function_g"] = atmosphereInfos.mie_phase_function_g;
	const float RayleighScatScale = 1.0f;
	buf["rayleigh_scattering"] = scale3(atmosphereInfos.rayleigh_scattering, RayleighScatScale);
	buf["mie_scattering"] = atmosphereInfos.mie_scattering;
	dx::XMFLOAT3 subF3 = sub3(atmosphereInfos.mie_extinction, atmosphereInfos.mie_scattering);
	buf["mie_absorption"] = MaxZero3(subF3);
	buf["mie_extinction"] = atmosphereInfos.mie_extinction;
	buf["ground_albedo"] = atmosphereInfos.ground_albedo;
	buf["bottom_radius"] = atmosphereInfos.bottom_radius;
	buf["top_radius"] = atmosphereInfos.top_radius;
	buf["MultipleScatteringFactor"] = currentMultipleScatteringFactor;
	buf["MultiScatteringLUTRes"] = MultiScatteringLUTRes;

	buf["TRANSMITTANCE_TEXTURE_WIDTH"] = int(lutsInfo.TRANSMITTANCE_TEXTURE_WIDTH);
	buf["TRANSMITTANCE_TEXTURE_HEIGHT"] = int(lutsInfo.TRANSMITTANCE_TEXTURE_HEIGHT);
	buf["IRRADIANCE_TEXTURE_WIDTH"] = int(lutsInfo.IRRADIANCE_TEXTURE_WIDTH);
	buf["IRRADIANCE_TEXTURE_HEIGHT"] = int(lutsInfo.IRRADIANCE_TEXTURE_HEIGHT);
	buf["SCATTERING_TEXTURE_R_SIZE"] = int(lutsInfo.SCATTERING_TEXTURE_R_SIZE);
	buf["SCATTERING_TEXTURE_MU_SIZE"] = int(lutsInfo.SCATTERING_TEXTURE_MU_SIZE);
	buf["SCATTERING_TEXTURE_MU_S_SIZE"] = int(lutsInfo.SCATTERING_TEXTURE_MU_S_SIZE);
	buf["SCATTERING_TEXTURE_NU_SIZE"] = int(lutsInfo.SCATTERING_TEXTURE_NU_SIZE);
	buf["SKY_SPECTRAL_RADIANCE_TO_LUMINANCE"] = dx::XMFLOAT3{ 114974.916437f,71305.954816f,65310.548555f };
	buf["SUN_SPECTRAL_RADIANCE_TO_LUMINANCE"] = dx::XMFLOAT3{ 98242.786222f,69954.398112f,66475.012354f };

	uiViewRayMarchMaxSPP = uiViewRayMarchMinSPP >= uiViewRayMarchMaxSPP ? uiViewRayMarchMinSPP + 1 : uiViewRayMarchMaxSPP;
	buf["RayMarchMinMaxSPP"] = dx::XMFLOAT2{ (float)uiViewRayMarchMinSPP,(float)uiViewRayMarchMaxSPP };

	buf["gScatteringMaxPathDepth"] = NumScatteringOrder;
	AtmosphereSkyParamsPS->SetBuffer(buf);
}
