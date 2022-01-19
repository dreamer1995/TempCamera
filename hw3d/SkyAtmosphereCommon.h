#pragma once

#include <DirectXMath.h>
#include "ConstantBuffersEx.h"

namespace dx = DirectX;

class SkyAtmosphereCommon
{
public:
	// An atmosphere layer of width 'width', and whose density is defined as
	//   'exp_term' * exp('exp_scale' * h) + 'linear_term' * h + 'constant_term',
	// clamped to [0,1], and where h is the altitude.
	struct DensityProfileLayer {
		float width;
		float exp_term;
		float exp_scale;
		float linear_term;
		float constant_term;
	};

	// An atmosphere density profile made of several layers on top of each other
	// (from bottom to top). The width of the last layer is ignored, i.e. it always
	// extend to the top atmosphere boundary. The profile values vary between 0
	// (null density) to 1 (maximum density).
	struct DensityProfile {
		DensityProfileLayer layers[2];
	};

	struct AtmosphereParameters {
		// The solar irradiance at the top of the atmosphere.
		dx::XMFLOAT3 solar_irradiance;
		// The sun's angular radius. Warning: the implementation uses approximations
		// that are valid only if this angle is smaller than 0.1 radians.
		float sun_angular_radius;
		// The distance between the planet center and the bottom of the atmosphere.
		float bottom_radius;
		// The distance between the planet center and the top of the atmosphere.
		float top_radius;
		// The density profile of air molecules, i.e. a function from altitude to
		// dimensionless values between 0 (null density) and 1 (maximum density).
		DensityProfile rayleigh_density;
		// The scattering coefficient of air molecules at the altitude where their
		// density is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'rayleigh_scattering' times 'rayleigh_density' at this altitude.
		dx::XMFLOAT3 rayleigh_scattering;
		// The density profile of aerosols, i.e. a function from altitude to
		// dimensionless values between 0 (null density) and 1 (maximum density).
		DensityProfile mie_density;
		// The scattering coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The scattering coefficient at altitude h is equal to
		// 'mie_scattering' times 'mie_density' at this altitude.
		dx::XMFLOAT3 mie_scattering;
		// The extinction coefficient of aerosols at the altitude where their density
		// is maximum (usually the bottom of the atmosphere), as a function of
		// wavelength. The extinction coefficient at altitude h is equal to
		// 'mie_extinction' times 'mie_density' at this altitude.
		dx::XMFLOAT3 mie_extinction;
		// The asymetry parameter for the Cornette-Shanks phase function for the
		// aerosols.
		float mie_phase_function_g;
		// The density profile of air molecules that absorb light (e.g. ozone), i.e.
		// a function from altitude to dimensionless values between 0 (null density)
		// and 1 (maximum density).
		DensityProfile absorption_density;
		// The extinction coefficient of molecules that absorb light (e.g. ozone) at
		// the altitude where their density is maximum, as a function of wavelength.
		// The extinction coefficient at altitude h is equal to
		// 'absorption_extinction' times 'absorption_density' at this altitude.
		dx::XMFLOAT3 absorption_extinction;
		// The average albedo of the ground.
		dx::XMFLOAT3 ground_albedo;
		// The cosine of the maximum Sun zenith angle for which atmospheric scattering
		// must be precomputed (for maximum precision, use the smallest Sun zenith
		// angle yielding negligible sky light radiance values. For instance, for the
		// Earth case, 102 degrees is a good choice - yielding mu_s_min = -0.2).
		float mu_s_min;
	};

	struct LookUpTablesInfo
	{
#if 1
		unsigned int TRANSMITTANCE_TEXTURE_WIDTH = 256;
		unsigned int TRANSMITTANCE_TEXTURE_HEIGHT = 64;

		unsigned int SCATTERING_TEXTURE_R_SIZE = 32;
		unsigned int SCATTERING_TEXTURE_MU_SIZE = 128;
		unsigned int SCATTERING_TEXTURE_MU_S_SIZE = 32;
		unsigned int SCATTERING_TEXTURE_NU_SIZE = 8;

		unsigned int IRRADIANCE_TEXTURE_WIDTH = 64;
		unsigned int IRRADIANCE_TEXTURE_HEIGHT = 16;
#else
		unsigned int TRANSMITTANCE_TEXTURE_WIDTH = 64;
		unsigned int TRANSMITTANCE_TEXTURE_HEIGHT = 16;

