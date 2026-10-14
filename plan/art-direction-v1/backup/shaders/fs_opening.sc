$input v_world, v_local, v_normal, v_meta, v_clip, v_pattern
#include <bgfx_shader.sh>
SAMPLER2D(s_detail,0);
uniform vec4 u_scene;
uniform vec4 u_clock;
uniform vec4 u_style;
uniform vec4 u_color0;
uniform vec4 u_color1;
uniform vec4 u_eye;
uniform vec4 u_atmosphere;
void main()
{
    float kind=v_meta.z;
    float pulse=pow(max(0.0,sin(u_clock.x*12.5663706)),4.0)*u_scene.y/0.4;
    vec2 uv=v_pattern;
    float emission=0.0;
    if(kind>0.5 && kind<2.5)
    {
        // Máscaras Detail recuperadas; Mapping dinâmico reconstruído.
        float detail=texture2D(s_detail,uv).r;
        float top=step(0.45,v_normal.y*(1.0-2.0*u_clock.w));
        emission=detail*top*(0.12+pulse*0.65);
    }
    else if(kind<7.5 && kind>2.5)
    {
        // Barras contínuas observadas no vídeo; Mapping original ainda por resolver.
        float bars=step(0.67,fract(uv.y*4.0+kind*0.173+u_clock.x*0.025));
        emission=bars*(0.25+pulse*0.35);
    }
    else if(kind<8.5 && kind>7.5) emission=0.25+pulse*0.60;
    else if(kind<9.5 && kind>8.5)
        emission=step(1540.0,v_local.y)*(0.22+pulse*0.55);
    else if(kind>10.5 && kind<11.5)
        emission=step(134.0,v_local.y)*(0.22+pulse*0.55);
    else if(kind>9.5)
    {
        // Padrão branco da primeira fase; as texturas Substance não foram recuperadas.
        float contour=uv.y+abs(uv.x-0.5)*2.0;
        float lines=abs(fract(contour*6.0)-0.5);
        float white=1.0-smoothstep(0.022,0.036+fwidth(contour)*6.0,lines);
        emission=white*(0.40+pulse*0.25);
    }
    vec3 tint=mix(vec3(1.0,1.0,1.0),mix(u_color0.xyz,u_color1.xyz,
        0.25+0.50*smoothstep(1500.0,12000.0,abs(v_world.z))),0.20);
    float distance=length(v_world-u_eye.xyz);
    float fog=exp(-distance*(0.000025+u_scene.z*0.00004));
    vec3 base=vec3(0.004,0.006,0.010)*(0.3+0.7*max(0.0,v_normal.y));
    vec3 rgb=(base+emission*tint*4.0)*u_scene.x/0.6;
    rgb=mix(vec3(0.001,0.002,0.005)+u_atmosphere.xyz*0.012,rgb,fog);
    gl_FragColor=vec4(rgb*u_style.x,1.0);
}
