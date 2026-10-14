$input a_position, a_normal, a_texcoord0, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_world, v_local, v_normal, v_meta, v_clip, v_pattern
#include <bgfx_shader.sh>
uniform vec4 u_scene;
uniform vec4 u_clock;
uniform vec4 u_style;
void main()
{
    vec3 local=a_position;
    vec3 normal=a_normal;
    float ground=1.0-step(0.1,abs(i_data4.z-2.0));
    float tunnel=step(2.5,i_data4.z)*(1.0-step(7.5,i_data4.z));
    float pyramid=1.0-step(0.55,i_data4.z)+1.0-step(0.1,abs(i_data4.z-10.0));
    // Modelação estática das pirâmides; nenhuma dimensão depende da animação ou opacidade.
    if(ground<0.5 && pyramid>0.5 && local.y>0.0) local.xz=vec2(0.0,0.0);
    if(u_style.z>0.5 && abs(i_data4.z-5.0)<0.1)
    {
        float radius=max(length(local.xy),0.0001);
        local.xy*=(0.82+(radius-0.82)*0.2)/radius;
        local.z*=0.2;
    }
    if(u_style.z>0.5 && i_data4.z>8.5 && i_data4.z<9.5)
    {
        float c=cos(i_data0.w),s=sin(i_data0.w);
        local.xz=vec2(c*local.x+s*local.z,-s*local.x+c*local.z);
        normal.xz=vec2(c*normal.x+s*normal.z,-s*normal.x+c*normal.z);
    }
    // Os componentes w transportam ângulo/opacidade, separados da transformação afim.
    mat4 model=mtxFromCols(vec4(i_data0.xyz,0.0),vec4(i_data1.xyz,0.0),vec4(i_data2.xyz,0.0),vec4(i_data3.xyz,1.0));
    vec4 world=mul(model,vec4(local,1.0));
    if(ground<0.5 && u_style.z<0.5) world.z+=mod(i_data3.z-u_clock.y+u_clock.z+tunnel*1600.0,u_clock.z)-tunnel*1600.0-i_data3.z;
    if(u_clock.w>0.5) world.y=-world.y;
    v_world=world.xyz;
    v_local=a_position;
    v_normal=normalize(i_data0.xyz*normal.x/max(dot(i_data0.xyz,i_data0.xyz),0.000001)
        +i_data1.xyz*normal.y/max(dot(i_data1.xyz,i_data1.xyz),0.000001)
        +i_data2.xyz*normal.z/max(dot(i_data2.xyz,i_data2.xyz),0.000001));
    if(u_clock.w>0.5) v_normal.y=-v_normal.y;
    v_pattern=abs(a_normal.y)>0.5?a_position.xz*0.5+0.5:abs(a_normal.z)>0.5?a_position.xy*0.5+0.5:a_position.zy*0.5+0.5;
    if(i_data4.z>3.5) v_pattern=a_texcoord0;
    if(u_style.z>0.5 && i_data4.z>8.5 && i_data4.z<9.5) v_pattern.x=i_data2.w;
    v_meta=i_data4; v_meta.w=i_data1.w;
    gl_Position=mul(u_viewProj,world);
    if(ground<0.5 && i_data1.w<=0.0) gl_Position=vec4(2.0,2.0,2.0,1.0);
    v_clip=gl_Position;
}
