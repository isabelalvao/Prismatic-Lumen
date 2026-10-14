$input v_world, v_local, v_normal, v_meta, v_clip, v_pattern
#include <bgfx_shader.sh>
SAMPLER2D(s_reflection,0);
uniform vec4 u_scene;
uniform vec4 u_clock;
uniform vec4 u_color0;
uniform vec4 u_eye;
uniform vec4 u_style;
uniform vec4 u_atmosphere;
uniform vec4 u_tuning[22];
void main()
{
    vec2 uv = v_clip.xy/v_clip.w*0.5+0.5;
    uv.y = 1.0-uv.y;
    vec2 disturbance = vec2(sin(v_world.z*0.009),cos(v_world.x*0.007))*u_tuning[17].z;
    vec2 roughness = vec2(u_tuning[17].w,u_tuning[18].x);
    vec3 reflection = texture2D(s_reflection,uv+disturbance).xyz*0.5;
    reflection += (texture2D(s_reflection,uv+disturbance+roughness).xyz
        +texture2D(s_reflection,uv+disturbance-roughness).xyz)*0.25;
    vec2 grid = abs(fract(vec2(v_world.x,v_world.z+u_clock.y)/u_tuning[17].x)-0.5);
    float grid_line = 1.0-smoothstep(0.001,0.006,min(grid.x,grid.y));
    float patches = 0.55+0.45*sin(v_world.x*0.002+sin(v_world.z*0.003));
    float fresnel=pow(1.0-max(0.0,normalize(u_eye.xyz-v_world).y),u_tuning[16].w);
    float pools=smoothstep(0.45,0.65,patches);
    vec3 rgb = reflection*(u_tuning[16].x+pools*u_tuning[16].y+fresnel*u_tuning[16].z)*u_tuning[19].w+u_color0.xyz*grid_line*u_scene.x*u_tuning[17].y;
    float distance = length(v_world-u_eye.xyz);
    float fog = exp(-distance*(u_tuning[18].y+u_scene.z*u_tuning[18].z));
    rgb = mix(u_tuning[19].xyz+u_atmosphere.xyz*u_tuning[18].w*u_scene.x,rgb,fog);
    gl_FragColor = vec4(rgb,u_style.x);
}
