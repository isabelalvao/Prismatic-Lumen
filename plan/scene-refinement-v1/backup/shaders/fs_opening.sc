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
uniform vec4 u_tuning[22];
void main()
{
    float kind=v_meta.z;
    if(kind>10.5 && kind<11.5 && v_world.y*(1.0-2.0*u_clock.w)<0.0) discard;
    float role=u_art.w;
    float pulse=u_art.x*u_scene.y/0.4;
    vec2 uv=v_pattern;
    float emission=0.0;
    if(role<0.5)
    {
        // Material sem padrão emissivo recuperado: acende linhas, sem clarear o corpo inteiro.
        float contour=uv.y+abs(uv.x-0.5)*2.0;
        float lines=abs(fract(contour*u_tuning[5].w)-0.5);
        float mask=1.0-smoothstep(u_tuning[12].w,u_tuning[12].w+max(fwidth(contour)*u_tuning[5].w,0.005),lines);
        emission=mask*u_tuning[4].w*u_tuning[9].w;
    }
    else if(role>2.5 && role<3.5)
    {
        // Formas arredondadas só aparecem durante as batidas graves do MASTER.
        if(pulse<u_tuning[4].z && u_tuning[4].x<=0.0) discard;
        emission=u_tuning[4].x+pulse*u_tuning[4].y;
    }
    else if(role>4.5 && role<5.5)
    {
        float height=abs(v_world.y);
        float edges=smoothstep(0.0,u_tuning[5].y,height)*(1.0-smoothstep(u_tuning[5].x-u_tuning[5].y,u_tuning[5].x,height));
        emission=edges*u_tuning[5].z;
    }
    else if(role>1.5 && role<2.5)
    {
        // Colunas alternadas sobem/descem a10 cm/s; a geometria mantém-se no lugar.
        float direction=v_meta.w<0.5?1.0:-1.0;
        if(abs(u_tuning[6].z)>0.5) direction=-u_tuning[6].z;
        float height=v_world.y*(1.0-2.0*u_clock.w);
        float coordinate=(height+direction*u_tuning[21].y)/u_tuning[6].x;
        float edge=fract(coordinate);
        float width=max(fwidth(coordinate),u_tuning[7].w);
        float bars=smoothstep(u_tuning[7].y,u_tuning[7].y+width,edge)*(1.0-smoothstep(u_tuning[7].z-width,u_tuning[7].z,edge));
        emission=bars*(u_tuning[6].w+pulse*u_tuning[7].x);
    }
    else if(kind>0.5 && kind<2.5)
    {
        // Máscaras Detail recuperadas; Mapping dinâmico reconstruído.
        float detail=texture2D(s_detail,uv).r;
        float top=step(u_tuning[8].z,v_normal.y*(1.0-2.0*u_clock.w));
        emission=detail*top*(u_tuning[8].x+pulse*u_tuning[8].y);
    }
    else if(kind<9.5 && kind>8.5)
        emission=step(u_tuning[9].y,v_local.y)*(u_tuning[8].w+pulse*u_tuning[9].x);
    else if(kind>10.5 && kind<11.5)
        emission=step(u_tuning[9].z,v_local.y)*(u_tuning[8].w+pulse*u_tuning[9].x);
    else if(role>3.5 && role<4.5)
    {
        // Padrão branco da primeira fase; as texturas Substance não foram recuperadas.
        float contour=uv.y+abs(uv.x-0.5)*u_tuning[10].w;
        float lines=abs(fract(contour*u_tuning[10].x)-0.5);
        float white=1.0-smoothstep(u_tuning[10].y,u_tuning[10].z+fwidth(contour)*u_tuning[10].x,lines);
        // As linhas acendem gradualmente aos80s, sem subir a malha.
        float order=clamp(floor(contour*u_tuning[10].x)/u_tuning[11].w,0.0,0.95);
        float activation=smoothstep(order,min(1.0,order+u_tuning[11].z),u_art.y);
        emission=white*activation*(u_tuning[11].x+pulse*u_tuning[11].y);
    }
    vec3 tint=mix(vec3(1.0,1.0,1.0),mix(u_color0.xyz,u_color1.xyz,
        mix(u_tuning[1].z,u_tuning[1].w,smoothstep(u_tuning[1].x,u_tuning[1].y,abs(v_world.z)))),u_tuning[0].y)*u_tuning[20].xyz;
    float distance=length(v_world-u_eye.xyz);
    float fog=exp(-distance*(u_tuning[0].z+u_scene.z*u_tuning[0].w));
    vec3 base=u_tuning[2].xyz*(u_tuning[2].w+(1.0-u_tuning[2].w)*max(0.0,v_normal.y));
    if(role<0.5) base*=u_tuning[20].xyz*u_tuning[20].w;
    if(role>2.5 && role<3.5) base=vec3(0.0,0.0,0.0);
    vec3 rgb=(base+emission*tint*u_tuning[0].x*u_tuning[20].w)*u_scene.x/0.6;
    rgb=mix(u_tuning[3].xyz+u_atmosphere.xyz*u_tuning[3].w,rgb,fog);
    gl_FragColor=vec4(rgb,clamp(u_style.x*v_meta.x,0.0,1.0));
}
