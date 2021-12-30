
#include "Constants.hlsli"
#include "SkyAtmosphereCommon.hlsli"

AtmosphereParameters GetAtmosphereParameters()
{
	AtmosphereParameters Parameters;
	Parameters.solar_irradiance = solar_irradiance;
	Parameters.sun_angular_radius = sun_angular_radius;
	Parameters.absorption_extinction = absorption_extinction;
	Parameters.mu_s_min = mu_s_min;

	//Parameters.rayleigh_density = rayleigh_density;
	//Parameters.mie_density = mie_density;
	//Parameters.absorption_density = absorption_density;
	Parameters.rayleigh_density.layers[0].width = rayleigh_density[0].x;
	Parameters.rayleigh_density.layers[0].exp_term = rayleigh_density[0].y;
	Parameters.rayleigh_density.layers[0].exp_scale = rayleigh_density[0].z;
	Parameters.rayleigh_density.layers[0].linear_term = rayleigh_density[0].w;
	Parameters.rayleigh_density.layers[0].constant_term = rayleigh_density[1].x;
	Parameters.rayleigh_density.layers[1].width = rayleigh_density[1].y;
	Parameters.rayleigh_density.layers[1].exp_term = rayleigh_density[1].z;
	Parameters.rayleigh_density.layers[1].exp_scale = rayleigh_density[1].w;
	Parameters.rayleigh_density.layers[1].linear_term = rayleigh_density[2].x;
	Parameters.rayleigh_density.layers[1].constant_term = rayleigh_density[2].y;
	Parameters.mie_density.layers[0].width = mie_density[0].x;
	Parameters.mie_density.layers[0].exp_term = mie_density[0].y;
	Parameters.mie_density.layers[0].exp_scale = mie_density[0].z;
	Parameters.mie_density.layers[0].linear_term = mie_density[0].w;
	Parameters.mie_density.layers[0].constant_term = mie_density[1].x;
	Parameters.mie_density.layers[1].width = mie_density[1].y;
	Parameters.mie_density.layers[1].exp_term = mie_density[1].z;
	Parameters.mie_density.layers[1].exp_scale = mie_density[1].w;
	Parameters.mie_density.layers[1].linear_term = mie_density[2].x;
	Parameters.mie_density.layers[1].constant_term = mie_density[2].y;
	Parameters.absorption_density.layers[0].width = absorption_density[0].x;
	Parameters.absorption_density.layers[0].exp_term = absorption_density[0].y;
	Parameters.absorption_density.layers[0].exp_scale = absorption_density[0].z;
	Parameters.absorption_density.layers[0].linear_term = absorption_density[0].w;
	Parameters.absorption_density.layers[0].constant_term = absorption_density[1].x;
	Parameters.absorption_density.layers[1].width = absorption_density[1].y;
	Parameters.absorption_density.layers[1].exp_term = absorption_density[1].z;
	Parameters.absorption_density.layers[1].exp_scale = absorption_density[1].w;
	Parameters.absorption_density.layers[1].linear_term = absorption_density[2].x;
	Parameters.absorption_density.layers[1].constant_term = absorption_density[2].y;

	Parameters.mie_phase_function_g = mie_phase_function_g;
	Parameters.rayleigh_scattering = rayleigh_scattering;
	Parameters.mie_scattering = mie_scattering;
	Parameters.mie_extinction = mie_extinction;
	Parameters.ground_albedo = ground_albedo;
	Parameters.bottom_radius = bottom_radius;
	Parameters.top_radius = top_radius;
	return Parameters;
}

float4 main(float2 uv : Texcoord) : SV_Target
{
	//int index = round((uv.y * screenInfo.y - 0.5f) * screenInfo.x + (uv.x * screenInfo.x - 0.5f));
	float3 Color = sceneColor.SampleLevel(splr, uv, 0).rgb;
	
	//float3 color = sceneColor[index].rgb;
	float3 color = Color;
	return float4(color, 0);
}
