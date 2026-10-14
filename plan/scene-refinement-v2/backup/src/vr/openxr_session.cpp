#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <openxr/openxr.h>
#include <openxr/openxr_platform.h>
#include "openxr_session.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <stdexcept>
#include <vector>

namespace fantasynt::vr {
using Microsoft::WRL::ComPtr;
namespace {
void checked(XrResult result,const char* operation) {
    if(XR_FAILED(result)) throw std::runtime_error(std::string(operation)+" failed (OpenXR "+std::to_string(result)+
        "). Connect Quest 3 in Virtual Desktop and select VDXR in the Streamer settings.");
}
void dx_checked(HRESULT result,const char* operation) {
    if(FAILED(result)) throw std::runtime_error(std::string(operation)+" failed (D3D11 "+std::to_string(result)+")");
}
struct Instance {
    XrInstance handle{XR_NULL_HANDLE};
    Instance() {
        std::uint32_t count{};
        checked(xrEnumerateInstanceExtensionProperties(nullptr,0,&count,nullptr),"Enumerate OpenXR extensions");
        std::vector<XrExtensionProperties> extensions(count,{XR_TYPE_EXTENSION_PROPERTIES});
        checked(xrEnumerateInstanceExtensionProperties(nullptr,count,&count,extensions.data()),"Read OpenXR extensions");
        if(std::none_of(extensions.begin(),extensions.end(),[](const auto& e) { return std::strcmp(e.extensionName,XR_KHR_D3D11_ENABLE_EXTENSION_NAME)==0; }))
            throw std::runtime_error("OpenXR runtime does not support Direct3D 11");
        const char* enabled[]={XR_KHR_D3D11_ENABLE_EXTENSION_NAME};
        XrInstanceCreateInfo info{XR_TYPE_INSTANCE_CREATE_INFO};
        std::strcpy(info.applicationInfo.applicationName,"Fantasynt");
        std::strcpy(info.applicationInfo.engineName,"Fantasynt C++");
        info.applicationInfo.applicationVersion=1;
        info.applicationInfo.apiVersion=XR_MAKE_VERSION(1,0,0);
        info.enabledExtensionCount=1; info.enabledExtensionNames=enabled;
        checked(xrCreateInstance(&info,&handle),"Create OpenXR instance");
    }
    ~Instance() { if(handle!=XR_NULL_HANDLE) xrDestroyInstance(handle); }
    Instance(const Instance&)=delete;
    Instance& operator=(const Instance&)=delete;
};
XrSystemId system(XrInstance instance) {
    XrSystemGetInfo info{XR_TYPE_SYSTEM_GET_INFO}; info.formFactor=XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrSystemId id{}; checked(xrGetSystem(instance,&info,&id),"Find connected VR headset"); return id;
}
std::string runtime_name(XrInstance instance) {
    XrInstanceProperties properties{XR_TYPE_INSTANCE_PROPERTIES};
    checked(xrGetInstanceProperties(instance,&properties),"Read OpenXR runtime");
    return properties.runtimeName;
}
}
std::string probe_openxr() {
    Instance instance;
    XrSystemGetInfo info{XR_TYPE_SYSTEM_GET_INFO}; info.formFactor=XR_FORM_FACTOR_HEAD_MOUNTED_DISPLAY;
    XrSystemId id{}; const auto result=xrGetSystem(instance.handle,&info,&id);
    nlohmann::json report{{"type","xr_probe"},{"runtime",runtime_name(instance.handle)},
        {"d3d11",true},{"headset_available",XR_SUCCEEDED(result)},{"system_result",result}};
    if(XR_SUCCEEDED(result)) {
        XrSystemProperties properties{XR_TYPE_SYSTEM_PROPERTIES};
        checked(xrGetSystemProperties(instance.handle,id,&properties),"Read headset properties");
        report["headset"]=properties.systemName;
    }
    return report.dump();
}
struct OpenXRSession::Impl {
    Instance instance;
    XrSystemId system_id{};
    XrSession session{XR_NULL_HANDLE};
    XrSpace space{XR_NULL_HANDLE};
    ComPtr<ID3D11Device> device;
    ComPtr<ID3D11DeviceContext> context;
    struct Eye {
        XrSwapchain swapchain{XR_NULL_HANDLE};
        std::vector<XrSwapchainImageD3D11KHR> images;
        std::uint32_t index{};
        bool acquired{},waited{};
    };
    std::array<Eye,2> eyes;
    std::array<XrView,2> views{{{XR_TYPE_VIEW},{XR_TYPE_VIEW}}};
    std::uint16_t width{},height{};
    DXGI_FORMAT format{};
    XrTime display_time{};
    bool running{},frame_open{},quit{},has_views{};
    std::string runtime;
    explicit Impl(float scale) {
        if(!std::isfinite(scale)||scale<0.4f||scale>1.0f) throw std::runtime_error("XR resolution scale must be 0.4..1.0");
        system_id=system(instance.handle);
        runtime=runtime_name(instance.handle);
        try {
            PFN_xrGetD3D11GraphicsRequirementsKHR requirements_function{};
            checked(xrGetInstanceProcAddr(instance.handle,"xrGetD3D11GraphicsRequirementsKHR",
                reinterpret_cast<PFN_xrVoidFunction*>(&requirements_function)),"Get OpenXR D3D11 requirements");
            XrGraphicsRequirementsD3D11KHR requirements{XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR};
            checked(requirements_function(instance.handle,system_id,&requirements),"Read OpenXR D3D11 requirements");
            ComPtr<IDXGIFactory1> factory; dx_checked(CreateDXGIFactory1(IID_PPV_ARGS(&factory)),"Create DXGI factory");
            ComPtr<IDXGIAdapter1> adapter;
            for(UINT i=0;;++i) {
                ComPtr<IDXGIAdapter1> candidate;
                const auto result=factory->EnumAdapters1(i,&candidate);
                if(result==DXGI_ERROR_NOT_FOUND) break;
                dx_checked(result,"Enumerate GPU");
                DXGI_ADAPTER_DESC1 description{}; dx_checked(candidate->GetDesc1(&description),"Read GPU");
                if(description.AdapterLuid.LowPart==requirements.adapterLuid.LowPart&&description.AdapterLuid.HighPart==requirements.adapterLuid.HighPart) {
                    adapter=candidate; break;
                }
            }
            if(!adapter) throw std::runtime_error("GPU requested by OpenXR runtime was not found");
            const D3D_FEATURE_LEVEL levels[]={D3D_FEATURE_LEVEL_11_1,D3D_FEATURE_LEVEL_11_0};
            D3D_FEATURE_LEVEL actual{};
            dx_checked(D3D11CreateDevice(adapter.Get(),D3D_DRIVER_TYPE_UNKNOWN,nullptr,D3D11_CREATE_DEVICE_BGRA_SUPPORT,
                levels,2,D3D11_SDK_VERSION,&device,&actual,&context),"Create VR D3D11 device");
            if(actual<requirements.minFeatureLevel) throw std::runtime_error("GPU feature level is insufficient for OpenXR");
            XrGraphicsBindingD3D11KHR binding{XR_TYPE_GRAPHICS_BINDING_D3D11_KHR}; binding.device=device.Get();
            XrSessionCreateInfo session_info{XR_TYPE_SESSION_CREATE_INFO}; session_info.next=&binding; session_info.systemId=system_id;
            checked(xrCreateSession(instance.handle,&session_info,&session),"Create VR session");
            XrReferenceSpaceCreateInfo reference{XR_TYPE_REFERENCE_SPACE_CREATE_INFO};
            reference.referenceSpaceType=XR_REFERENCE_SPACE_TYPE_LOCAL; reference.poseInReferenceSpace.orientation.w=1;
            checked(xrCreateReferenceSpace(session,&reference,&space),"Create VR reference space");
            std::uint32_t count{};
            checked(xrEnumerateViewConfigurationViews(instance.handle,system_id,XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,0,&count,nullptr),"Count stereo views");
            if(count!=2) throw std::runtime_error("OpenXR runtime must provide two stereo views");
            std::array<XrViewConfigurationView,2> configuration{{{XR_TYPE_VIEW_CONFIGURATION_VIEW},{XR_TYPE_VIEW_CONFIGURATION_VIEW}}};
            checked(xrEnumerateViewConfigurationViews(instance.handle,system_id,XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO,2,&count,configuration.data()),"Read stereo dimensions");
            const auto w=static_cast<std::uint32_t>(std::max(configuration[0].recommendedImageRectWidth,configuration[1].recommendedImageRectWidth)*scale);
            const auto h=static_cast<std::uint32_t>(std::max(configuration[0].recommendedImageRectHeight,configuration[1].recommendedImageRectHeight)*scale);
            if(w<64||h<64||w>8192||h>8192) throw std::runtime_error("Unsupported VR render dimensions");
            for(const auto& view:configuration)
                if(w>view.maxImageRectWidth||h>view.maxImageRectHeight) throw std::runtime_error("VR dimensions exceed runtime limits");
            width=static_cast<std::uint16_t>(w); height=static_cast<std::uint16_t>(h);
            checked(xrEnumerateSwapchainFormats(session,0,&count,nullptr),"Count VR image formats");
            std::vector<std::int64_t> formats(count);
            checked(xrEnumerateSwapchainFormats(session,count,&count,formats.data()),"Read VR image formats");
            for(auto preferred:{DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,DXGI_FORMAT_B8G8R8A8_UNORM_SRGB,DXGI_FORMAT_R8G8B8A8_UNORM,DXGI_FORMAT_B8G8R8A8_UNORM})
                if(std::find(formats.begin(),formats.end(),preferred)!=formats.end()) { format=preferred; break; }
            if(format==DXGI_FORMAT_UNKNOWN) throw std::runtime_error("No compatible OpenXR RGBA8/BGRA8 image format");
            for(auto& eye:eyes) {
                XrSwapchainCreateInfo image_info{XR_TYPE_SWAPCHAIN_CREATE_INFO};
                image_info.usageFlags=XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT|XR_SWAPCHAIN_USAGE_SAMPLED_BIT;
                image_info.format=format; image_info.sampleCount=1; image_info.width=width; image_info.height=height;
                image_info.faceCount=1; image_info.arraySize=1; image_info.mipCount=1;
                checked(xrCreateSwapchain(session,&image_info,&eye.swapchain),"Create VR images");
                checked(xrEnumerateSwapchainImages(eye.swapchain,0,&count,nullptr),"Count VR images");
                eye.images.resize(count,{XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR});
                checked(xrEnumerateSwapchainImages(eye.swapchain,count,&count,reinterpret_cast<XrSwapchainImageBaseHeader*>(eye.images.data())),"Read VR images");
            }
        } catch(...) { cleanup(); throw; }
    }
    ~Impl() { cleanup(); }
    void cleanup() noexcept {
        for(auto& eye:eyes) if(eye.acquired&&eye.waited) {
            XrSwapchainImageReleaseInfo release{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
            xrReleaseSwapchainImage(eye.swapchain,&release); eye.acquired=false;
        }
        if(frame_open) {
            XrFrameEndInfo end{XR_TYPE_FRAME_END_INFO}; end.displayTime=display_time; end.environmentBlendMode=XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
            xrEndFrame(session,&end); frame_open=false;
        }
        if(space!=XR_NULL_HANDLE) { xrDestroySpace(space); space=XR_NULL_HANDLE; }
        for(auto& eye:eyes) if(eye.swapchain!=XR_NULL_HANDLE) { xrDestroySwapchain(eye.swapchain); eye.swapchain=XR_NULL_HANDLE; }
        if(session!=XR_NULL_HANDLE) { xrDestroySession(session); session=XR_NULL_HANDLE; }
    }
    bool poll() {
        XrEventDataBuffer event{XR_TYPE_EVENT_DATA_BUFFER};
        for(auto result=xrPollEvent(instance.handle,&event);result!=XR_EVENT_UNAVAILABLE;result=xrPollEvent(instance.handle,&event)) {
            checked(result,"Poll VR events");
            if(event.type==XR_TYPE_EVENT_DATA_INSTANCE_LOSS_PENDING)
                throw std::runtime_error("OpenXR runtime lost its instance. Reconnect Virtual Desktop and restart Fantasynt.");
            if(event.type==XR_TYPE_EVENT_DATA_SESSION_STATE_CHANGED) {
                const auto& state=*reinterpret_cast<const XrEventDataSessionStateChanged*>(&event);
                if(state.state==XR_SESSION_STATE_READY&&!running) {
                    XrSessionBeginInfo begin{XR_TYPE_SESSION_BEGIN_INFO}; begin.primaryViewConfigurationType=XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
                    checked(xrBeginSession(session,&begin),"Begin VR session"); running=true;
                } else if(state.state==XR_SESSION_STATE_STOPPING&&running) {
                    checked(xrEndSession(session),"End VR session"); running=false;
                } else if(state.state==XR_SESSION_STATE_LOSS_PENDING)
                    throw std::runtime_error("OpenXR runtime lost its session. Reconnect Virtual Desktop and restart Fantasynt.");
                else if(state.state==XR_SESSION_STATE_EXITING) quit=true;
            }
            event={XR_TYPE_EVENT_DATA_BUFFER};
        }
        return !quit;
    }
    void finish() {
        if(!frame_open) return;
        context->Flush();
        std::array<XrCompositionLayerProjectionView,2> projections{{{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW},{XR_TYPE_COMPOSITION_LAYER_PROJECTION_VIEW}}};
        for(std::size_t i=0;i<eyes.size();++i) {
            auto& eye=eyes[i];
            if(eye.acquired) {
                XrSwapchainImageReleaseInfo release{XR_TYPE_SWAPCHAIN_IMAGE_RELEASE_INFO};
                checked(xrReleaseSwapchainImage(eye.swapchain,&release),"Release VR image"); eye.acquired=eye.waited=false;
            }
            projections[i].pose=views[i].pose; projections[i].fov=views[i].fov;
            projections[i].subImage.swapchain=eye.swapchain;
            projections[i].subImage.imageRect.extent={width,height};
        }
        XrCompositionLayerProjection layer{XR_TYPE_COMPOSITION_LAYER_PROJECTION};
        layer.space=space; layer.viewCount=2; layer.views=projections.data();
        const auto* layer_pointer=reinterpret_cast<const XrCompositionLayerBaseHeader*>(&layer);
        XrFrameEndInfo end{XR_TYPE_FRAME_END_INFO}; end.displayTime=display_time; end.environmentBlendMode=XR_ENVIRONMENT_BLEND_MODE_OPAQUE;
        end.layerCount=has_views?1:0; end.layers=has_views?&layer_pointer:nullptr;
        checked(xrEndFrame(session,&end),"Submit VR frame"); frame_open=false;
    }
    std::optional<std::array<EyeView,2>> begin() {
        if(!running) return std::nullopt;
        XrFrameWaitInfo wait{XR_TYPE_FRAME_WAIT_INFO}; XrFrameState frame{XR_TYPE_FRAME_STATE};
        checked(xrWaitFrame(session,&wait,&frame),"Wait for VR frame"); display_time=frame.predictedDisplayTime;
        XrFrameBeginInfo begin{XR_TYPE_FRAME_BEGIN_INFO}; checked(xrBeginFrame(session,&begin),"Begin VR frame");
        frame_open=true; has_views=false;
        if(!frame.shouldRender) { finish(); return std::nullopt; }
        XrViewLocateInfo location{XR_TYPE_VIEW_LOCATE_INFO}; location.viewConfigurationType=XR_VIEW_CONFIGURATION_TYPE_PRIMARY_STEREO;
        location.displayTime=display_time; location.space=space;
        XrViewState state{XR_TYPE_VIEW_STATE}; std::uint32_t count{};
        checked(xrLocateViews(session,&location,&state,2,&count,views.data()),"Locate tracked VR eyes");
        const auto valid=XR_VIEW_STATE_POSITION_VALID_BIT|XR_VIEW_STATE_ORIENTATION_VALID_BIT;
        if(count!=2||(state.viewStateFlags&valid)!=valid) { finish(); return std::nullopt; }
        std::array<EyeView,2> result;
        // Origem comum entre os olhos; a orientação média conserva a inclinação própria de cada lente.
        const auto& left=views[0].pose; const auto& right=views[1].pose;
        const auto& a=left.orientation; const auto& b=right.orientation;
        const float sign=a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w<0?-1.0f:1.0f;
        Quaternion orientation{a.x+sign*b.x,a.y+sign*b.y,a.z+sign*b.z,a.w+sign*b.w};
        const float norm=std::sqrt(orientation.x*orientation.x+orientation.y*orientation.y+orientation.z*orientation.z+orientation.w*orientation.w);
        if(norm<0.00001f) throw std::runtime_error("Invalid cyclopean XR orientation");
        orientation.x/=norm; orientation.y/=norm; orientation.z/=norm; orientation.w/=norm;
        const auto center=make_eye_matrices({orientation,{(left.position.x+right.position.x)*0.5f,
            (left.position.y+right.position.y)*0.5f,(left.position.z+right.position.z)*0.5f}},
            {-0.7f,0.7f,0.7f,-0.7f});
        std::array<float,16> head{};
        for(int col=0;col<3;++col) for(int row=0;row<3;++row) head[col*4+row]=center.view[row*4+col];
        head[12]=center.eye_world.x; head[13]=center.eye_world.y; head[14]=center.eye_world.z; head[15]=1;
        for(std::size_t i=0;i<eyes.size();++i) {
            auto& eye=eyes[i];
            XrSwapchainImageAcquireInfo acquire{XR_TYPE_SWAPCHAIN_IMAGE_ACQUIRE_INFO};
            checked(xrAcquireSwapchainImage(eye.swapchain,&acquire,&eye.index),"Acquire VR image"); eye.acquired=true;
            XrSwapchainImageWaitInfo image_wait{XR_TYPE_SWAPCHAIN_IMAGE_WAIT_INFO}; image_wait.timeout=XR_INFINITE_DURATION;
            const auto image_result=xrWaitSwapchainImage(eye.swapchain,&image_wait);
            checked(image_result,"Wait for VR image");
            if(image_result==XR_TIMEOUT_EXPIRED) throw std::runtime_error("OpenXR image wait timed out");
            eye.waited=true;
            if(eye.index>=eye.images.size()) throw std::runtime_error("OpenXR returned an invalid image index");
            const auto& pose=views[i].pose; const auto& fov=views[i].fov;
            result[i].matrices=make_eye_matrices({{pose.orientation.x,pose.orientation.y,pose.orientation.z,pose.orientation.w},
                {pose.position.x,pose.position.y,pose.position.z}},{fov.angleLeft,fov.angleRight,fov.angleUp,fov.angleDown});
            result[i].head_world=head;
            result[i].texture=eye.images[eye.index].texture;
            result[i].width=width; result[i].height=height;
            result[i].bgra=format==DXGI_FORMAT_B8G8R8A8_UNORM||format==DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            result[i].srgb=format==DXGI_FORMAT_R8G8B8A8_UNORM_SRGB||format==DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        }
        has_views=true; return result;
    }
};
OpenXRSession::OpenXRSession(float scale):impl_(std::make_unique<Impl>(scale)) {}
OpenXRSession::~OpenXRSession()=default;
void* OpenXRSession::graphics_device() const { return impl_->device.Get(); }
std::string OpenXRSession::description() const { return impl_->runtime+" "+std::to_string(impl_->width)+"x"+std::to_string(impl_->height)+" per eye"; }
bool OpenXRSession::poll_events() { return impl_->poll(); }
std::optional<std::array<EyeView,2>> OpenXRSession::begin_frame() { return impl_->begin(); }
void OpenXRSession::end_frame() { impl_->finish(); }
}
