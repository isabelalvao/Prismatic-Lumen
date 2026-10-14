#include "art_direction.hpp"
#include "sequence.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

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
bool blocked(const Footprint& bounds,const VisualConfig& c) {
    const float half=static_cast<float>(c[Setting::CorridorWidth]*50);
    return bounds.high_y>c[Setting::CorridorFloor]*100&&bounds.low_y<c[Setting::CorridorCeiling]*100&&bounds.low_x<half&&bounds.high_x>-half;
}
}
ArtMotion sample_art_motion(double time,const VisualConfig& c) {
    const double multiplier=c[Setting::AnimationSpeed];
    return {time*multiplier*c[Setting::PointSpeed]*100,time*multiplier*c[Setting::TraceSpeed]*100,
        std::max(0.0,time-c[Setting::PyramidStart])*multiplier*c[Setting::PyramidRotation]*3.141592653589793/180,
        time*multiplier*c[Setting::SkySpeed]};
}
Composition directed_composition(double time,bool opening_only,const VisualConfig& config) {
    if(opening_only) time=std::clamp(time,0.0,config[Setting::OpeningDuration]);
    auto result=sample_sequence(time);
    result.opening_weight=opening_only?1:1-smooth(config[Setting::OpeningDuration],config[Setting::OpeningDuration]+config[Setting::Handoff],time);
    const float forest=smooth(80,112,time),tunnel=smooth(224,232,time),legacy=1-result.opening_weight;
    result.weights={(1-forest)*legacy,forest*(1-tunnel)*legacy,tunnel*legacy};
    if(opening_only) result.exposure=1;
    return result;
}
double directed_camera_position(double seconds,const VisualConfig& c) {
    if(!std::isfinite(seconds)) seconds=0;
    seconds=std::clamp(seconds,0.0,sequence_duration);
    const double duration=c[Setting::OpeningDuration];
    const double t=std::max(0.0,std::min(seconds,duration)-c[Setting::CameraDelay]);
    const double ramp=c[Setting::CameraAcceleration],speed=c[Setting::CameraSpeed]*100;
    const double distance=ramp>0&&t<ramp?speed*(t*t*t/(ramp*ramp)-t*t*t*t/(2*ramp*ramp*ramp)):speed*(t-ramp*0.5);
    const double opening=(c[Setting::CameraStartZ]*c[Setting::WorldScale]+c[Setting::OffsetZ])*100+std::max(0.0,distance);
    return seconds<=duration?opening:opening+sample_camera_position(seconds)-sample_camera_position(duration);
}
double directed_camera_step(double from,double to,float speed,const VisualConfig& c) {
    return (directed_camera_position(to,c)-directed_camera_position(from,c))*(speed/0.3f);
}
float opening_art_presence(ArtRole role,double time,double start,const VisualConfig& c) {
    if(!std::isfinite(time)||time<start) return 0;
    if(role==ArtRole::Point) return static_cast<float>(c[Setting::PointInitial]+(c[Setting::PointFinal]-c[Setting::PointInitial])*smooth(0,c[Setting::PointGrowth],time));
    if(role==ArtRole::Pyramid) return smooth(c[Setting::PyramidStart],c[Setting::PyramidStart]+c[Setting::PyramidReveal],time);
    return static_cast<float>(c[Setting::InitialPresence]+(1-c[Setting::InitialPresence])*smooth(0,c[Setting::PresenceGrowth],time));
}
float falling_point_height(float height,double seconds,const VisualConfig& c) {
    if(!std::isfinite(seconds)) seconds=0;
    const double span=c[Setting::PointHeight]*100;
    const double y=std::fmod(static_cast<double>(height)-sample_art_motion(seconds,c).points,span);
    return static_cast<float>(y<0?y+span:y);
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
double directed_group_start(const OpeningGroup& group,const VisualConfig& c) {
    if(group.role==ArtRole::Pyramid) return c[Setting::PyramidStart];
    if(group.stage==OpeningStage::Forest) return c[group.start_time>=112?Setting::GroundStart:Setting::ForestStart];
    return group.start_time;
}
bool directed_group_visible(const OpeningGroup& group,double time,const VisualConfig& c) {
    if(!c.family_enabled(group.role)||!std::isfinite(time)||time<directed_group_start(group,c)) return false;
    return group.stage==OpeningStage::Forest||time<c[group.stage==OpeningStage::Dome?Setting::DomeEnd:Setting::IntroEnd];
}
Instance directed_instance(const Instance& source,const OpeningGroup& group,double time,const VisualConfig& c) {
    return directed_instance_with_motion(source,group,sample_art_motion(time,c),c);
}
Instance directed_instance_with_motion(const Instance& source,const OpeningGroup& group,const ArtMotion& motion,const VisualConfig& c) {
    auto result=source;
    if(group.role==ArtRole::Point) {
        const double span=c[Setting::PointHeight]*100;
        const double height=std::fmod(source.transform[13]-motion.points,span);
        result.transform[13]=static_cast<float>(height<0?height+span:height);
        // Partículas pequenas: preserva a posição e reduz só o volume luminoso.
        for(int column=0;column<3;++column) for(int row=0;row<3;++row) result.transform[column*4+row]*=static_cast<float>(c[Setting::PointSize]);
    }
    if(group.role==ArtRole::Pyramid) {
        const double angle=motion.pyramids*(c[Setting::PyramidAlternate]&&source.padding>=0.5f?-1:1);
        const float c=static_cast<float>(std::cos(angle)),s=static_cast<float>(std::sin(angle));
        const auto pivot=world_point(source,group.pivot);
        auto& m=result.transform;
        for(int column=0;column<3;++column) { const int k=column*4; m[k]=c*source.transform[k]+s*source.transform[k+2]; m[k+2]=-s*source.transform[k]+c*source.transform[k+2]; }
        const float dx=source.transform[12]-pivot[0],dz=source.transform[14]-pivot[2];
        m[12]=pivot[0]+c*dx+s*dz; m[14]=pivot[2]-s*dx+c*dz;
    }
    return result;
}
bool intersects_directed_route(const Instance& source,const OpeningGroup& group,const SurfaceGeometry&,const VisualConfig& c) {
    return group.role!=ArtRole::Point&&blocked(footprint(source,group),c);
}
std::size_t clear_directed_route(OpeningScene& scene,const VisualConfig& c) {
    std::size_t moved{};
    for(auto& group:scene.groups) for(auto& instance:group.instances) {
        if(group.role==ArtRole::Point) continue;
        const auto bounds=footprint(instance,group);
        if(!blocked(bounds,c)) continue;
        const float midpoint=(bounds.low_x+bounds.high_x)*0.5f;
        const bool right=midpoint>0||(midpoint==0&&instance.padding>=0.5f);
        const float edge=static_cast<float>((c[Setting::CorridorWidth]*0.5+c[Setting::CorridorMargin])*100);
        instance.transform[12]+=right?edge-bounds.low_x:-edge-bounds.high_x;
        ++moved;
    }
    return moved;
}
std::size_t extend_opening_intro(OpeningScene& scene,const VisualConfig& c) {
    std::size_t added{};
    std::size_t prepared{};
    for(const auto& g:scene.groups) prepared+=g.instances.size()*(g.stage==OpeningStage::Forest?1:static_cast<std::size_t>(c[Setting::IntroSectors]));
    if(prepared>65000) throw std::runtime_error("Visual config: layout exceeds 65000 prepared instances");
    // O avanço aos10s exige ambiente ao longo do percurso, antes de Forest aos80s.
    for(auto& group:scene.groups) {
        for(auto& instance:group.instances) {
            auto& m=instance.transform;
            for(int col=0;col<3;++col) for(int row=0;row<3;++row) m[col*4+row]*=static_cast<float>(c[Setting::WorldScale]);
            const Setting offsets[]={Setting::OffsetX,Setting::OffsetY,Setting::OffsetZ};
            for(int row=0;row<3;++row) m[12+row]=static_cast<float>(m[12+row]*c[Setting::WorldScale]+c[offsets[row]]*100);
        }
        if(group.stage==OpeningStage::Forest) continue;
        const auto count=group.instances.size();
        const int sectors=static_cast<int>(c[Setting::IntroSectors]);
        group.instances.reserve(count*sectors);
        for(int sector=1;sector<sectors;++sector) for(std::size_t i=0;i<count;++i) {
            auto instance=group.instances[i];
            instance.transform[14]+=static_cast<float>(sector*c[Setting::SectorSpacing]*100);
            instance.padding=std::fmod(instance.padding+sector*0.197f,1.0f);
            group.instances.push_back(instance); ++added;
        }
    }
    return added;
}
}
