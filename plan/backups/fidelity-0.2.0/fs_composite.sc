$input v_uv
#include <bgfx_shader.sh>
SAMPLER2D(s_source,0);
SAMPLER2D(s_bloom,1);
uniform vec4 u_post;
uniform mat4 u_sky;
void main()
{
    vec3 color = texture2D(s_source,v_uv).xyz+texture2D(s_bloom,v_uv).xyz*u_post.x*1.5;
    vec3 direction = normalize(mul(u_sky,vec4(v_uv*2.0-1.0,1.0,0.0)).xyz);
    vec2 cells = direction.xy/max(abs(direction.z),0.1)*vec2(240.0,135.0);
    float hash = fract(sin(dot(floor(cells),vec2(12.9898,78.233)))*43758.5453);
    float spark = (1.0-smoothstep(0.035,0.22,length(fract(cells)-0.5)))*step(0.994,hash);
    if(direction.y>0.02 && max(color.x,max(color.y,color.z))<0.02) color += vec3(spark*0.45,spark*0.45,spark*0.45);
    color *= 1.15;
    color = clamp((color*(2.51*color+0.03))/(color*(2.43*color+0.59)+0.14),0.0,1.0);
    if(u_post.y<0.5) color = pow(color,vec3(1.0/2.2,1.0/2.2,1.0/2.2));
    float vignette = 1.0-0.25*pow(length((v_uv-0.5)*1.2),2.0);
    gl_FragColor = vec4(color*vignette,1.0);
}
