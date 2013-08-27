uniform sampler2D tex;
uniform vec2 pixel;

void main()
{
    vec2 pixel_pos   = gl_TexCoord[0].xy;
    vec3 pixel_rgb   = texture2D(tex, pixel_pos).rgb;
    
    ivec3 neighbours = ivec3(0.0);
    neighbours += ivec3(greaterThan(texture2D(tex, pixel_pos + pixel*vec2(-1,-1)).rgb, vec3(0.1)));
    neighbours += ivec3(greaterThan(texture2D(tex, pixel_pos + pixel*vec2(-1, 0)).rgb, vec3(0.1)));
    neighbours += ivec3(greaterThan(texture2D(tex, pixel_pos + pixel*vec2(-1, 1)).rgb, vec3(0.1)));
    neighbours += ivec3(greaterThan(texture2D(tex, pixel_pos + pixel*vec2( 0,-1)).rgb, vec3(0.1)));
    neighbours += ivec3(greaterThan(texture2D(tex, pixel_pos + pixel*vec2( 0, 1)).rgb, vec3(0.1)));
    neighbours += ivec3(greaterThan(texture2D(tex, pixel_pos + pixel*vec2( 1,-1)).rgb, vec3(0.1)));
    neighbours += ivec3(greaterThan(texture2D(tex, pixel_pos + pixel*vec2( 1, 0)).rgb, vec3(0.1)));
    neighbours += ivec3(greaterThan(texture2D(tex, pixel_pos + pixel*vec2( 1, 1)).rgb, vec3(0.1)));

    vec3 live = vec3(greaterThan(pixel_rgb, vec3(0.0)));
    pixel_rgb += (1.0 - live) *vec3(equal(neighbours, ivec3(3)));
    pixel_rgb *= vec3(equal(neighbours, ivec3(2))) + vec3(equal(neighbours,ivec3(3)));
    pixel_rgb -= vec3(greaterThan(pixel_rgb, vec3(0.4)))*0.05;

    gl_FragColor = vec4(pixel_rgb, 1.0);
}

