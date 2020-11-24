void PhongShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V)
{
	// diffuse light
	litRes.diffuseLighting = Diffuse(litData.pointIrradiance, litData.pointAtten, litData.pointDirToL, gBuffer.normal);
	litRes.diffuseLighting += Diffuse(litData.directionalIrradiance, 1.0f, litData.directionalDirToL, gBuffer.normal);
	// specular reflected
	litRes.specularLighting = Speculate_New(normalize(V + litData.pointDirToL), litData.pointIrradiance * gBuffer.specularColor,
		gBuffer.specularWeight, gBuffer.normal, litData.pointAtten, gBuffer.specularGloss);
	litRes.specularLighting += Speculate_New(normalize(V + litData.directionalDirToL), litData.directionalIrradiance * gBuffer.specularColor,
		gBuffer.specularWeight, gBuffer.normal, 1.0f, gBuffer.specularGloss);
}

void PBRShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V)
{

}

void LiquidShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V)
{

}

void ToonShading(inout LightingResult litRes, GBuffer gBuffer, LightData litData, float3 V)
{

}
