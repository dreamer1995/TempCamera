float3 MapNormal(
    const in float3 tan,
    const in float3 binor,
    const in float3 normal,
    const in float2 tc,
    uniform Texture2D nmap,
    uniform SamplerState splr)
{
    // build the tranform (rotation) into same space as tan/binor/normal (target space)
    const float3x3 tanToTarget = float3x3(tan, binor, normal);
    // sample and unpack the normal from texture into target space   
    const float3 normalSample = nmap.Sample(splr, tc).xyz;
    const float3 tanNormal = normalSample * 2.0f - 1.0f;
    // bring normal from tanspace into target space
    return normalize(mul(tanNormal, tanToTarget));
}

float Attenuate(uniform float attConst, uniform float attLin, uniform float attQuad, const in float distFragToL)
{
    return 1.0f / (attConst + attLin * distFragToL + attQuad * (distFragToL * distFragToL));
}

float3 Diffuse(
    uniform float3 irradiance,
    const in float att,
    const in float3 lightDir,
    const in float3 normal)
{
    return irradiance * att * max(0.0f, dot(normal, lightDir));
}

float3 Speculate(
    const in float3 viewPos,
    const in float3 fragPos,
    const in float3 lightDir,
    const in float3 specularColor,
    uniform float specularIntensity,
    const in float3 normal,
    const in float att,
    const in float specularPower)
{
    // vector from camera to fragment (in view space)
    const float3 viewDir = normalize(viewPos - fragPos);
    // calculate half light vector
    const float3 halfDir = normalize(lightDir + viewDir);
    // calculate specular component color based on angle between
    // viewing vector and reflection vector, narrow with power function
    return att * specularColor * specularIntensity * pow(max(0.0f, dot(normal, halfDir)), specularPower);
}

float3 Diffuse_New(
    uniform float3 irradiance,
    const in float3 lightDir,
    const in float3 normal)
{
    return irradiance * max(0.0f, dot(normal, lightDir));
}

float3 Speculate_New(
    const float3 halfDir,
    const in float3 irradiance,
    uniform float specularIntensity,
    const in float3 normal,
    const in float specularPower)
{
    // calculate specular component color based on angle between
    // viewing vector and reflection vector, narrow with power function
    return irradiance * specularIntensity * pow(max(0.0f, dot(normal, halfDir)), specularPower);
}

float4 ToShadowHomoSpace(const in float4 worldPos)
{
    const float4 shadowHomo = mul(worldPos, shadowMatrix_VP);
    return shadowHomo * float4(0.5f, -0.5f, 1.0f, 1.0f) + float4(0.5f, 0.5f, 0.0f, 0.0f) * shadowHomo.w;
}

float ShadowLoop_(const in float3 spos, uniform int range)
{    
    float shadowLevel = 0.0f;
    [unroll]
    for (int x = -range; x <= range; x++)
    {
        [unroll]
        for (int y = -range; y <= range; y++)
        {
            if( hwPcf )
            {
                shadowLevel += smap.SampleCmpLevelZero(ssamHw, spos.xy, spos.b - depthBias, int2(x, y));
            }
            else
            {
                shadowLevel += smap.Sample(ssamSw, spos.xy, int2(x, y)).r >= spos.b - depthBias ? 1.0f : 0.0f;
            }
        }
    }
    return shadowLevel / ((range * 2 + 1) * (range * 2 + 1));
}

float Shadow(const in float4 shadowHomoPos)
{    
    float shadowLevel = 0.0f;
    const float3 spos = shadowHomoPos.xyz / shadowHomoPos.w;
    
    if( spos.z > 1.0f || spos.z < 0.0f )
    {
        shadowLevel = 1.0f;
    }
    else
    {
        [unroll]
        for (int level = 0; level <= 4; level++)
        {
            if (level == pcfLevel)
            {
                shadowLevel = ShadowLoop_(spos, level);
            }
        }
    }
    return shadowLevel;
}
