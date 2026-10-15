$input a_position, a_normal
$output v_normal
#include <bgfx_shader.sh>
void main()
{
    // Posições físicas já convertidas para o espaço dos olhos, sem movimento da cena.
    gl_Position=mul(u_viewProj,vec4(a_position,1.0));
    v_normal=a_normal;
}
