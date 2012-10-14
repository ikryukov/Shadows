#	extension GL_EXT_shadow_samplers : require
uniform sampler2DShadow shadowMapTex;

varying highp vec4 fColor;
varying highp vec3 fNormal;
varying highp vec2 fTexCoord;
varying highp vec4 fShadowMapCoord;
												
highp vec3 Light = vec3(0.0, 4.0, 7.0);
highp vec4 Color = vec4(0.2, 0.4, 0.5, 1.0);


void main(void)
{
	Light = normalize(Light);
	lowp float visibility = shadow2DProjEXT(shadowMapTex, fShadowMapCoord) * 0.6 + 0.4;
	gl_FragColor = fColor * max(0.0, dot(fNormal, Light)) * visibility;
	//gl_FragColor = Color * visibility;
}
	