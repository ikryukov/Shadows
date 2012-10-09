const char* ShadowMapFragmentShader =
STRINGIFY
(
		  
varying lowp vec4 fColor;
varying lowp vec3 fNormal;
varying lowp vec3 fTexCoord;
varying lowp vec4 fShadowMapCoord;
												
lowp vec3 Light = vec3(0.0, 0.0, 1.0);
												
uniform sampler2DShadow shadowMapTex;
												
void main(void)
{
	Light = normalize(Light);
	float visibility = shadow2DProjEXT(shadowMapTex, fShadowMapCoord);
	gl_FragColor = fColor * max(0.0, dot(normalize(fNormal), Light)) * visibility;
}
	
 );
