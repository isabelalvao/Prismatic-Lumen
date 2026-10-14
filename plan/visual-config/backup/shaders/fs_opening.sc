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
uniform vec4 u_art;
void main()
{
    float kind=v_meta.z;
    float role=u_art.w;
    float pulse=u_art.x*u_scene.y/0.4;
    vec2 uv=v_pattern;
    float emission=0.0;
    if(role>2.5 && role<3.5)
    {
        // Formas arredondadas só aparecem durante as batidas graves do MASTER.
        if(pulse<0.003) discard;
        emission=pulse*0.75;
    }
    else if(role>4.5 && role<5.5)
    {
        float height=abs(v_world.y);
        float edges=smoothstep(0.0,200.0,height)*(1.0-smoothstep(5800.0,6000.0,height));
        emission=edges*0.45;
    }
    else if(role>1.5 && role<2.5)
    {
        // Colunas alternadas sobem/descem a10 cm/s; a geometria mantém-se no lugar.
        float direction=v_meta.w<0.5?1.0:-1.0;
        float height=v_world.y*(1.0-2.0*u_clock.w);
        float coordinate=height/240.0+direction*u_clock.x/24.0;
        float edge=fract(coordinate);
        float width=max(fwidth(coordinate),0.004);
        float bars=smoothstep(0.63,0.63+width,edge)*(1.0-smoothstep(0.96-width,0.96,edge));
        emission=bars*(0.20+pulse*0.12);
    }
    else if(kind>0.5 && kind<2.5)
    {
        // Máscaras Detail recuperadas; Mapping dinâmico reconstruído.
        float detail=texture2D(s_detail,uv).r;
        float top=step(0.45,v_normal.y*(1.0-2.0*u_clock.w));
        emission=detail*top*(0.12+pulse*0.65);
    }
    else if(kind<9.5 && kind>8.5)
        emission=step(1540.0,v_local.y)*(0.22+pulse*0.55);
    else if(kind>10.5 && kind<11.5)
        emission=step(134.0,v_local.y)*(0.22+pulse*0.55);
    else if(role>3.5 && role<4.5)
    {
        // Padrão branco da primeira fase; as texturas Substance não foram recuperadas.
        float contour=uv.y+abs(uv.x-0.5)*2.0;
        float lines=abs(fract(contour*6.0)-0.5);
        float white=1.0-smoothstep(0.022,0.036+fwidth(contour)*6.0,lines);
        // As linhas acendem gradualmente aos80s, sem subir a malha.
        float order=clamp(floor(contour*6.0)/12.0,0.0,0.95);
        float activation=smoothstep(order,min(1.0,order+0.12),u_art.y);
        emission=white*activation*(0.30+pulse*0.08);
    }
    vec3 tint=mix(vec3(1.0,1.0,1.0),mix(u_color0.xyz,u_color1.xyz,
        0.25+0.50*smoothstep(1500.0,12000.0,abs(v_world.z))),0.20);
    float distance=length(v_world-u_eye.xyz);
    float fog=exp(-distance*(0.000025+u_scene.z*0.00004));
    vec3 base=vec3(0.004,0.006,0.010)*(0.3+0.7*max(0.0,v_normal.y));
    if(role>2.5 && role<3.5) base=vec3(0.0,0.0,0.0);
    vec3 rgb=(base+emission*tint*4.0)*u_scene.x/0.6;
    rgb=mix(vec3(0.001,0.002,0.005)+u_atmosphere.xyz*0.012,rgb,fog);
    gl_FragColor=vec4(rgb*u_style.x,1.0);
}
