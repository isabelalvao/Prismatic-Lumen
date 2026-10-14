$input v_uv
#include <bgfx_shader.sh>
SAMPLER2D(s_source,0);
uniform vec4 u_blur;
void main()
{
    vec2 offset = u_blur.xy;
    vec3 color = vec3(0.0,0.0,0.0);
    float total = 0.0;
    for(int i=-8;i<=8;++i) {
        float distance = float(i);
        float weight = exp(-distance*distance/32.0);
        color += texture2D(s_source,v_uv+offset*distance).xyz*weight;
        total += weight;
    }
    color /= total;
    gl_FragColor = vec4(color,1.0);
}
