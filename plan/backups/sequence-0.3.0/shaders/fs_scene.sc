$input v_world, v_local, v_normal, v_meta, v_clip, v_pattern
#include <bgfx_shader.sh>
uniform vec4 u_scene;
uniform vec4 u_clock;
uniform vec4 u_color0;
uniform vec4 u_color1;
uniform vec4 u_eye;
float ledDots(vec2 uv,vec2 grid,float radius)
{
    vec2 cells=uv*grid;
    vec2 pixel=abs(fract(cells)-0.5);
    float footprint=max(fwidth(cells.x),fwidth(cells.y));
    float pointMask=1.0-smoothstep(radius,radius+max(footprint*0.6,0.035),max(pixel.x,pixel.y));
    return mix(pointMask,radius*radius*4.0,smoothstep(0.6,1.5,footprint));
}
void main()
{
    float phase=v_meta.y*6.2831853;
    float beat=pow(max(0.0,sin(u_clock.x*12.5663706-phase*0.25)),6.0);
    float tunnel=step(2.5,v_meta.z);
    float depth=smoothstep(1800.0,8000.0,v_world.z);
    vec3 color=mix(u_color0.xyz,u_color1.xyz,clamp(depth+step(0.85,v_meta.x)*0.3,0.0,1.0));
    vec2 uv=v_pattern;
    float mask=0.0;
    if(tunnel>0.5)
    {
        float travel=uv.x*7.0+uv.y*3.0-u_clock.x*0.3+v_meta.y*4.0;
        float bands=1.0-smoothstep(0.035,0.075,abs(fract(uv.y*5.0+u_clock.x*0.08)-0.5));
        float wave=0.5+0.5*sin(travel*6.2831853);
        float islands=smoothstep(0.53,0.68,wave);
        float dots=ledDots(uv,vec2(48.0,24.0),0.26);
        if(v_meta.z<3.5)
        {
            vec3 inward=normalize(vec3(-v_world.x,260.0-v_world.y,0.0));
            float front=step(0.5,dot(normalize(v_normal),inward));
            float frame=smoothstep(0.06,0.13,min(min(uv.x,1.0-uv.x),min(uv.y,1.0-uv.y)));
            float scribble=1.0-smoothstep(0.025,0.065,abs(uv.y-0.5-0.35*sin(uv.x*13.0+u_clock.x*0.4+phase)));
            float diagonal=1.0-smoothstep(0.025,0.065,abs(fract(uv.x*3.0+uv.y*2.0+u_clock.x*0.12)-0.5));
            float pattern=v_meta.x<0.33?bands:v_meta.x<0.66?max(scribble,diagonal):islands;
            mask=pattern*ledDots(uv,vec2(128.0,96.0),0.35)*front*frame;
        }
        else if(v_meta.z<4.5) mask=dots*step(0.30,fract(uv.x*6.0+v_meta.y))*step(0.25,fract(uv.y*4.0));
        else if(v_meta.z<5.5) mask=(0.04+dots*0.4)*step(0.35,v_meta.x);
        else if(v_meta.z<6.5) mask=ledDots(uv,vec2(72.0,9.0),0.25)*islands*step(0.5,v_meta.x);
        else mask=0.5;
        mask*=0.55+0.45*pow(0.5+0.5*sin(u_clock.x*0.8+phase),2.0);
    }
    else
    {
        color=mix(u_color0.xyz,u_color1.xyz,v_meta.x*0.55);
        vec2 face=abs(v_normal.z)>0.5?v_local.xy:v_local.zy;
        float contour=face.y+2.0*abs(face.x);
        float chevron=abs(fract(contour*3.0+u_clock.x*0.025+v_meta.y*0.1)-0.5);
        float lines=1.0-smoothstep(0.035,0.035+max(fwidth(contour)*3.0,0.006),chevron);
        float outer=1.0-smoothstep(0.018,0.018+fwidth(contour),abs(contour-1.0));
        float stripes=1.0-smoothstep(0.04,0.095,abs(fract((face.y+1.0)*5.5)-0.5));
        mask=(v_meta.z<0.55?max(lines,outer):stripes)*ledDots(uv,vec2(120.0,160.0),0.43);
        mask*=0.10+0.90*pow(0.5+0.5*sin(u_clock.x*0.65+phase),3.0);
    }
    vec3 normal=normalize(v_normal);
    vec3 viewDirection=normalize(u_eye.xyz-v_world);
    if(dot(normal,viewDirection)<0.0) normal=-normal;
    vec3 lightDirection=normalize(vec3(-0.4,0.45,-0.8));
    float diffuse=max(0.0,dot(normal,lightDirection));
    float highlight=pow(max(0.0,dot(reflect(-lightDirection,normal),viewDirection)),48.0);
    float rim=pow(1.0-max(0.0,dot(normal,viewDirection)),3.0);
    vec3 body=mix(vec3(0.009,0.012,0.02),color*0.026,0.65);
    body*=0.45+diffuse*2.0;
    vec3 reflected=reflect(-viewDirection,normal);
    vec3 reflectedColor=mix(u_color0.xyz,u_color1.xyz,pow(max(0.0,reflected.z),3.0));
    body+=reflectedColor*(highlight*0.2+rim*0.016);
    vec3 rgb=body*u_scene.x+color*mask*u_scene.x*(2.8+beat*u_scene.y*3.0);
    float distanceToEye=length(v_world-u_eye.xyz);
    float fog=exp(-distanceToEye*(0.000008+u_scene.z*0.000065));
    vec3 haze=mix(u_color0.xyz,u_color1.xyz,depth)*u_scene.x*0.025;
    rgb=mix(haze,rgb,fog);
    gl_FragColor=vec4(rgb,1.0);
}
