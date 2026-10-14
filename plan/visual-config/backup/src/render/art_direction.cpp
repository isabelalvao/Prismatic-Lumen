#include "art_direction.hpp"
#include "sequence.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace fantasynt {
namespace {
float smooth(double a,double b,double t) {
    const auto x=static_cast<float>(std::clamp((t-a)/(b-a),0.0,1.0));
    return x*x*(3-2*x);
}
std::array<float,3> world_point(const Instance& instance,const std::array<float,3>& local) {
    const auto& m=instance.transform;
    return {m[0]*local[0]+m[4]*local[1]+m[8]*local[2]+m[12],
        m[1]*local[0]+m[5]*local[1]+m[9]*local[2]+m[13],
        m[2]*local[0]+m[6]*local[1]+m[10]*local[2]+m[14]};
}
struct Footprint { float low_x,high_x,low_y,high_y; };
Footprint footprint(const Instance& instance,const OpeningGroup& group) {
    Footprint bounds{std::numeric_limits<float>::max(),std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::max(),std::numeric_limits<float>::lowest()};
    const auto pivot=world_point(instance,group.pivot);
    float radius{};
    for(int corner=0;corner<8;++corner) {
        std::array<float,3> point{};
        for(int axis=0;axis<3;++axis) point[axis]=(corner&(1<<axis))?group.bounds_max[axis]:group.bounds_min[axis];
        const auto world=world_point(instance,point);
        bounds.low_x=std::min(bounds.low_x,world[0]); bounds.high_x=std::max(bounds.high_x,world[0]);
        bounds.low_y=std::min(bounds.low_y,world[1]); bounds.high_y=std::max(bounds.high_y,world[1]);
        radius=std::max(radius,std::hypot(world[0]-pivot[0],world[2]-pivot[2]));
    }
    // Engloba a rotação inteira: um vértice não pode entrar no percurso mais tarde.
    if(group.role==ArtRole::Pyramid) { bounds.low_x=pivot[0]-radius; bounds.high_x=pivot[0]+radius; }
    return bounds;
}
bool blocked(const Footprint& bounds) {
    return bounds.high_y>10&&bounds.low_y<550&&bounds.low_x<200&&bounds.high_x>-200;
}
}
double directed_camera_position(double seconds) {
    if(!std::isfinite(seconds)) seconds=0;
    seconds=std::clamp(seconds,0.0,sequence_duration);
    const double t=std::clamp(seconds-10,0.0,110.0);
    // Aceleração suave em dois segundos; 150 cm/s, menos25% que os200 cm/s anteriores.
    const double distance=t<2?150*(t*t*t/4-t*t*t*t/16):150*(t-1);
    const double opening=sample_camera_position(0)+distance;
    return seconds<=120?opening:opening+sample_camera_position(seconds)-sample_camera_position(120);
}
double directed_camera_step(double from,double to,float speed) {
    return (directed_camera_position(to)-directed_camera_position(from))*(speed/0.3f);
}
float opening_art_presence(ArtRole role,double time,double start) {
    if(!std::isfinite(time)||time<start) return 0;
    if(role==ArtRole::Point) return 0.05f+0.65f*smooth(0,40,time);
    if(role==ArtRole::Pyramid) return smooth(std::max(80.0,start),std::max(80.0,start)+8,time);
    return 0.15f*smooth(10,20,time)+0.85f*smooth(20,60,time);
}
float falling_point_height(float height,double seconds) {
    if(!std::isfinite(seconds)) seconds=0;
    const double y=std::fmod(static_cast<double>(height)-seconds*10,6000.0);
    return static_cast<float>(y<0?y+6000:y);
}
ArtRole classify_art_role(std::string_view name,int material) {
    if(name.find("RockLight")!=name.npos) return ArtRole::Point;
    if(material==8) return ArtRole::Pulse;
    if(material==1||material==2) return ArtRole::Hex;
    if((material>=3&&material<=7)||name.find("Tube")!=name.npos) return ArtRole::Trace;
    if(material==10) return ArtRole::Pyramid;
    if(material==9||material==11) return ArtRole::Pole;
    return ArtRole::Dark;
}
void prepare_art_bounds(OpeningGroup& group,const SurfaceGeometry& mesh) {
    group.role=classify_art_role(group.name,group.material);
    group.bounds_min.fill(std::numeric_limits<float>::max());
    group.bounds_max.fill(std::numeric_limits<float>::lowest());
    for(const auto& v:mesh.vertices) {
        const float point[]={v.x,v.y,v.z};
        for(int i=0;i<3;++i) { group.bounds_min[i]=std::min(group.bounds_min[i],point[i]); group.bounds_max[i]=std::max(group.bounds_max[i],point[i]); }
    }
    group.pivot={(group.bounds_min[0]+group.bounds_max[0])*0.5f,group.bounds_min[1],
        (group.bounds_min[2]+group.bounds_max[2])*0.5f};
}
Instance directed_instance(const Instance& source,const OpeningGroup& group,double time) {
    auto result=source;
    if(group.role==ArtRole::Point) {
        result.transform[13]=falling_point_height(source.transform[13],time);
        // Partículas pequenas: preserva a posição e reduz só o volume luminoso.
        for(int column=0;column<3;++column) for(int row=0;row<3;++row) result.transform[column*4+row]*=0.15f;
    }
    if(group.role==ArtRole::Pyramid) {
        const double seconds=std::max(0.0,time-group.start_time);
        const double angle=seconds*(0.25*3.141592653589793/180)*(source.padding<0.5f?1:-1);
        const float c=static_cast<float>(std::cos(angle)),s=static_cast<float>(std::sin(angle));
        const auto pivot=world_point(source,group.pivot);
        auto& m=result.transform;
        for(int column=0;column<3;++column) { const int k=column*4; m[k]=c*source.transform[k]+s*source.transform[k+2]; m[k+2]=-s*source.transform[k]+c*source.transform[k+2]; }
        const float dx=source.transform[12]-pivot[0],dz=source.transform[14]-pivot[2];
        m[12]=pivot[0]+c*dx+s*dz; m[14]=pivot[2]-s*dx+c*dz;
    }
    return result;
}
bool intersects_directed_route(const Instance& source,const OpeningGroup& group,const SurfaceGeometry&) {
    return group.role!=ArtRole::Point&&blocked(footprint(source,group));
}
std::size_t clear_directed_route(OpeningScene& scene) {
    std::size_t moved{};
    for(auto& group:scene.groups) for(auto& instance:group.instances) {
        if(group.role==ArtRole::Point) continue;
        const auto bounds=footprint(instance,group);
        if(!blocked(bounds)) continue;
        const float midpoint=(bounds.low_x+bounds.high_x)*0.5f;
        const bool right=midpoint>0||(midpoint==0&&instance.padding>=0.5f);
        instance.transform[12]+=right?220-bounds.low_x:-220-bounds.high_x;
        ++moved;
    }
    return moved;
}
std::size_t extend_opening_intro(OpeningScene& scene) {
    std::size_t added{};
    // O avanço aos10s exige ambiente ao longo do percurso, antes de Forest aos80s.
    for(auto& group:scene.groups) {
        if(group.stage==OpeningStage::Forest) continue;
        const auto count=group.instances.size();
        group.instances.reserve(count*5);
        for(int sector=1;sector<5;++sector) for(std::size_t i=0;i<count;++i) {
            auto instance=group.instances[i];
            instance.transform[14]+=sector*3200;
            instance.padding=std::fmod(instance.padding+sector*0.197f,1.0f);
            group.instances.push_back(instance); ++added;
        }
    }
    return added;
}
}
