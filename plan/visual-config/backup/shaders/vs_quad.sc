$input a_position
$output v_uv
#include <bgfx_shader.sh>
void main()
{
    gl_Position = vec4(a_position.xy,0.0,1.0);
    v_uv = vec2(a_position.x*0.5+0.5,0.5-a_position.y*0.5);
}
