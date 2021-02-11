float EncodeShadingModelID(uint ShadingModelID)
{
	return (float)ShadingModelID / (float)0xFF;
}
uint DecodeShadingModelID(float channelVal)
{
	return (uint)round(channelVal * (float)0xFF);
}

//#define EncodeNormal(a) (a*127.0/255.0 + 127.0/255.0)
//#define DecodeNormal(a) (a*255.0/127.0 - 1.0)
float3 EncodeNormal( float3 N )
{
	return N * 0.5 + 0.5;
	//return Pack1212To888( UnitVectorToOctahedron( N ) * 0.5 + 0.5 );
}
float3 DecodeNormal( float3 N )
{
	return N * 2 - 1;
	//return OctahedronToUnitVector( Pack888To1212( N ) * 2 - 1 );
}

//float ConvertFromLinearDepth(float linearDepth)
//{
//	return (CameraInfo.x / linearDepth - CameraInfo.x) / (CameraInfo.y - CameraInfo.x);
//}

//float ConvertToLinearDepth(float depth)
//{
//	return CameraInfo.x / (depth * (CameraInfo.y - CameraInfo.x) + CameraInfo.x);
//}