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
XrInstance instance_handle=reinterpret_cast<XrInstance>(1);
XrSession session_handle=reinterpret_cast<XrSession>(2);
bool mode(const char* name) { const char* value=std::getenv("FANTASYNT_MOCK_MODE"); return value&&std::strcmp(value,name)==0; }
XrResult XRAPI_CALL extensions(const char*,uint32_t capacity,uint32_t* count,XrExtensionProperties* output) {
    *count=1; if(!capacity) return XR_SUCCESS; if(capacity<1) return XR_ERROR_SIZE_INSUFFICIENT;
    std::strcpy(output[0].extensionName,XR_KHR_D3D11_ENABLE_EXTENSION_NAME); output[0].extensionVersion=9; return XR_SUCCESS;
}
XrResult XRAPI_CALL create_instance(const XrInstanceCreateInfo*,XrInstance* output) { *output=instance_handle; return XR_SUCCESS; }
XrResult XRAPI_CALL destroy_instance(XrInstance) { return XR_SUCCESS; }
XrResult XRAPI_CALL instance_properties(XrInstance,XrInstanceProperties* output) {
    std::strcpy(output->runtimeName,"FantasyntMock"); output->runtimeVersion=XR_MAKE_VERSION(1,0,0); return XR_SUCCESS;
}
XrResult XRAPI_CALL get_system(XrInstance,const XrSystemGetInfo*,XrSystemId* output) {
    if(mode("no-headset")) return XR_ERROR_FORM_FACTOR_UNAVAILABLE;
    *output=1; return XR_SUCCESS;
}
XrResult XRAPI_CALL system_properties(XrInstance,XrSystemId,XrSystemProperties* output) {
    output->systemId=1; std::strcpy(output->systemName,"Simulated stereo D3D11 headset"); return XR_SUCCESS;
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
    event_number=frame_number=0; loss_reported=false; return XR_SUCCESS;
}
XrResult XRAPI_CALL destroy_session(XrSession) { context.Reset(); device.Reset(); return XR_SUCCESS; }
XrResult XRAPI_CALL create_space(XrSession,const XrReferenceSpaceCreateInfo*,XrSpace* output) { *output=reinterpret_cast<XrSpace>(3); return XR_SUCCESS; }
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
