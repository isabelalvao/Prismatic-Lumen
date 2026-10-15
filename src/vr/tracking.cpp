#define XR_USE_PLATFORM_WIN32
#define NOMINMAX
#include <Windows.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include "tracking.hpp"
#include <stdexcept>

namespace fantasynt::vr {
namespace {
interaction::PoseSample pose_sample(const XrPosef& pose,XrSpaceLocationFlags flags,float radius=0) {
    interaction::PoseSample result;
    result.position_valid=(flags&XR_SPACE_LOCATION_POSITION_VALID_BIT)!=0;
    result.orientation_valid=(flags&XR_SPACE_LOCATION_ORIENTATION_VALID_BIT)!=0;
    result.position_tracked=(flags&XR_SPACE_LOCATION_POSITION_TRACKED_BIT)!=0;
    result.orientation_tracked=(flags&XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT)!=0;
    // Não ler componentes que o runtime declara indefinidos.
    if(result.position_valid) result.pose.position={pose.position.x,pose.position.y,pose.position.z};
    if(result.orientation_valid) result.pose.orientation={pose.orientation.x,pose.orientation.y,pose.orientation.z,pose.orientation.w};
    if(result.position_valid) result.radius_m=radius;
    return result;
}
template<typename T> bool function(XrInstance instance,const char* name,T& output) {
    return XR_SUCCEEDED(xrGetInstanceProcAddr(instance,name,reinterpret_cast<PFN_xrVoidFunction*>(&output)))&&output;
}
}
struct Tracking::Impl {
    XrInstance instance;
    XrSpace local,view{XR_NULL_HANDLE};
    std::array<XrHandTrackerEXT,2> hands{{XR_NULL_HANDLE,XR_NULL_HANDLE}};
    PFN_xrDestroyHandTrackerEXT destroy_hand{};
    PFN_xrLocateHandJointsEXT locate_hand{};
    PFN_xrConvertTimeToWin32PerformanceCounterKHR convert_time{};
    bool extension_enabled{},system_support{},trackers_ready{};
    std::string hand_reason;
    XrResult hand_setup_result{XR_SUCCESS};
    std::uint64_t sequence{};
    Impl(XrInstance instance_value,XrSystemId system,XrSession session,XrSpace base,bool hand_extension,bool time_extension):
        instance(instance_value),local(base),extension_enabled(hand_extension) {
        XrReferenceSpaceCreateInfo info{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
        info.referenceSpaceType=XR_REFERENCE_SPACE_TYPE_VIEW; info.poseInReferenceSpace.orientation.w=1;
        if(XR_FAILED(xrCreateReferenceSpace(session,&info,&view)))
            throw std::runtime_error("Cannot create OpenXR VIEW space for head observations");
        if(time_extension) function(instance,"xrConvertTimeToWin32PerformanceCounterKHR",convert_time);
        if(!hand_extension) { hand_reason="extension_unavailable"; return; }
        XrSystemHandTrackingPropertiesEXT hand_properties{XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT};
        XrSystemProperties properties{XR_TYPE_SYSTEM_PROPERTIES}; properties.next=&hand_properties;
        hand_setup_result=xrGetSystemProperties(instance,system,&properties);
        if(XR_FAILED(hand_setup_result)) { hand_reason="system_query_failed"; return; }
        system_support=hand_properties.supportsHandTracking==XR_TRUE;
        if(!system_support) { hand_reason="system_not_supported"; return; }
        PFN_xrCreateHandTrackerEXT create_hand{};
        if(!function(instance,"xrCreateHandTrackerEXT",create_hand)||
           !function(instance,"xrDestroyHandTrackerEXT",destroy_hand)||
           !function(instance,"xrLocateHandJointsEXT",locate_hand)) { hand_reason="functions_unavailable"; return; }
        for(std::size_t i=0;i<2;++i) {
            XrHandTrackerCreateInfoEXT create{XR_TYPE_HAND_TRACKER_CREATE_INFO_EXT};
            create.hand=i==0?XR_HAND_LEFT_EXT:XR_HAND_RIGHT_EXT; create.handJointSet=XR_HAND_JOINT_SET_DEFAULT_EXT;
            hand_setup_result=create_hand(session,&create,&hands[i]);
            if(XR_FAILED(hand_setup_result)) {
                for(auto& hand:hands) if(hand!=XR_NULL_HANDLE) { destroy_hand(hand); hand=XR_NULL_HANDLE; }
                hand_reason="tracker_creation_failed"; return;
            }
        }
        trackers_ready=true; hand_reason="ready";
    }
    ~Impl() {
        for(auto hand:hands) if(hand!=XR_NULL_HANDLE&&destroy_hand) destroy_hand(hand);
        if(view!=XR_NULL_HANDLE) xrDestroySpace(view);
    }
    interaction::ObservationFrame sample(XrTime time,std::uint64_t epoch,XrSessionState state,const char* status) {
        interaction::ObservationFrame output;
        output.sequence=++sequence; output.origin_epoch=epoch; output.xr_time_ns=time;
        output.session_state=state; output.frame_status=status;
        const auto before=interaction::monotonic_now_ns();
        if(time>0) {
            XrSpaceLocation head{XR_TYPE_SPACE_LOCATION};
            output.head_locate_result=xrLocateSpace(view,local,time,&head);
            if(XR_SUCCEEDED(output.head_locate_result)) output.head=pose_sample(head.pose,head.locationFlags);
            for(std::size_t i=0;i<2;++i) if(trackers_ready) {
                auto& hand=output.hands[i];
                std::array<XrHandJointLocationEXT,XR_HAND_JOINT_COUNT_EXT> joints{};
                XrHandJointLocationsEXT locations{XR_TYPE_HAND_JOINT_LOCATIONS_EXT};
                locations.jointCount=XR_HAND_JOINT_COUNT_EXT; locations.jointLocations=joints.data();
                XrHandJointsLocateInfoEXT info{XR_TYPE_HAND_JOINTS_LOCATE_INFO_EXT}; info.baseSpace=local; info.time=time;
                hand.locate_result=locate_hand(hands[i],&info,&locations);
                hand.active=XR_SUCCEEDED(hand.locate_result)&&locations.isActive==XR_TRUE;
                hand.status=XR_FAILED(hand.locate_result)?"error":hand.active?"located":"inactive";
                if(hand.active) for(std::size_t j=0;j<joints.size();++j)
                    hand.joints[j]=pose_sample(joints[j].pose,joints[j].locationFlags,joints[j].radius);
            }
            if(convert_time) {
                LARGE_INTEGER predicted{};
                if(XR_SUCCEEDED(convert_time(instance,time,&predicted))) output.predicted_qpc=predicted.QuadPart;
            }
        }
        output.received_monotonic_ns=interaction::monotonic_now_ns();
        output.query_duration_ns=output.received_monotonic_ns-before;
        LARGE_INTEGER now{},frequency{}; QueryPerformanceCounter(&now); QueryPerformanceFrequency(&frequency);
        output.received_qpc=now.QuadPart; output.qpc_frequency=frequency.QuadPart;
        return output;
    }
};
Tracking::Tracking(XrInstance instance,XrSystemId system,XrSession session,XrSpace local,bool hand,bool time):
    impl_(std::make_unique<Impl>(instance,system,session,local,hand,time)) {}
Tracking::~Tracking()=default;
interaction::ObservationFrame Tracking::sample(XrTime time,std::uint64_t epoch,XrSessionState state,const char* status) {
    return impl_->sample(time,epoch,state,status);
}
nlohmann::json Tracking::capabilities() const {
    return {{"head_source","VIEW_space_in_LOCAL"},{"hand_tracking_extension",impl_->extension_enabled},
        {"hand_tracking_system_support",impl_->system_support},{"hand_trackers_ready",impl_->trackers_ready},
        {"hand_tracking_reason",impl_->hand_reason},{"hand_setup_result",impl_->hand_setup_result},
        {"xr_to_qpc_available",impl_->convert_time!=nullptr},{"joint_count",interaction::hand_joint_count}};
}
}
