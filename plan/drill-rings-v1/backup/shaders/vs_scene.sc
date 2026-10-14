$input a_position, a_normal, a_texcoord0, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_world, v_local, v_normal, v_meta, v_clip, v_pattern
#include <bgfx_shader.sh>
uniform vec4 u_scene;
uniform vec4 u_clock;
uniform vec4 u_style;
uniform vec4 u_choreography[5];
void main()
{
    vec3 local = a_position;
    vec3 localNormal = a_normal;
    float pieceVisibility = 1.0;
    float ground = 1.0-step(0.1,abs(i_data4.z-2.0));
    float tunnel = step(2.5,i_data4.z)*(1.0-step(7.5,i_data4.z));
    float pyramid = 1.0-step(0.55,i_data4.z)+1.0-step(0.1,abs(i_data4.z-10.0));
    if (ground < 0.5 && pyramid > 0.5 && local.y > 0.0) local.xz = vec2(0.0,0.0);
    // A densidade continua a selecionar o mesmo conjunto estável de instâncias.
    float density = clamp((u_scene.w-i_data4.w)*64.0, 0.0, 1.0)*mix(u_style.x,1.0,step(0.5,u_style.z));
    if (ground < 0.5 && i_data4.z<2.0) local.y = -1.0 + (local.y+1.0)*density;
    if (ground < 0.5 && i_data4.z>2.5) local *= max(density,0.0001);
    if (u_style.z > 0.5 && ground < 0.5)
    {
        float seed = clamp(i_data4.y,0.0,1.0);
        float sceneTime = u_choreography[0].x;
        float revealSeconds = max(u_choreography[0].w,0.0001);
        if (i_data4.z > 8.5 && i_data4.z < 9.5)
        {
            // O índice usa a altura original, incluindo as duas tampas de cada anel.
            float segment = clamp(floor(a_position.y*3.5+0.0001),0.0,6.0);
            float delay = segment*u_choreography[0].z+seed*u_choreography[1].z;
            pieceVisibility = smoothstep(0.0,revealSeconds,sceneTime-u_choreography[0].y-delay);
            float angle = i_data0.w*step(1.0,pieceVisibility);
            float c = cos(angle), s = sin(angle);
            local.xz = vec2(c*local.x+s*local.z,-s*local.x+c*local.z);
            localNormal.xz = vec2(c*a_normal.x+s*a_normal.z,-s*a_normal.x+c*a_normal.z);
            float centerY = (segment+0.365)*2.0/7.0*max(density,0.0001);
            local = vec3(local.x,(local.y-centerY),local.z)*pieceVisibility+vec3(0.0,centerY,0.0);
        }
        else if (i_data4.z > 7.5)
        {
            float familyStart = u_style.y<0.5 ? u_choreography[4].x : u_choreography[0].y;
            float delay = seed*max(0.0,u_choreography[3].w-revealSeconds);
            pieceVisibility = smoothstep(0.0,revealSeconds,sceneTime-familyStart-delay);
            local *= pieceVisibility;
        }
        if (i_data4.z > 7.5)
        {
            float exitSeed = clamp(i_data4.w,0.0,1.0);
            float exitVisibility = 1.0-smoothstep(exitSeed*0.65,exitSeed*0.65+0.35,u_choreography[2].x);
            // A saída fecha cada peça no seu centro, sem cobertura aleatória de píxeis.
            if (i_data4.z > 8.5 && i_data4.z < 9.5)
            {
                float segment = clamp(floor(a_position.y*3.5+0.0001),0.0,6.0);
                float centerY = (segment+0.365)*2.0/7.0*max(density,0.0001);
                local = (local-vec3(0.0,centerY,0.0))*exitVisibility+vec3(0.0,centerY,0.0);
            }
            else local *= exitVisibility;
            pieceVisibility *= exitVisibility;
        }
        if (tunnel > 0.5)
        {
            float reveal = max(u_choreography[2].w,0.0001);
            float delay = seed*max(0.0,u_choreography[2].z-reveal);
            pieceVisibility = step(1.0,u_choreography[2].x)
                *smoothstep(0.0,reveal,sceneTime-u_choreography[2].y-delay);
            local *= pieceVisibility;
        }
    }
    mat4 model = mtxFromCols(i_data0,i_data1,i_data2,i_data3);
    vec4 world = mul(model,vec4(local,1.0));
    // A primeira coluna transporta o ângulo calculado por segmento, sem alterar a posição afim.
    if(u_style.z>0.5 && i_data4.z>8.5 && i_data4.z<9.5) world.w=1.0;
    if (ground < 0.5) world.z += mod(i_data3.z-u_clock.y+u_clock.z+tunnel*1600.0,u_clock.z)-tunnel*1600.0-i_data3.z;
    if (u_clock.w > 0.5) world.y = -world.y;
    v_world = world.xyz;
    v_local = u_style.z > 0.5 ? a_position : local;
    v_normal = normalize(i_data0.xyz*localNormal.x/dot(i_data0.xyz,i_data0.xyz)
        +i_data1.xyz*localNormal.y/dot(i_data1.xyz,i_data1.xyz)
        +i_data2.xyz*localNormal.z/dot(i_data2.xyz,i_data2.xyz));
    if(u_clock.w>0.5) v_normal.y=-v_normal.y;
    v_pattern = abs(a_normal.y)>0.5 ? a_position.xz*0.5+0.5 : abs(a_normal.z)>0.5 ? a_position.xy*0.5+0.5 : a_position.zy*0.5+0.5;
    if(i_data4.z>3.5) v_pattern=a_texcoord0;
    v_meta = i_data4;
    if (u_style.z > 0.5) v_meta.w = pieceVisibility;
    gl_Position = mul(u_viewProj,world);
    if (ground < 0.5 && (density <= 0.0 || pieceVisibility <= 0.0)) gl_Position = vec4(2.0,2.0,2.0,1.0);
    v_clip = gl_Position;
}
