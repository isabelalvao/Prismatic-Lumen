$input v_uv
#include <bgfx_shader.sh>
SAMPLER2D(s_source,0);
uniform vec4 u_post;
void main()
{
    vec3 color = texture2D(s_source,v_uv).xyz;
    if(u_post.y>0.5) color = pow(max(color,vec3(0.0,0.0,0.0)),vec3(1.0/2.2,1.0/2.2,1.0/2.2));
    gl_FragColor = vec4(color,1.0);
}
