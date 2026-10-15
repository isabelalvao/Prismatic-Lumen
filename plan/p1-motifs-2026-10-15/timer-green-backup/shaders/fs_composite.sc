$input v_uv
#include <bgfx_shader.sh>
SAMPLER2D(s_source,0);
SAMPLER2D(s_bloom,1);
uniform vec4 u_post;
uniform mat4 u_sky;
uniform vec4 u_scene;
uniform vec4 u_clock;
uniform vec4 u_color0;
uniform vec4 u_color1;
uniform vec4 u_eye;
uniform vec4 u_style;
uniform vec4 u_atmosphere;
uniform vec4 u_art;
uniform vec4 u_tuning[22];
uniform vec4 u_choreography[7];
uniform vec4 u_timer[5];
float timer_box(vec2 position,vec2 center,vec2 extent,float softness)
{
    vec2 edge=abs(position-center)-extent;
    return 1.0-smoothstep(-softness,softness,max(edge.x,edge.y));
}
float timer_digit(vec2 position,float digit,float softness)
{
    float ink=0.0;
    if(digit!=1.0 && digit!=4.0)
        ink=max(ink,timer_box(position,vec2(0.25,0.08),vec2(0.17,0.045),softness));
    if(digit!=0.0 && digit!=1.0 && digit!=7.0)
        ink=max(ink,timer_box(position,vec2(0.25,0.5),vec2(0.17,0.045),softness));
    if(digit!=1.0 && digit!=4.0 && digit!=7.0)
        ink=max(ink,timer_box(position,vec2(0.25,0.92),vec2(0.17,0.045),softness));
    if(digit==0.0 || digit==4.0 || digit==5.0 || digit==6.0 || digit==8.0 || digit==9.0)
        ink=max(ink,timer_box(position,vec2(0.06,0.29),vec2(0.045,0.15),softness));
    if(digit==0.0 || digit==2.0 || digit==6.0 || digit==8.0)
        ink=max(ink,timer_box(position,vec2(0.06,0.71),vec2(0.045,0.15),softness));
    if(digit!=5.0 && digit!=6.0)
        ink=max(ink,timer_box(position,vec2(0.44,0.29),vec2(0.045,0.15),softness));
    if(digit!=2.0)
        ink=max(ink,timer_box(position,vec2(0.44,0.71),vec2(0.045,0.15),softness));
    return ink;
}
vec3 timer_overlay(vec3 color,vec2 screen_uv)
{
    if(u_timer[0].y<0.5 || u_timer[1].w<=0.0) return color;
    // Homografia por olho da mesma placa presa à cabeça: os dois visores fundem-se.
    vec3 screen_position=vec3(screen_uv,1.0);
    vec3 mapped=vec3(dot(u_timer[2].xyz,screen_position),dot(u_timer[3].xyz,screen_position),dot(u_timer[4].xyz,screen_position));
    if(mapped.z<=0.000001) return color;
    vec2 timer_position=mapped.xy/mapped.z;
    float softness=max(fwidth(timer_position.y)*0.65,0.006);
    float minutes=floor(u_timer[0].x/600.0);
    float seconds=mod(floor(u_timer[0].x/10.0),60.0);
    float ink=timer_digit(timer_position,floor(minutes/10.0),softness);
    ink=max(ink,timer_digit(timer_position-vec2(0.68,0.0),mod(minutes,10.0),softness));
    ink=max(ink,timer_box(timer_position,vec2(1.30,0.32),vec2(0.035,0.04),softness));
    ink=max(ink,timer_box(timer_position,vec2(1.30,0.68),vec2(0.035,0.04),softness));
    ink=max(ink,timer_digit(timer_position-vec2(1.58,0.0),floor(seconds/10.0),softness));
    ink=max(ink,timer_digit(timer_position-vec2(2.26,0.0),mod(seconds,10.0),softness));
    ink=max(ink,timer_box(timer_position,vec2(2.96,0.92),vec2(0.035,0.04),softness));
    ink=max(ink,timer_digit(timer_position-vec2(3.14,0.0),mod(u_timer[0].x,10.0),softness));
    vec2 edge=abs(timer_position-vec2(1.82,0.5))-vec2(1.98,0.62);
    float plate=1.0-smoothstep(-softness,softness,length(max(edge,vec2(0.0,0.0)))+min(max(edge.x,edge.y),0.0)-0.10);
    color=mix(color,vec3(0.0,0.0,0.0),plate*0.65*u_timer[1].w);
    vec3 ink_color=vec3(0.95,0.97,1.0);
    if(u_post.y>0.5) ink_color=pow(ink_color,vec3(2.2,2.2,2.2));
    return mix(color,ink_color,ink*u_timer[1].w);
}
void main()
{
    vec3 color = texture2D(s_source,v_uv).xyz+texture2D(s_bloom,v_uv).xyz*u_post.x*u_tuning[14].x;
    vec3 direction = normalize(mul(u_sky,vec4(v_uv*2.0-1.0,1.0,0.0)).xyz);
    // Pontos presos ao mundo: queda angular lenta, ciclo vertical periódico sem salto.
    vec2 cells=vec2((atan2(direction.x,direction.z)/6.2831853+0.5)*u_tuning[12].x,
        asin(clamp(direction.y,-1.0,1.0))*u_tuning[12].y+u_tuning[21].w);
    vec2 index=floor(cells);
    index.x=mod(index.x,u_tuning[12].x);
    index.y=mod(index.y,u_tuning[12].z);
    float hash = fract(sin(dot(index,vec2(12.9898,78.233)))*43758.5453);
    float spark = (1.0-smoothstep(u_tuning[13].y,u_tuning[13].z,length(fract(cells)-0.5)))*step(1.0-u_tuning[13].x,hash);
    float empty=1.0-smoothstep(0.005,0.055,max(color.x,max(color.y,color.z)));
    float minimal=mix(1.0,u_tuning[15].x+(1.0-u_tuning[15].x)*smoothstep(0.0,u_tuning[15].y,u_art.z),u_art.w);
    float horizon=smoothstep(-0.04,0.15,direction.y)*(1.0-smoothstep(0.97,1.0,abs(direction.y)));
    color += spark*u_tuning[13].w*u_tuning[20].xyz*u_tuning[20].w*minimal*horizon*empty*u_scene.x;
    if(u_post.w>0.0)
    {
        float horizon=pow(1.0-abs(direction.y),3.0);
        vec3 skyPoint=direction*vec3(3.0,4.0,3.0);
        float cloud=0.5+0.5*sin(skyPoint.x*3.0+sin(skyPoint.z*4.0)+u_clock.x*0.013);
        cloud*=0.5+0.5*sin(skyPoint.x*6.0-skyPoint.y*8.0);
        vec3 horizonColor=u_atmosphere.xyz*0.5+u_color1.xyz*0.018;
        vec3 clouds=mix(u_atmosphere.xyz*0.20,u_color0.xyz*0.09,cloud)*cloud;
        color+=(horizonColor*horizon+clouds*smoothstep(0.0,0.3,direction.y))*u_post.w*empty*u_scene.x;
    }
    if(u_post.z>0.0)
    {
        vec3 distant=normalize(vec3(0.0,260.0,18000.0)-u_eye.xyz);
        float axial=max(0.0,dot(direction,distant));
        float core=pow(axial,230.0);
        float halo=pow(axial,5.0);
        vec3 mistPoint=u_eye.xyz+direction*7000.0;
        float clouds=0.55+0.45*sin(mistPoint.x*0.0007+sin(mistPoint.y*0.001)+u_clock.x*0.07);
        vec3 glowColour=u_style.z>0.5?u_choreography[5].xyz:u_color1.xyz;
        float glowGain=u_style.z>0.5?u_choreography[5].w:1.0;
        vec3 atmosphere=glowColour*(core*0.8+halo*clouds*0.036+0.008)
            +mix(u_color0.xyz,u_color1.xyz,halo)*clouds*0.006*u_scene.z;
        color+=(atmosphere+u_atmosphere.xyz*halo*0.10)*u_scene.x*u_post.z*u_choreography[3].x*glowGain;
    }
    color *= u_tuning[14].y;
    if(u_tuning[14].w>0.5) color = (color*(2.51*color+0.03))/(color*(2.43*color+0.59)+0.14);
    color=clamp(color,0.0,1.0);
    if(u_post.y<0.5) color = pow(color,vec3(1.0/u_tuning[15].z,1.0/u_tuning[15].z,1.0/u_tuning[15].z));
    float vignette = 1.0-u_tuning[14].z*pow(length((v_uv-0.5)*u_tuning[15].w),2.0);
    // Composição final após exposição e bloom, fora da cena e das reflexões.
    // A saída final apaga também pontos, brilho de fundo e contador.
    gl_FragColor = vec4(timer_overlay(color*vignette,v_uv)*u_atmosphere.w,1.0);
}
