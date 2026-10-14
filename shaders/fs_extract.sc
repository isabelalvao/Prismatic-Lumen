$input v_uv
#include <bgfx_shader.sh>
SAMPLER2D(s_source,0);
void main()
{
    vec3 color = texture2D(s_source,v_uv).xyz;
    float brightness = max(color.x,max(color.y,color.z));
    gl_FragColor = vec4(color*max(brightness-0.85,0.0)/max(brightness,0.0001),1.0);
}
