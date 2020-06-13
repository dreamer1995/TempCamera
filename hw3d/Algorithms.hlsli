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