		unsigned int SCATTERING_TEXTURE_R_SIZE = 16;
		unsigned int SCATTERING_TEXTURE_MU_SIZE = 16;
		unsigned int SCATTERING_TEXTURE_MU_S_SIZE = 16;
		unsigned int SCATTERING_TEXTURE_NU_SIZE = 4;

		unsigned int IRRADIANCE_TEXTURE_WIDTH = 32;
		unsigned int IRRADIANCE_TEXTURE_HEIGHT = 8;
#endif

		// Derived from above
		unsigned int SCATTERING_TEXTURE_WIDTH = 0xDEADBEEF;
		unsigned int SCATTERING_TEXTURE_HEIGHT = 0xDEADBEEF;
		unsigned int SCATTERING_TEXTURE_DEPTH = 0xDEADBEEF;

		void updateDerivedData()
		{
			SCATTERING_TEXTURE_WIDTH = SCATTERING_TEXTURE_NU_SIZE * SCATTERING_TEXTURE_MU_S_SIZE;
			SCATTERING_TEXTURE_HEIGHT = SCATTERING_TEXTURE_MU_SIZE;
			SCATTERING_TEXTURE_DEPTH = SCATTERING_TEXTURE_R_SIZE;
		}

		LookUpTablesInfo() { updateDerivedData(); }
	};

	SkyAtmosphereCommon(Graphics& gfx);
	void SetupEarthAtmosphere(AtmosphereParameters& info);
	void ConvertUItoPhysical();
	bool RenderUI();
	void UpdateConstants();
	
public:
	AtmosphereParameters atmosphereInfos;
	LookUpTablesInfo lutsInfo;

	float MieScatteringLength;
	dx::XMFLOAT3 MieScatteringColor;
	float MieAbsLength;
	dx::XMFLOAT3 MieAbsColor;
	float MieScaleHeight;

	float RayleighScatteringLength;
	dx::XMFLOAT3 RayleighScatteringColor;
	float RayleighScaleHeight;

	float AbsorptionLength;
	dx::XMFLOAT3 AbsorptionColor;

	float AtmosphereHeight;

	dx::XMFLOAT3 uiGroundAbledo = { 0.0f, 0.0f, 0.0f };
	dx::XMFLOAT3 uiGroundAbledoPrev;

	int NumScatteringOrder = 4;
	int uiRenderingMethodPrev = -1;
	int NumScatteringOrderPrev = 0;

	int transPermutationPrev = 0;
	bool shadowPermutationPrev = 0;
	bool RenderTerrainPrev = 0;
	float multipleScatteringFactorPrev = 0;

	bool forceGenLut = false;
	int uiViewRayMarchMinSPP = 100;
	int uiViewRayMarchMaxSPP = 14;

	float currentMultipleScatteringFactor = 1.0f;
	float MultiScatteringLUTRes = 32;

	std::shared_ptr<Bind::CachingPixelConstantBufferEx> AtmosphereSkyParamsPS;
	std::shared_ptr<Bind::CachingComputeConstantBufferEx> AtmosphereSkyParamsCS;
};

//struct AtmosphereParameters
//{
//	// Radius of the planet (center to ground)
//	float BottomRadius;
//	// Maximum considered atmosphere height (center to atmosphere top)
//	float TopRadius;
//
//	// Rayleigh scattering exponential distribution scale in the atmosphere
//	float RayleighDensityExpScale;
//	// Rayleigh scattering coefficients
//	dx::XMFLOAT3 RayleighScattering;
//
//	// Mie scattering exponential distribution scale in the atmosphere
//	float MieDensityExpScale;
//	// Mie scattering coefficients
//	dx::XMFLOAT3 MieScattering;
//	// Mie extinction coefficients
//	dx::XMFLOAT3 MieExtinction;
//	// Mie absorption coefficients
//	dx::XMFLOAT3 MieAbsorption;
//	// Mie phase function excentricity
//	float MiePhaseG;
//
//	// Another medium type in the atmosphere
//	float AbsorptionDensity0LayerWidth;
//	float AbsorptionDensity0ConstantTerm;
//	float AbsorptionDensity0LinearTerm;
//	float AbsorptionDensity1ConstantTerm;
//	float AbsorptionDensity1LinearTerm;
//	// This other medium only absorb light, e.g. useful to represent ozone in the earth atmosphere
//	dx::XMFLOAT3 AbsorptionExtinction;
//
//	// The albedo of the ground.
//	dx::XMFLOAT3 GroundAlbedo;
//};