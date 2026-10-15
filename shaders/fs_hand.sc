$input v_normal
#include <bgfx_shader.sh>
uniform vec4 u_hand_colour;
void main()
{
    // Contraste fixo para ler o volume mesmo em zonas escuras da experiência.
    float shade=0.55+0.45*abs(dot(normalize(v_normal),normalize(vec3(0.3,0.7,-0.6))));
    gl_FragColor=vec4(u_hand_colour.xyz*shade,1.0);
}
