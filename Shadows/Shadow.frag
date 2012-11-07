#extension GL_EXT_shadow_samplers : require
uniform sampler2DShadow shadowMapTex;

varying highp vec4 fColor;
varying highp vec3 fNormal;
varying highp vec2 fTexCoord;
varying highp vec4 fShadowMapCoord;

highp vec3 Light = vec3(0.0, 4.0, 7.0);
highp vec4 Color = vec4(0.2, 0.4, 0.5, 1.0);
lowp vec2 texmapscale = vec2(1.0 / 1024.0, 1.0 / 1024.0);

lowp float offset_lookup(highp vec4 loc, lowp vec2 offset)
{
	//highp vec2 tmp = loc.xy + offset * texmapscale * loc.w;
	return shadow2DProjEXT(shadowMapTex, lowp vec4(loc.xy + offset * texmapscale * loc.w, loc.z, loc.w));
}

void main(void)
{
	Light = normalize(Light);
	lowp float visibility = shadow2DProjEXT(shadowMapTex, fShadowMapCoord) * 0.6 + 0.4;
	/*
	lowp float sum = 0.0;
	lowp float x, y;
	
	for (y = -1.5; y <= 1.5; y += 1.0)
		for (x = -1.5; x <= 1.5; x += 1.0)
			sum += offset_lookup(fShadowMapCoord, vec2(x, y));
	
	lowp float visibility = sum * (1.0 / 16.0);
	*/

	gl_FragColor = fColor * max(0.0, dot(fNormal, Light) * 0.5 + 0.5) * visibility;
}
