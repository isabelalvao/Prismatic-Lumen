$input a_position, a_normal, a_texcoord0
$output v_normal, v_pattern
#include <bgfx_shader.sh>
void main()
{
    gl_Position=mul(u_viewProj,vec4(a_position,1.0));
    v_normal=a_normal;
    v_pattern=a_texcoord0;
}
