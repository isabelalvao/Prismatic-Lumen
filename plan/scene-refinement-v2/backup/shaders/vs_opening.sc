$input a_position, a_normal, a_texcoord0, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_world, v_local, v_normal, v_meta, v_clip, v_pattern
#include <bgfx_shader.sh>
uniform vec4 u_scene;
uniform vec4 u_clock;
uniform vec4 u_style;
uniform vec4 u_layout;
void main()
{
    mat4 model=mtxFromCols(vec4(i_data0.xyz,0.0),vec4(i_data1.xyz,0.0),vec4(i_data2.xyz,0.0),vec4(i_data3.xyz,1.0));
    vec3 local=a_position;
    // Cauda subterrânea fixa: o poste desloca-se inteiro, sem alongar a haste.
    if(i_data4.x>0.0 && i_data4.z>10.5 && i_data4.z<11.5)
        if(local.y<=0.001) local.y=-1000.0/max(length(i_data1.xyz),0.0001);
    vec4 world=mul(model,vec4(local,1.0));
    world.z-=u_clock.y;
    if(u_clock.w>0.5) world.y=-world.y;
    v_world=world.xyz;
    v_local=a_position;
    v_pattern=a_texcoord0;
    v_normal=normalize(i_data0.xyz*a_normal.x/max(dot(i_data0.xyz,i_data0.xyz),0.000001)
        +i_data1.xyz*a_normal.y/max(dot(i_data1.xyz,i_data1.xyz),0.000001)
        +i_data2.xyz*a_normal.z/max(dot(i_data2.xyz,i_data2.xyz),0.000001));
    if(u_clock.w>0.5) v_normal.y=-v_normal.y;
    v_meta=i_data4;
    // Opacidade preparada na CPU: a classificação opaco/transparente usa exatamente o mesmo valor.
    float presence=i_data1.w;
    v_meta.x=presence;
    gl_Position=mul(u_viewProj,world);
    if(u_scene.w<=0.0||i_data4.w>=clamp(u_scene.w*2.0,0.0,1.0)||u_style.x<=0.0||presence<=0.0)
        gl_Position=vec4(2.0,2.0,2.0,1.0);
    v_clip=gl_Position;
}
