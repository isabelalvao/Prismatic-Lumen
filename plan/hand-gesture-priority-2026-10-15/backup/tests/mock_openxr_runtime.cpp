// Runtime exclusivo dos testes: valida a ordem OpenXR e lê as imagens D3D11 submetidas.
#define NOMINMAX
#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include <openxr/openxr_loader_negotiation.h>
#include <array>
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <vector>

namespace {
using Microsoft::WRL::ComPtr;
ComPtr<ID3D11Device> device;
ComPtr<ID3D11DeviceContext> context;
struct Chain {
    std::array<ComPtr<ID3D11Texture2D>,3> images;
    std::uint32_t index{},last{};
    bool acquired{},waited{},released{};
};
std::vector<Chain*> chains;
int event_number{},frame_number{};
bool running{},frame_open{},wait_called{},should_render{},valid_tracking{};
bool loss_reported{};
bool hand_extension_enabled{},time_extension_enabled{},reference_reported{};
std::int64_t time_qpc_origin{};
int trackers{};
XrInstance instance_handle=reinterpret_cast<XrInstance>(1);
XrSession session_handle=reinterpret_cast<XrSession>(2);
bool mode(const char* name) { const char* value=std::getenv("FANTASYNT_MOCK_MODE"); return value&&std::strcmp(value,name)==0; }
XrResult XRAPI_CALL extensions(const char*,uint32_t capacity,uint32_t* count,XrExtensionProperties* output) {
    *count=mode("no-hands")?1:mode("time-conversion")?3:2;
    if(!capacity) return XR_SUCCESS; if(capacity<*count) return XR_ERROR_SIZE_INSUFFICIENT;
    std::strcpy(output[0].extensionName,XR_KHR_D3D11_ENABLE_EXTENSION_NAME); output[0].extensionVersion=9;
    if(*count>=2) { std::strcpy(output[1].extensionName,XR_EXT_HAND_TRACKING_EXTENSION_NAME); output[1].extensionVersion=4; }
    if(*count==3) { std::strcpy(output[2].extensionName,XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME); output[2].extensionVersion=1; }
    return XR_SUCCESS;
}
XrResult XRAPI_CALL create_instance(const XrInstanceCreateInfo* info,XrInstance* output) {
    hand_extension_enabled=time_extension_enabled=false;
    for(std::uint32_t i=0;i<info->enabledExtensionCount;++i) {
        if(std::strcmp(info->enabledExtensionNames[i],XR_EXT_HAND_TRACKING_EXTENSION_NAME)==0) hand_extension_enabled=true;
        if(std::strcmp(info->enabledExtensionNames[i],XR_KHR_WIN32_CONVERT_PERFORMANCE_COUNTER_TIME_EXTENSION_NAME)==0) time_extension_enabled=true;
    }
    *output=instance_handle; return XR_SUCCESS;
}
XrResult XRAPI_CALL destroy_instance(XrInstance) { return XR_SUCCESS; }
XrResult XRAPI_CALL instance_properties(XrInstance,XrInstanceProperties* output) {
    std::strcpy(output->runtimeName,"FantasyntMock"); output->runtimeVersion=XR_MAKE_VERSION(1,0,0); return XR_SUCCESS;
}
XrResult XRAPI_CALL get_system(XrInstance,const XrSystemGetInfo*,XrSystemId* output) {
    if(mode("no-headset")) return XR_ERROR_FORM_FACTOR_UNAVAILABLE;
    *output=1; return XR_SUCCESS;
}
XrResult XRAPI_CALL system_properties(XrInstance,XrSystemId,XrSystemProperties* output) {
    output->systemId=1; std::strcpy(output->systemName,"Simulated stereo D3D11 headset");
    if(output->next) {
        auto* hand=static_cast<XrSystemHandTrackingPropertiesEXT*>(output->next);
        if(hand->type!=XR_TYPE_SYSTEM_HAND_TRACKING_PROPERTIES_EXT||!hand_extension_enabled) return XR_ERROR_VALIDATION_FAILURE;
        hand->supportsHandTracking=mode("no-hand-support")?XR_FALSE:XR_TRUE;
    }
    return XR_SUCCESS;
}
XrResult XRAPI_CALL requirements(XrInstance,XrSystemId,XrGraphicsRequirementsD3D11KHR* output) {
    ComPtr<IDXGIFactory1> factory; ComPtr<IDXGIAdapter1> adapter; DXGI_ADAPTER_DESC1 description{};
    if(FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))||FAILED(factory->EnumAdapters1(0,&adapter))||FAILED(adapter->GetDesc1(&description))) return XR_ERROR_RUNTIME_FAILURE;
    output->adapterLuid=description.AdapterLuid; output->minFeatureLevel=D3D_FEATURE_LEVEL_11_0; return XR_SUCCESS;
}
XrResult XRAPI_CALL create_session(XrInstance,const XrSessionCreateInfo* info,XrSession* output) {
    const auto* binding=static_cast<const XrGraphicsBindingD3D11KHR*>(info->next);
    if(!binding||binding->type!=XR_TYPE_GRAPHICS_BINDING_D3D11_KHR||!binding->device) return XR_ERROR_GRAPHICS_DEVICE_INVALID;
    device=binding->device; device->GetImmediateContext(&context); *output=session_handle;
    event_number=frame_number=trackers=0; loss_reported=reference_reported=false; return XR_SUCCESS;
}
XrResult XRAPI_CALL convert_time(XrInstance,XrTime time,LARGE_INTEGER* output) {
    if(!time_extension_enabled) return XR_ERROR_FUNCTION_UNSUPPORTED;
    LARGE_INTEGER frequency{}; QueryPerformanceFrequency(&frequency);
    output->QuadPart=time_qpc_origin+static_cast<std::int64_t>(time/1e9*frequency.QuadPart);
    return XR_SUCCESS;
}
XrResult XRAPI_CALL destroy_session(XrSession) { context.Reset(); device.Reset(); return XR_SUCCESS; }
XrResult XRAPI_CALL create_space(XrSession,const XrReferenceSpaceCreateInfo* info,XrSpace* output) {
    *output=reinterpret_cast<XrSpace>(static_cast<std::uintptr_t>(info->referenceSpaceType==XR_REFERENCE_SPACE_TYPE_VIEW?4:3)); return XR_SUCCESS;
}
XrResult XRAPI_CALL destroy_space(XrSpace) { return XR_SUCCESS; }
XrResult XRAPI_CALL configuration_views(XrInstance,XrSystemId,XrViewConfigurationType,uint32_t capacity,uint32_t* count,XrViewConfigurationView* output) {
    *count=2; if(!capacity) return XR_SUCCESS; if(capacity<2) return XR_ERROR_SIZE_INSUFFICIENT;
    for(int i=0;i<2;++i) { output[i].recommendedImageRectWidth=640; output[i].recommendedImageRectHeight=360;
        output[i].maxImageRectWidth=2048; output[i].maxImageRectHeight=2048;
        output[i].recommendedSwapchainSampleCount=output[i].maxSwapchainSampleCount=1; }
    return XR_SUCCESS;
}
XrResult XRAPI_CALL formats(XrSession,uint32_t capacity,uint32_t* count,int64_t* output) {
    *count=1; if(capacity) output[0]=DXGI_FORMAT_R8G8B8A8_UNORM_SRGB; return XR_SUCCESS;
}
XrResult XRAPI_CALL create_chain(XrSession,const XrSwapchainCreateInfo* info,XrSwapchain* output) {
    // Confirma as dimensões efetivas, para além da opção anunciada pelo programa.
    if(mode("native-resolution")&&(info->width!=640||info->height!=360)) return XR_ERROR_VALIDATION_FAILURE;
    auto* chain=new Chain;
    D3D11_TEXTURE2D_DESC description{}; description.Width=info->width; description.Height=info->height;
    description.MipLevels=description.ArraySize=description.SampleDesc.Count=1;
    description.Format=DXGI_FORMAT_R8G8B8A8_TYPELESS; description.Usage=D3D11_USAGE_DEFAULT;
    description.BindFlags=D3D11_BIND_RENDER_TARGET|D3D11_BIND_SHADER_RESOURCE;
    for(auto& image:chain->images) {
        if(FAILED(device->CreateTexture2D(&description,nullptr,&image))) { delete chain; return XR_ERROR_RUNTIME_FAILURE; }
        D3D11_RENDER_TARGET_VIEW_DESC view{}; view.Format=DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        view.ViewDimension=D3D11_RTV_DIMENSION_TEXTURE2D;
        ComPtr<ID3D11RenderTargetView> target;
        if(FAILED(device->CreateRenderTargetView(image.Get(),&view,&target))) { delete chain; return XR_ERROR_RUNTIME_FAILURE; }
        const float black[]={0,0,0,0}; context->ClearRenderTargetView(target.Get(),black);
    }
    chains.push_back(chain); *output=reinterpret_cast<XrSwapchain>(chain); return XR_SUCCESS;
}
XrResult XRAPI_CALL destroy_chain(XrSwapchain handle) {
    auto* chain=reinterpret_cast<Chain*>(handle);
    for(auto& entry:chains) if(entry==chain) entry=nullptr;
    delete chain; return XR_SUCCESS;
}
XrResult XRAPI_CALL chain_images(XrSwapchain handle,uint32_t capacity,uint32_t* count,XrSwapchainImageBaseHeader* output) {
    *count=3; if(!capacity) return XR_SUCCESS; if(capacity<3) return XR_ERROR_SIZE_INSUFFICIENT;
    auto* chain=reinterpret_cast<Chain*>(handle); auto* images=reinterpret_cast<XrSwapchainImageD3D11KHR*>(output);
    for(int i=0;i<3;++i) images[i].texture=chain->images[i].Get(); return XR_SUCCESS;
}
XrResult XRAPI_CALL poll(XrInstance,XrEventDataBuffer* output) {
    if(frame_number==2&&!reference_reported&&mode("recenter")) {
        reference_reported=true;
        XrEventDataReferenceSpaceChangePending event{XR_TYPE_EVENT_DATA_REFERENCE_SPACE_CHANGE_PENDING};
        event.session=session_handle; event.referenceSpaceType=XR_REFERENCE_SPACE_TYPE_LOCAL;
        event.changeTime=4*11111111LL; event.poseValid=XR_TRUE;
        event.poseInPreviousSpace.orientation.w=1; event.poseInPreviousSpace.position.x=1;
        std::memcpy(output,&event,sizeof(event)); return XR_SUCCESS;
    }
    if(frame_number==2&&!loss_reported&&mode("loss-session")) {
        loss_reported=true;
        XrEventDataSessionStateChanged event{XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED};
        event.session=session_handle; event.state=XR_SESSION_STATE_LOSS_PENDING;
        std::memcpy(output,&event,sizeof(event)); return XR_SUCCESS;
    }
    if(frame_number==2&&!loss_reported&&mode("loss-instance")) {
        loss_reported=true;
        XrEventDataInstanceLossPending event{XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING};
        std::memcpy(output,&event,sizeof(event)); return XR_SUCCESS;
    }
    if(event_number++) return XR_EVENT_UNAVAILABLE;
    XrEventDataSessionStateChanged event{XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED}; event.session=session_handle; event.state=XR_SESSION_STATE_READY;
    std::memcpy(output,&event,sizeof(event)); return XR_SUCCESS;
}
XrResult XRAPI_CALL begin_session(XrSession,const XrSessionBeginInfo*) { running=true; return XR_SUCCESS; }
XrResult XRAPI_CALL end_session(XrSession) { running=false; return XR_SUCCESS; }
XrResult XRAPI_CALL wait_frame(XrSession,const XrFrameWaitInfo*,XrFrameState* output) {
    if(!running||frame_open||wait_called) return XR_ERROR_CALL_ORDER_INVALID;
    Sleep(5); ++frame_number; wait_called=true;
    output->predictedDisplayTime=frame_number*11111111LL; output->predictedDisplayPeriod=11111111;
    should_render=!(mode("skip-frames")&&frame_number==1); output->shouldRender=should_render;
    valid_tracking=!(mode("skip-frames")&&frame_number==2); return XR_SUCCESS;
}
XrResult XRAPI_CALL begin_frame(XrSession,const XrFrameBeginInfo*) {
    if(!wait_called||frame_open) return XR_ERROR_CALL_ORDER_INVALID;
    wait_called=false; frame_open=true; return XR_SUCCESS;
}
XrPosef eye_pose(int eye) {
    XrPosef pose{}; const float yaw=mode("backward")?1.57079632679f:frame_number*0.002f;
    pose.orientation={0,std::sin(yaw),0,std::cos(yaw)}; pose.position={eye?0.032f:-0.032f,0,0}; return pose;
}
XrResult XRAPI_CALL locate_space(XrSpace handle,XrSpace base,XrTime time,XrSpaceLocation* output) {
    if(handle!=reinterpret_cast<XrSpace>(4)||base!=reinterpret_cast<XrSpace>(3)||time!=frame_number*11111111LL)
        return XR_ERROR_VALIDATION_FAILURE;
    output->locationFlags=valid_tracking?XR_SPACE_LOCATION_POSITION_VALID_BIT|XR_SPACE_LOCATION_ORIENTATION_VALID_BIT|
        XR_SPACE_LOCATION_POSITION_TRACKED_BIT|XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT:0;
    if(mode("head-position-only")) output->locationFlags&=~(XR_SPACE_LOCATION_ORIENTATION_VALID_BIT|XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT);
    if(mode("head-orientation-only")) output->locationFlags&=~(XR_SPACE_LOCATION_POSITION_VALID_BIT|XR_SPACE_LOCATION_POSITION_TRACKED_BIT);
    output->pose=eye_pose(0); output->pose.position.x=mode("recenter")&&frame_number>=4?1.0f:0.0f;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL create_hand(XrSession,const XrHandTrackerCreateInfoEXT* info,XrHandTrackerEXT* output) {
    if(!hand_extension_enabled) return XR_ERROR_FUNCTION_UNSUPPORTED;
    if(mode("hand-create-failure")&&info->hand==XR_HAND_RIGHT_EXT) return XR_ERROR_RUNTIME_FAILURE;
    ++trackers; *output=reinterpret_cast<XrHandTrackerEXT>(static_cast<std::uintptr_t>(info->hand==XR_HAND_LEFT_EXT?10:11)); return XR_SUCCESS;
}
XrResult XRAPI_CALL destroy_hand(XrHandTrackerEXT) { --trackers; return XR_SUCCESS; }
XrResult XRAPI_CALL locate_hand(XrHandTrackerEXT hand,const XrHandJointsLocateInfoEXT* info,XrHandJointLocationsEXT* output) {
    if(!frame_open||info->baseSpace!=reinterpret_cast<XrSpace>(3)||info->time!=frame_number*11111111LL||
       output->jointCount!=XR_HAND_JOINT_COUNT_EXT) return XR_ERROR_VALIDATION_FAILURE;
    if(mode("hand-locate-failure")) return XR_ERROR_RUNTIME_FAILURE;
    output->isActive=!mode("hands-inactive")&&!(mode("hands-lost")&&(frame_number==2||frame_number==3))&&
        !(mode("hands-disappear")&&frame_number>=4)&&!(mode("hands-appear")&&frame_number<4);
    if(!output->isActive) return XR_SUCCESS;
    for(std::uint32_t i=0;i<output->jointCount;++i) {
        auto& joint=output->jointLocations[i];
        joint.locationFlags=XR_SPACE_LOCATION_POSITION_VALID_BIT|XR_SPACE_LOCATION_ORIENTATION_VALID_BIT|
            XR_SPACE_LOCATION_POSITION_TRACKED_BIT|XR_SPACE_LOCATION_ORIENTATION_TRACKED_BIT;
        joint.pose.orientation.w=1;
        joint.pose.position={hand==reinterpret_cast<XrHandTrackerEXT>(10)?-0.2f:0.2f,0.0f,-0.4f};
        joint.radius=0.01f;
        if(mode("pencil-write")||mode("pencil-two-fingers")||mode("pencil-stop")||mode("pencil-lost")||mode("pencil-natural")||mode("pencil-chatter")) {
            const float x=(hand==reinterpret_cast<XrHandTrackerEXT>(10)?-0.2f:0.2f)+0.055f*std::sin(frame_number*0.10f);
            const float y=0.025f*std::sin(frame_number*0.16f);
            joint.pose.position={x,y,-0.4f}; joint.radius=0.006f;
            const auto set=[&](int id,float dx,float dy) { if(i==static_cast<std::uint32_t>(id)) joint.pose.position={x+dx,y+dy,-0.4f}; };
            set(1,0,-0.04f); set(5,-0.025f,0.02f);
            set(7,-0.025f,0.04f); set(8,-0.025f,0.075f); set(9,-0.025f,0.10f); set(10,-0.025f,0.122f);
            set(12,0,0.04f); set(13,0,0.07f); set(15,0,0.018f);
            set(17,0.024f,0.035f); set(18,0.024f,0.062f); set(20,0.024f,0.015f);
            set(22,0.045f,0.025f); set(23,0.045f,0.052f); set(25,0.045f,0.005f);
            if(mode("pencil-two-fingers")||(mode("pencil-stop")&&frame_number>=20)) set(15,0,0.12f);
            if(mode("pencil-lost")&&frame_number>=20) joint.locationFlags=0;
            if(mode("pencil-natural")) { set(5,-0.060f,0.055f); set(15,0.030f,0.090f); if(i==9) joint.locationFlags=0; }
            if(mode("pencil-chatter")&&frame_number%5==2&&i==20) joint.locationFlags=0;
        }
        if(mode("snap-gesture")||mode("snap-hold")||mode("snap-lost")) {
            const bool released=frame_number>=9&&!mode("snap-hold");
            if(i==XR_HAND_JOINT_WRIST_EXT) joint.pose.position.y=-0.04f;
            if(i==XR_HAND_JOINT_MIDDLE_PROXIMAL_EXT) joint.pose.position.y=0.04f;
            if(i==XR_HAND_JOINT_THUMB_TIP_EXT) joint.pose.position.y=0.10f;
            if(i==XR_HAND_JOINT_MIDDLE_TIP_EXT) joint.pose.position.y=released?0.025f:0.10f;
            if(i==XR_HAND_JOINT_MIDDLE_DISTAL_EXT) joint.pose.position.y=released?0.04f:0.08f;
            if(mode("snap-lost")&&frame_number==9) joint.locationFlags=0;
        }
        if(mode("hand-skeleton")||mode("hand-skeleton-partial")||mode("hand-skeleton-untracked")||mode("hand-skeleton-moved")) {
            const bool left=hand==reinterpret_cast<XrHandTrackerEXT>(10);
            const float sign=left?-1.0f:1.0f;
            // Pose sintética aberta, dedos separados e pulso abaixo da palma.
            joint.pose.position={sign*0.18f,-0.08f,-0.45f}; joint.radius=0.007f;
            if(i==XR_HAND_JOINT_WRIST_EXT) joint.pose.position.y-=0.06f;
            else if(i>=XR_HAND_JOINT_THUMB_METACARPAL_EXT&&i<=XR_HAND_JOINT_THUMB_TIP_EXT) {
                const float step=static_cast<float>(i-XR_HAND_JOINT_THUMB_METACARPAL_EXT);
                joint.pose.position.x-=sign*(0.035f+step*0.018f);
                joint.pose.position.y+=step*0.018f-0.03f;
            } else if(i>=XR_HAND_JOINT_INDEX_METACARPAL_EXT) {
                const auto finger=(i-XR_HAND_JOINT_INDEX_METACARPAL_EXT)/5,step=(i-XR_HAND_JOINT_INDEX_METACARPAL_EXT)%5;
                joint.pose.position.x+=sign*(-0.035f+static_cast<float>(finger)*0.024f);
                joint.pose.position.y+=static_cast<float>(step)*0.028f;
            }
            if(mode("hand-skeleton-moved")) joint.pose.position.x+=0.08f;
            if((mode("hand-skeleton-partial")||mode("hand-skeleton-untracked"))&&i>=XR_HAND_JOINT_INDEX_METACARPAL_EXT&&i<=XR_HAND_JOINT_INDEX_TIP_EXT) {
                if(mode("hand-skeleton-partial")) joint.locationFlags=0;
                else joint.locationFlags&=~XR_SPACE_LOCATION_POSITION_TRACKED_BIT;
            }
        }
    }
    return XR_SUCCESS;
}
XrResult XRAPI_CALL locate(XrSession,const XrViewLocateInfo*,XrViewState* state,uint32_t capacity,uint32_t* count,XrView* output) {
    *count=2; if(capacity<2) return XR_ERROR_SIZE_INSUFFICIENT;
    state->viewStateFlags=valid_tracking?XR_VIEW_STATE_ORIENTATION_VALID_BIT|XR_VIEW_STATE_POSITION_VALID_BIT:0;
    for(int i=0;i<2;++i) { output[i].pose=eye_pose(i); output[i].fov={-0.9f,0.9f,0.7f,-0.7f}; }
    return XR_SUCCESS;
}
XrResult XRAPI_CALL acquire(XrSwapchain handle,const XrSwapchainImageAcquireInfo*,uint32_t* output) {
    auto& chain=*reinterpret_cast<Chain*>(handle);
    if(!frame_open||!should_render||chain.acquired) return XR_ERROR_CALL_ORDER_INVALID;
    *output=chain.index; chain.acquired=true; chain.waited=chain.released=false; return XR_SUCCESS;
}
XrResult XRAPI_CALL wait_image(XrSwapchain handle,const XrSwapchainImageWaitInfo*) {
    auto& chain=*reinterpret_cast<Chain*>(handle); if(!chain.acquired) return XR_ERROR_CALL_ORDER_INVALID;
    chain.waited=true; return XR_SUCCESS;
}
XrResult XRAPI_CALL release(XrSwapchain handle,const XrSwapchainImageReleaseInfo*) {
    auto& chain=*reinterpret_cast<Chain*>(handle); if(!chain.acquired||!chain.waited) return XR_ERROR_CALL_ORDER_INVALID;
    chain.last=chain.index; chain.index=(chain.index+1)%3; chain.acquired=false; chain.released=true; return XR_SUCCESS;
}
std::uint64_t image_hash(Chain& chain) {
    D3D11_TEXTURE2D_DESC description{}; chain.images[chain.last]->GetDesc(&description);
    description.BindFlags=0; description.Usage=D3D11_USAGE_STAGING; description.CPUAccessFlags=D3D11_CPU_ACCESS_READ;
    ComPtr<ID3D11Texture2D> staging; if(FAILED(device->CreateTexture2D(&description,nullptr,&staging))) return false;
    context->CopyResource(staging.Get(),chain.images[chain.last].Get());
    D3D11_MAPPED_SUBRESOURCE pixels{}; if(FAILED(context->Map(staging.Get(),0,D3D11_MAP_READ,0,&pixels))) return false;
    std::size_t bright{}; std::uint64_t hash=14695981039346656037ULL;
    for(UINT y=0;y<description.Height;++y) {
        const auto* row=static_cast<const unsigned char*>(pixels.pData)+y*pixels.RowPitch;
        for(UINT x=0;x<description.Width;++x) {
            if(row[x*4]>10||row[x*4+1]>10||row[x*4+2]>10) ++bright;
            for(int c=0;c<3;++c) hash=(hash^row[x*4+c])*1099511628211ULL;
        }
    }
    context->Unmap(staging.Get(),0); return bright>20?hash:0;
}
XrResult XRAPI_CALL end_frame(XrSession,const XrFrameEndInfo* info) {
    if(!frame_open) return XR_ERROR_CALL_ORDER_INVALID;
    frame_open=false;
    if(!should_render||!valid_tracking) return info->layerCount?XR_ERROR_LAYER_INVALID:XR_SUCCESS;
    if(info->layerCount!=1||info->layers[0]->type!=XR_TYPE_COMPOSITION_LAYER_PROJECTION) return XR_ERROR_LAYER_INVALID;
    const auto* layer=reinterpret_cast<const XrCompositionLayerProjection*>(info->layers[0]);
    if(layer->viewCount!=2) return XR_ERROR_LAYER_INVALID;
    std::array<std::uint64_t,2> hashes{};
    for(int i=0;i<2;++i) {
        auto* chain=reinterpret_cast<Chain*>(layer->views[i].subImage.swapchain);
        hashes[i]=image_hash(*chain);
        if(!chain->released||chain->acquired||std::abs(layer->views[i].pose.position.x-eye_pose(i).position.x)>0.0001f||!hashes[i]) return XR_ERROR_LAYER_INVALID;
    }
    if(hashes[0]==hashes[1]) return XR_ERROR_LAYER_INVALID;
    return XR_SUCCESS;
}
XrResult XRAPI_CALL result_string(XrInstance,XrResult result,char* output) { std::strcpy(output,result==XR_SUCCESS?"XR_SUCCESS":"MOCK_ERROR"); return XR_SUCCESS; }
XrResult XRAPI_CALL proc(XrInstance,const char* name,PFN_xrVoidFunction* output) {
    #define ENTRY(api,implementation) if(std::strcmp(name,#api)==0) { *output=reinterpret_cast<PFN_xrVoidFunction>(implementation); return XR_SUCCESS; }
    ENTRY(xrGetInstanceProcAddr,proc) ENTRY(xrEnumerateInstanceExtensionProperties,extensions)
    ENTRY(xrCreateInstance,create_instance) ENTRY(xrDestroyInstance,destroy_instance) ENTRY(xrGetInstanceProperties,instance_properties)
    ENTRY(xrGetSystem,get_system) ENTRY(xrGetSystemProperties,system_properties) ENTRY(xrGetD3D11GraphicsRequirementsKHR,requirements)
    ENTRY(xrCreateSession,create_session) ENTRY(xrDestroySession,destroy_session) ENTRY(xrCreateReferenceSpace,create_space) ENTRY(xrDestroySpace,destroy_space)
    ENTRY(xrLocateSpace,locate_space) ENTRY(xrCreateHandTrackerEXT,create_hand)
    ENTRY(xrConvertTimeToWin32PerformanceCounterKHR,convert_time)
    ENTRY(xrDestroyHandTrackerEXT,destroy_hand) ENTRY(xrLocateHandJointsEXT,locate_hand)
    ENTRY(xrEnumerateViewConfigurationViews,configuration_views) ENTRY(xrEnumerateSwapchainFormats,formats)
    ENTRY(xrCreateSwapchain,create_chain) ENTRY(xrDestroySwapchain,destroy_chain) ENTRY(xrEnumerateSwapchainImages,chain_images)
    ENTRY(xrPollEvent,poll) ENTRY(xrBeginSession,begin_session) ENTRY(xrEndSession,end_session)
    ENTRY(xrWaitFrame,wait_frame) ENTRY(xrBeginFrame,begin_frame) ENTRY(xrLocateViews,locate)
    ENTRY(xrAcquireSwapchainImage,acquire) ENTRY(xrWaitSwapchainImage,wait_image) ENTRY(xrReleaseSwapchainImage,release) ENTRY(xrEndFrame,end_frame)
    ENTRY(xrResultToString,result_string)
    #undef ENTRY
    *output=nullptr; return XR_ERROR_FUNCTION_UNSUPPORTED;
}
}
extern "C" __declspec(dllexport) XrResult XRAPI_CALL xrNegotiateLoaderRuntimeInterface(const XrNegotiateLoaderInfo*,XrNegotiateRuntimeRequest* output) {
    output->runtimeInterfaceVersion=XR_CURRENT_LOADER_RUNTIME_VERSION;
    output->runtimeApiVersion=XR_MAKE_VERSION(1,0,0); output->getInstanceProcAddr=proc; return XR_SUCCESS;
}
