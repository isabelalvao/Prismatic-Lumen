$input a_position, a_normal, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_world, v_local, v_normal, v_meta, v_clip
#include <bgfx_shader.sh>
uniform vec4 u_scene;
uniform vec4 u_clock;
void main()
{
    vec3 local = a_position;
    float ground = step(1.5, i_data4.z);
    float pyramid = 1.0 - step(0.55, i_data4.z);
    if (ground < 0.5 && pyramid > 0.5 && local.y > 0.0) local.xz = vec2(0.0,0.0);
    float density = clamp((u_scene.w-i_data4.w)*64.0, 0.0, 1.0);
    if (ground < 0.5) local.y = -1.0 + (local.y+1.0)*density;
    mat4 model = mtxFromCols(i_data0,i_data1,i_data2,i_data3);
    vec4 world = mul(model,vec4(local,1.0));
    if (ground < 0.5) world.z = mod(i_data3.z-u_clock.y+u_clock.z,u_clock.z)+local.z*i_data2.z;
    if (u_clock.w > 0.5) world.y = -world.y;
    v_world = world.xyz;
    v_local = local;
    v_normal = a_normal;
    v_meta = i_data4;
    gl_Position = mul(u_viewProj,world);
    if (ground < 0.5 && density <= 0.0) gl_Position = vec4(2.0,2.0,2.0,1.0);
    v_clip = gl_Position;
}
