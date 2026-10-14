$input v_world, v_local, v_normal, v_meta, v_clip
#include <bgfx_shader.sh>
uniform vec4 u_scene;
uniform vec4 u_clock;
uniform vec4 u_color0;
uniform vec4 u_color1;
uniform vec4 u_eye;
void main()
{
    vec3 color = mix(u_color0.xyz,u_color1.xyz,v_meta.x*0.55);
    vec2 face = abs(v_normal.z)>0.5 ? v_local.xy : v_local.zy;
    float pyramid = 1.0-step(0.55,v_meta.z);
    float phase = v_meta.y*6.2831853;
    float pulse = pow(max(0.0,sin(u_clock.x*12.5663706-phase*0.2)),8.0);
    float contour = face.y+2.0*abs(face.x);
    float chevron = abs(fract(contour*3.0+u_clock.x*0.025+v_meta.y*0.1)-0.5);
    float antialias = max(fwidth(contour)*3.0,0.006);
    float lines = 1.0-smoothstep(0.035,0.035+antialias,chevron);
    float outer = 1.0-smoothstep(0.018,0.018+fwidth(contour),abs(contour-1.0));
    float stripes = 1.0-smoothstep(0.04,0.095,abs(fract((face.y+1.0)*5.5)-0.5));
    float pixelX = 0.82+0.18*step(0.2,fract(face.x*90.0));
    float pixelY = 0.82+0.18*step(0.2,fract(face.y*120.0));
    float border = 1.0-smoothstep(0.025,0.06,1.0-max(abs(face.x),abs(face.y)));
    float lightMask = pyramid>0.5 ? max(lines,outer)*pixelX*pixelY : stripes*pixelX*pixelY+border*0.5;
    if(abs(v_normal.y)>0.5) lightMask *= 0.25;
    float breathing = 0.08+0.92*pow(0.5+0.5*sin(u_clock.x*0.65+phase),3.0);
    float emission = u_scene.x*(1.2+u_scene.y*pulse*2.3)*breathing;
    vec3 rgb = color*(0.005+0.013*max(0.0,v_normal.y))+color*lightMask*emission*3.5;
    float distance = length(v_world-u_eye.xyz);
    float fog = exp(-distance*(0.000012+u_scene.z*0.00012));
    rgb = mix(vec3(0.0007,0.0013,0.0032),rgb,fog);
    gl_FragColor = vec4(rgb,1.0);
}
