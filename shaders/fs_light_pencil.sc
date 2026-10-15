$input v_normal, v_pattern
#include <bgfx_shader.sh>
uniform vec4 u_hand_colour;
void main()
{
    float fade=clamp(v_pattern.x,0.0,1.0);
    float shade=0.75+0.25*abs(dot(normalize(v_normal),normalize(vec3(0.3,0.7,-0.6))));
    gl_FragColor=vec4(u_hand_colour.xyz*fade*fade*shade,1.0);
}
