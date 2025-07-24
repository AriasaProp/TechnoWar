#include "glad_egl.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// EGL Core Functions
EGLBoolean (*eglChooseConfig)(EGLDisplay, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) = NULL;
EGLBoolean (*eglCopyBuffers)(EGLDisplay,EGLSurface, EGLNativePixmapType target) = NULL;
EGLContext (*eglCreateContext)(EGLDisplay, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) = NULL;
EGLSurface (*eglCreatePbufferSurface)(EGLDisplay, EGLConfig config, const EGLint *attrib_list) = NULL;
EGLSurface (*eglCreatePixmapSurface)(EGLDisplay, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) = NULL;
EGLSurface (*eglCreateWindowSurface)(EGLDisplay, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) = NULL;
EGLBoolean (*eglDestroyContext)(EGLDisplay, EGLContext) = NULL;
EGLBoolean (*eglDestroySurface)(EGLDisplay,EGLSurface) = NULL;
EGLBoolean (*eglGetConfigAttrib)(EGLDisplay, EGLConfig config, EGLint attribute, EGLint *value) = NULL;
EGLBoolean (*eglGetConfigs)(EGLDisplay, EGLConfig *configs, EGLint config_size, EGLint *num_config) = NULL;
EGLDisplay (*eglGetCurrentDisplay)(void) = NULL;
EGLSurface (*eglGetCurrentSurface)(EGLint readdraw) = NULL;
EGLDisplay (*eglGetDisplay)(EGLNativeDisplayType display_id) = NULL;
EGLint (*eglGetError)(void) = NULL;
EGLBoolean (*eglInitialize)(EGLDisplay, EGLint *major, EGLint *minor) = NULL;
EGLBoolean (*eglMakeCurrent)(EGLDisplay, EGLSurface draw, EGLSurface read, EGLContext) = NULL;
EGLBoolean (*eglQueryContext)(EGLDisplay, EGLContext, EGLint attribute, EGLint *value) = NULL;
const char *(*eglQueryString)(EGLDisplay, EGLint name) = NULL;
EGLBoolean (*eglQuerySurface)(EGLDisplay,EGLSurface, EGLint attribute, EGLint *value) = NULL;
EGLBoolean (*eglSwapBuffers)(EGLDisplay,EGLSurface) = NULL;
EGLBoolean (*eglTerminate)(EGLDisplay) = NULL;
EGLBoolean (*eglWaitGL)(void) = NULL;
EGLBoolean (*eglWaitNative)(EGLint engine) = NULL;

// EGL 1.1 Functions
EGLBoolean (*eglBindTexImage)(EGLDisplay,EGLSurface, EGLint buffer) = NULL;
EGLBoolean (*eglReleaseTexImage)(EGLDisplay,EGLSurface, EGLint buffer) = NULL;
EGLBoolean (*eglSurfaceAttrib)(EGLDisplay,EGLSurface, EGLint attribute, EGLint value) = NULL;
EGLBoolean (*eglSwapInterval)(EGLDisplay, EGLint interval) = NULL;

// EGL 1.2 Functions
EGLBoolean (*eglBindAPI)(EGLenum api) = NULL;
EGLenum (*eglQueryAPI)(void) = NULL;
EGLSurface (*eglCreatePbufferFromClientBuffer)(EGLDisplay, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) = NULL;
EGLBoolean (*eglReleaseThread)(void) = NULL;
EGLBoolean (*eglWaitClient)(void) = NULL;

// EGL 1.4 Functions
EGLContext (*eglGetCurrentContext)(void) = NULL;

PFNEGLSETBLOBCACHEFUNCSANDROIDPROC glad_eglSetBlobCacheFuncsANDROID = NULL;
PFNEGLCREATENATIVECLIENTBUFFERANDROIDPROC glad_eglCreateNativeClientBufferANDROID = NULL;
PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC glad_eglGetCompositorTimingSupportedANDROID = NULL;
PFNEGLGETCOMPOSITORTIMINGANDROIDPROC glad_eglGetCompositorTimingANDROID = NULL;
PFNEGLGETNEXTFRAMEIDANDROIDPROC glad_eglGetNextFrameIdANDROID = NULL;
PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC glad_eglGetFrameTimestampSupportedANDROID = NULL;
PFNEGLGETFRAMETIMESTAMPSANDROIDPROC glad_eglGetFrameTimestampsANDROID = NULL;
PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC glad_eglGetNativeClientBufferANDROID = NULL;
PFNEGLDUPNATIVEFENCEFDANDROIDPROC glad_eglDupNativeFenceFDANDROID = NULL;
PFNEGLPRESENTATIONTIMEANDROIDPROC glad_eglPresentationTimeANDROID = NULL;
PFNEGLQUERYSURFACEPOINTERANGLEPROC glad_eglQuerySurfacePointerANGLE = NULL;
PFNEGLGETMSCRATEANGLEPROC glad_eglGetMscRateANGLE = NULL;
PFNEGLCLIENTSIGNALSYNCEXTPROC glad_eglClientSignalSyncEXT = NULL;
PFNEGLCOMPOSITORSETCONTEXTLISTEXTPROC glad_eglCompositorSetContextListEXT = NULL;
PFNEGLCOMPOSITORSETCONTEXTATTRIBUTESEXTPROC glad_eglCompositorSetContextAttributesEXT = NULL;
PFNEGLCOMPOSITORSETWINDOWLISTEXTPROC glad_eglCompositorSetWindowListEXT = NULL;
PFNEGLCOMPOSITORSETWINDOWATTRIBUTESEXTPROC glad_eglCompositorSetWindowAttributesEXT = NULL;
PFNEGLCOMPOSITORBINDTEXWINDOWEXTPROC glad_eglCompositorBindTexWindowEXT = NULL;
PFNEGLCOMPOSITORSETSIZEEXTPROC glad_eglCompositorSetSizeEXT = NULL;
PFNEGLCOMPOSITORSWAPPOLICYEXTPROC glad_eglCompositorSwapPolicyEXT = NULL;
PFNEGLQUERYDEVICEATTRIBEXTPROC glad_eglQueryDeviceAttribEXT = NULL;
PFNEGLQUERYDEVICESTRINGEXTPROC glad_eglQueryDeviceStringEXT = NULL;
PFNEGLQUERYDEVICESEXTPROC glad_eglQueryDevicesEXT = NULL;
PFNEGLQUERYDISPLAYATTRIBEXTPROC glad_eglQueryDisplayAttribEXT = NULL;
PFNEGLQUERYDEVICEBINARYEXTPROC glad_eglQueryDeviceBinaryEXT = NULL;
PFNEGLDESTROYDISPLAYEXTPROC glad_eglDestroyDisplayEXT = NULL;
PFNEGLQUERYDMABUFFORMATSEXTPROC glad_eglQueryDmaBufFormatsEXT = NULL;
PFNEGLQUERYDMABUFMODIFIERSEXTPROC glad_eglQueryDmaBufModifiersEXT = NULL;
PFNEGLGETOUTPUTLAYERSEXTPROC glad_eglGetOutputLayersEXT = NULL;
PFNEGLGETOUTPUTPORTSEXTPROC glad_eglGetOutputPortsEXT = NULL;
PFNEGLOUTPUTLAYERATTRIBEXTPROC glad_eglOutputLayerAttribEXT = NULL;
PFNEGLQUERYOUTPUTLAYERATTRIBEXTPROC glad_eglQueryOutputLayerAttribEXT = NULL;
PFNEGLQUERYOUTPUTLAYERSTRINGEXTPROC glad_eglQueryOutputLayerStringEXT = NULL;
PFNEGLOUTPUTPORTATTRIBEXTPROC glad_eglOutputPortAttribEXT = NULL;
PFNEGLQUERYOUTPUTPORTATTRIBEXTPROC glad_eglQueryOutputPortAttribEXT = NULL;
PFNEGLQUERYOUTPUTPORTSTRINGEXTPROC glad_eglQueryOutputPortStringEXT = NULL;
PFNEGLGETPLATFORMDISPLAYEXTPROC glad_eglGetPlatformDisplayEXT = NULL;
PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC glad_eglCreatePlatformWindowSurfaceEXT = NULL;
PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC glad_eglCreatePlatformPixmapSurfaceEXT = NULL;
PFNEGLSTREAMCONSUMEROUTPUTEXTPROC glad_eglStreamConsumerOutputEXT = NULL;
PFNEGLQUERYSUPPORTEDCOMPRESSIONRATESEXTPROC glad_eglQuerySupportedCompressionRatesEXT = NULL;
PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC glad_eglSwapBuffersWithDamageEXT = NULL;
PFNEGLUNSIGNALSYNCEXTPROC glad_eglUnsignalSyncEXT = NULL;
PFNEGLCREATEPIXMAPSURFACEHIPROC glad_eglCreatePixmapSurfaceHI = NULL;
PFNEGLCREATESYNC64KHRPROC glad_eglCreateSync64KHR = NULL;
PFNEGLDEBUGMESSAGECONTROLKHRPROC glad_eglDebugMessageControlKHR = NULL;
PFNEGLQUERYDEBUGKHRPROC glad_eglQueryDebugKHR = NULL;
PFNEGLLABELOBJECTKHRPROC glad_eglLabelObjectKHR = NULL;
PFNEGLQUERYDISPLAYATTRIBKHRPROC glad_eglQueryDisplayAttribKHR = NULL;
PFNEGLCREATESYNCKHRPROC glad_eglCreateSyncKHR = NULL;
PFNEGLDESTROYSYNCKHRPROC glad_eglDestroySyncKHR = NULL;
PFNEGLCLIENTWAITSYNCKHRPROC glad_eglClientWaitSyncKHR = NULL;
PFNEGLGETSYNCATTRIBKHRPROC glad_eglGetSyncAttribKHR = NULL;
PFNEGLCREATEIMAGEKHRPROC glad_eglCreateImageKHR = NULL;
PFNEGLDESTROYIMAGEKHRPROC glad_eglDestroyImageKHR = NULL;
PFNEGLLOCKSURFACEKHRPROC glad_eglLockSurfaceKHR = NULL;
PFNEGLUNLOCKSURFACEKHRPROC glad_eglUnlockSurfaceKHR = NULL;
PFNEGLQUERYSURFACE64KHRPROC glad_eglQuerySurface64KHR = NULL;
PFNEGLSETDAMAGEREGIONKHRPROC glad_eglSetDamageRegionKHR = NULL;
PFNEGLSIGNALSYNCKHRPROC glad_eglSignalSyncKHR = NULL;
PFNEGLCREATESTREAMKHRPROC glad_eglCreateStreamKHR = NULL;
PFNEGLDESTROYSTREAMKHRPROC glad_eglDestroyStreamKHR = NULL;
PFNEGLSTREAMATTRIBKHRPROC glad_eglStreamAttribKHR = NULL;
PFNEGLQUERYSTREAMKHRPROC glad_eglQueryStreamKHR = NULL;
PFNEGLQUERYSTREAMU64KHRPROC glad_eglQueryStreamu64KHR = NULL;
PFNEGLCREATESTREAMATTRIBKHRPROC glad_eglCreateStreamAttribKHR = NULL;
PFNEGLSETSTREAMATTRIBKHRPROC glad_eglSetStreamAttribKHR = NULL;
PFNEGLQUERYSTREAMATTRIBKHRPROC glad_eglQueryStreamAttribKHR = NULL;
PFNEGLSTREAMCONSUMERACQUIREATTRIBKHRPROC glad_eglStreamConsumerAcquireAttribKHR = NULL;
PFNEGLSTREAMCONSUMERRELEASEATTRIBKHRPROC glad_eglStreamConsumerReleaseAttribKHR = NULL;
PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC glad_eglStreamConsumerGLTextureExternalKHR = NULL;
PFNEGLSTREAMCONSUMERACQUIREKHRPROC glad_eglStreamConsumerAcquireKHR = NULL;
PFNEGLSTREAMCONSUMERRELEASEKHRPROC glad_eglStreamConsumerReleaseKHR = NULL;
PFNEGLGETSTREAMFILEDESCRIPTORKHRPROC glad_eglGetStreamFileDescriptorKHR = NULL;
PFNEGLCREATESTREAMFROMFILEDESCRIPTORKHRPROC glad_eglCreateStreamFromFileDescriptorKHR = NULL;
PFNEGLQUERYSTREAMTIMEKHRPROC glad_eglQueryStreamTimeKHR = NULL;
PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC glad_eglCreateStreamProducerSurfaceKHR = NULL;
PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC glad_eglSwapBuffersWithDamageKHR = NULL;
PFNEGLWAITSYNCKHRPROC glad_eglWaitSyncKHR = NULL;
PFNEGLCREATEDRMIMAGEMESAPROC glad_eglCreateDRMImageMESA = NULL;
PFNEGLEXPORTDRMIMAGEMESAPROC glad_eglExportDRMImageMESA = NULL;
PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC glad_eglExportDMABUFImageQueryMESA = NULL;
PFNEGLEXPORTDMABUFIMAGEMESAPROC glad_eglExportDMABUFImageMESA = NULL;
PFNEGLGETDISPLAYDRIVERCONFIGPROC glad_eglGetDisplayDriverConfig = NULL;
PFNEGLGETDISPLAYDRIVERNAMEPROC glad_eglGetDisplayDriverName = NULL;
PFNEGLSWAPBUFFERSREGIONNOKPROC glad_eglSwapBuffersRegionNOK = NULL;
PFNEGLSWAPBUFFERSREGION2NOKPROC glad_eglSwapBuffersRegion2NOK = NULL;
PFNEGLQUERYNATIVEDISPLAYNVPROC glad_eglQueryNativeDisplayNV = NULL;
PFNEGLQUERYNATIVEWINDOWNVPROC glad_eglQueryNativeWindowNV = NULL;
PFNEGLQUERYNATIVEPIXMAPNVPROC glad_eglQueryNativePixmapNV = NULL;
PFNEGLPOSTSUBBUFFERNVPROC glad_eglPostSubBufferNV = NULL;
PFNEGLSTREAMIMAGECONSUMERCONNECTNVPROC glad_eglStreamImageConsumerConnectNV = NULL;
PFNEGLQUERYSTREAMCONSUMEREVENTNVPROC glad_eglQueryStreamConsumerEventNV = NULL;
PFNEGLSTREAMACQUIREIMAGENVPROC glad_eglStreamAcquireImageNV = NULL;
PFNEGLSTREAMRELEASEIMAGENVPROC glad_eglStreamReleaseImageNV = NULL;
PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALATTRIBSNVPROC glad_eglStreamConsumerGLTextureExternalAttribsNV = NULL;
PFNEGLSTREAMFLUSHNVPROC glad_eglStreamFlushNV = NULL;
PFNEGLQUERYDISPLAYATTRIBNVPROC glad_eglQueryDisplayAttribNV = NULL;
PFNEGLSETSTREAMMETADATANVPROC glad_eglSetStreamMetadataNV = NULL;
PFNEGLQUERYSTREAMMETADATANVPROC glad_eglQueryStreamMetadataNV = NULL;
PFNEGLRESETSTREAMNVPROC glad_eglResetStreamNV = NULL;
PFNEGLCREATESTREAMSYNCNVPROC glad_eglCreateStreamSyncNV = NULL;
PFNEGLCREATEFENCESYNCNVPROC glad_eglCreateFenceSyncNV = NULL;
PFNEGLDESTROYSYNCNVPROC glad_eglDestroySyncNV = NULL;
PFNEGLFENCENVPROC glad_eglFenceNV = NULL;
PFNEGLCLIENTWAITSYNCNVPROC glad_eglClientWaitSyncNV = NULL;
PFNEGLSIGNALSYNCNVPROC glad_eglSignalSyncNV = NULL;
PFNEGLGETSYNCATTRIBNVPROC glad_eglGetSyncAttribNV = NULL;
PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC glad_eglGetSystemTimeFrequencyNV = NULL;
PFNEGLGETSYSTEMTIMENVPROC glad_eglGetSystemTimeNV = NULL;
PFNEGLBINDWAYLANDDISPLAYWLPROC glad_eglBindWaylandDisplayWL = NULL;
PFNEGLUNBINDWAYLANDDISPLAYWLPROC glad_eglUnbindWaylandDisplayWL = NULL;
PFNEGLQUERYWAYLANDBUFFERWLPROC glad_eglQueryWaylandBufferWL = NULL;
PFNEGLCREATEWAYLANDBUFFERFROMIMAGEWLPROC glad_eglCreateWaylandBufferFromImageWL = NULL;
static void load_EGL_ANDROID_blob_cache(GLADloadproc load) {
  glad_eglSetBlobCacheFuncsANDROID = (PFNEGLSETBLOBCACHEFUNCSANDROIDPROC)load("eglSetBlobCacheFuncsANDROID");
}
static void load_EGL_ANDROID_create_native_client_buffer(GLADloadproc load) {
  glad_eglCreateNativeClientBufferANDROID = (PFNEGLCREATENATIVECLIENTBUFFERANDROIDPROC)load("eglCreateNativeClientBufferANDROID");
}
static void load_EGL_ANDROID_get_frame_timestamps(GLADloadproc load) {
  glad_eglGetCompositorTimingSupportedANDROID = (PFNEGLGETCOMPOSITORTIMINGSUPPORTEDANDROIDPROC)load("eglGetCompositorTimingSupportedANDROID");
  glad_eglGetCompositorTimingANDROID = (PFNEGLGETCOMPOSITORTIMINGANDROIDPROC)load("eglGetCompositorTimingANDROID");
  glad_eglGetNextFrameIdANDROID = (PFNEGLGETNEXTFRAMEIDANDROIDPROC)load("eglGetNextFrameIdANDROID");
  glad_eglGetFrameTimestampSupportedANDROID = (PFNEGLGETFRAMETIMESTAMPSUPPORTEDANDROIDPROC)load("eglGetFrameTimestampSupportedANDROID");
  glad_eglGetFrameTimestampsANDROID = (PFNEGLGETFRAMETIMESTAMPSANDROIDPROC)load("eglGetFrameTimestampsANDROID");
}
static void load_EGL_ANDROID_get_native_client_buffer(GLADloadproc load) {
  glad_eglGetNativeClientBufferANDROID = (PFNEGLGETNATIVECLIENTBUFFERANDROIDPROC)load("eglGetNativeClientBufferANDROID");
}
static void load_EGL_ANDROID_native_fence_sync(GLADloadproc load) {
  glad_eglDupNativeFenceFDANDROID = (PFNEGLDUPNATIVEFENCEFDANDROIDPROC)load("eglDupNativeFenceFDANDROID");
}
static void load_EGL_ANDROID_presentation_time(GLADloadproc load) {
  glad_eglPresentationTimeANDROID = (PFNEGLPRESENTATIONTIMEANDROIDPROC)load("eglPresentationTimeANDROID");
}
static void load_EGL_ANGLE_query_surface_pointer(GLADloadproc load) {
  glad_eglQuerySurfacePointerANGLE = (PFNEGLQUERYSURFACEPOINTERANGLEPROC)load("eglQuerySurfacePointerANGLE");
}
static void load_EGL_ANGLE_sync_control_rate(GLADloadproc load) {
  glad_eglGetMscRateANGLE = (PFNEGLGETMSCRATEANGLEPROC)load("eglGetMscRateANGLE");
}
static void load_EGL_EXT_client_sync(GLADloadproc load) {
  glad_eglClientSignalSyncEXT = (PFNEGLCLIENTSIGNALSYNCEXTPROC)load("eglClientSignalSyncEXT");
}
static void load_EGL_EXT_compositor(GLADloadproc load) {
  glad_eglCompositorSetContextListEXT = (PFNEGLCOMPOSITORSETCONTEXTLISTEXTPROC)load("eglCompositorSetContextListEXT");
  glad_eglCompositorSetContextAttributesEXT = (PFNEGLCOMPOSITORSETCONTEXTATTRIBUTESEXTPROC)load("eglCompositorSetContextAttributesEXT");
  glad_eglCompositorSetWindowListEXT = (PFNEGLCOMPOSITORSETWINDOWLISTEXTPROC)load("eglCompositorSetWindowListEXT");
  glad_eglCompositorSetWindowAttributesEXT = (PFNEGLCOMPOSITORSETWINDOWATTRIBUTESEXTPROC)load("eglCompositorSetWindowAttributesEXT");
  glad_eglCompositorBindTexWindowEXT = (PFNEGLCOMPOSITORBINDTEXWINDOWEXTPROC)load("eglCompositorBindTexWindowEXT");
  glad_eglCompositorSetSizeEXT = (PFNEGLCOMPOSITORSETSIZEEXTPROC)load("eglCompositorSetSizeEXT");
  glad_eglCompositorSwapPolicyEXT = (PFNEGLCOMPOSITORSWAPPOLICYEXTPROC)load("eglCompositorSwapPolicyEXT");
}
static void load_EGL_EXT_device_base(GLADloadproc load) {
  glad_eglQueryDeviceAttribEXT = (PFNEGLQUERYDEVICEATTRIBEXTPROC)load("eglQueryDeviceAttribEXT");
  glad_eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)load("eglQueryDeviceStringEXT");
  glad_eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)load("eglQueryDevicesEXT");
  glad_eglQueryDisplayAttribEXT = (PFNEGLQUERYDISPLAYATTRIBEXTPROC)load("eglQueryDisplayAttribEXT");
}
static void load_EGL_EXT_device_enumeration(GLADloadproc load) {
  glad_eglQueryDevicesEXT = (PFNEGLQUERYDEVICESEXTPROC)load("eglQueryDevicesEXT");
}
static void load_EGL_EXT_device_persistent_id(GLADloadproc load) {
  glad_eglQueryDeviceBinaryEXT = (PFNEGLQUERYDEVICEBINARYEXTPROC)load("eglQueryDeviceBinaryEXT");
}
static void load_EGL_EXT_device_query(GLADloadproc load) {
  glad_eglQueryDeviceAttribEXT = (PFNEGLQUERYDEVICEATTRIBEXTPROC)load("eglQueryDeviceAttribEXT");
  glad_eglQueryDeviceStringEXT = (PFNEGLQUERYDEVICESTRINGEXTPROC)load("eglQueryDeviceStringEXT");
  glad_eglQueryDisplayAttribEXT = (PFNEGLQUERYDISPLAYATTRIBEXTPROC)load("eglQueryDisplayAttribEXT");
}
static void load_EGL_EXT_display_alloc(GLADloadproc load) {
  glad_eglDestroyDisplayEXT = (PFNEGLDESTROYDISPLAYEXTPROC)load("eglDestroyDisplayEXT");
}
static void load_EGL_EXT_image_dma_buf_import_modifiers(GLADloadproc load) {
  glad_eglQueryDmaBufFormatsEXT = (PFNEGLQUERYDMABUFFORMATSEXTPROC)load("eglQueryDmaBufFormatsEXT");
  glad_eglQueryDmaBufModifiersEXT = (PFNEGLQUERYDMABUFMODIFIERSEXTPROC)load("eglQueryDmaBufModifiersEXT");
}
static void load_EGL_EXT_output_base(GLADloadproc load) {
  glad_eglGetOutputLayersEXT = (PFNEGLGETOUTPUTLAYERSEXTPROC)load("eglGetOutputLayersEXT");
  glad_eglGetOutputPortsEXT = (PFNEGLGETOUTPUTPORTSEXTPROC)load("eglGetOutputPortsEXT");
  glad_eglOutputLayerAttribEXT = (PFNEGLOUTPUTLAYERATTRIBEXTPROC)load("eglOutputLayerAttribEXT");
  glad_eglQueryOutputLayerAttribEXT = (PFNEGLQUERYOUTPUTLAYERATTRIBEXTPROC)load("eglQueryOutputLayerAttribEXT");
  glad_eglQueryOutputLayerStringEXT = (PFNEGLQUERYOUTPUTLAYERSTRINGEXTPROC)load("eglQueryOutputLayerStringEXT");
  glad_eglOutputPortAttribEXT = (PFNEGLOUTPUTPORTATTRIBEXTPROC)load("eglOutputPortAttribEXT");
  glad_eglQueryOutputPortAttribEXT = (PFNEGLQUERYOUTPUTPORTATTRIBEXTPROC)load("eglQueryOutputPortAttribEXT");
  glad_eglQueryOutputPortStringEXT = (PFNEGLQUERYOUTPUTPORTSTRINGEXTPROC)load("eglQueryOutputPortStringEXT");
}
static void load_EGL_EXT_platform_base(GLADloadproc load) {
  glad_eglGetPlatformDisplayEXT = (PFNEGLGETPLATFORMDISPLAYEXTPROC)load("eglGetPlatformDisplayEXT");
  glad_eglCreatePlatformWindowSurfaceEXT = (PFNEGLCREATEPLATFORMWINDOWSURFACEEXTPROC)load("eglCreatePlatformWindowSurfaceEXT");
  glad_eglCreatePlatformPixmapSurfaceEXT = (PFNEGLCREATEPLATFORMPIXMAPSURFACEEXTPROC)load("eglCreatePlatformPixmapSurfaceEXT");
}
static void load_EGL_EXT_stream_consumer_egloutput(GLADloadproc load) {
  glad_eglStreamConsumerOutputEXT = (PFNEGLSTREAMCONSUMEROUTPUTEXTPROC)load("eglStreamConsumerOutputEXT");
}
static void load_EGL_EXT_surface_compression(GLADloadproc load) {
  glad_eglQuerySupportedCompressionRatesEXT = (PFNEGLQUERYSUPPORTEDCOMPRESSIONRATESEXTPROC)load("eglQuerySupportedCompressionRatesEXT");
}
static void load_EGL_EXT_swap_buffers_with_damage(GLADloadproc load) {
  glad_eglSwapBuffersWithDamageEXT = (PFNEGLSWAPBUFFERSWITHDAMAGEEXTPROC)load("eglSwapBuffersWithDamageEXT");
}
static void load_EGL_EXT_sync_reuse(GLADloadproc load) {
  glad_eglUnsignalSyncEXT = (PFNEGLUNSIGNALSYNCEXTPROC)load("eglUnsignalSyncEXT");
}
static void load_EGL_HI_clientpixmap(GLADloadproc load) {
  glad_eglCreatePixmapSurfaceHI = (PFNEGLCREATEPIXMAPSURFACEHIPROC)load("eglCreatePixmapSurfaceHI");
}
static void load_EGL_KHR_cl_event2(GLADloadproc load) {
  glad_eglCreateSync64KHR = (PFNEGLCREATESYNC64KHRPROC)load("eglCreateSync64KHR");
}
static void load_EGL_KHR_debug(GLADloadproc load) {
  glad_eglDebugMessageControlKHR = (PFNEGLDEBUGMESSAGECONTROLKHRPROC)load("eglDebugMessageControlKHR");
  glad_eglQueryDebugKHR = (PFNEGLQUERYDEBUGKHRPROC)load("eglQueryDebugKHR");
  glad_eglLabelObjectKHR = (PFNEGLLABELOBJECTKHRPROC)load("eglLabelObjectKHR");
}
static void load_EGL_KHR_display_reference(GLADloadproc load) {
  glad_eglQueryDisplayAttribKHR = (PFNEGLQUERYDISPLAYATTRIBKHRPROC)load("eglQueryDisplayAttribKHR");
}
static void load_EGL_KHR_fence_sync(GLADloadproc load) {
  glad_eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC)load("eglCreateSyncKHR");
  glad_eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC)load("eglDestroySyncKHR");
  glad_eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC)load("eglClientWaitSyncKHR");
  glad_eglGetSyncAttribKHR = (PFNEGLGETSYNCATTRIBKHRPROC)load("eglGetSyncAttribKHR");
}
static void load_EGL_KHR_image(GLADloadproc load) {
  glad_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)load("eglCreateImageKHR");
  glad_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)load("eglDestroyImageKHR");
}
static void load_EGL_KHR_image_base(GLADloadproc load) {
  glad_eglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)load("eglCreateImageKHR");
  glad_eglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)load("eglDestroyImageKHR");
}
static void load_EGL_KHR_lock_surface(GLADloadproc load) {
  glad_eglLockSurfaceKHR = (PFNEGLLOCKSURFACEKHRPROC)load("eglLockSurfaceKHR");
  glad_eglUnlockSurfaceKHR = (PFNEGLUNLOCKSURFACEKHRPROC)load("eglUnlockSurfaceKHR");
}
static void load_EGL_KHR_lock_surface3(GLADloadproc load) {
  glad_eglLockSurfaceKHR = (PFNEGLLOCKSURFACEKHRPROC)load("eglLockSurfaceKHR");
  glad_eglUnlockSurfaceKHR = (PFNEGLUNLOCKSURFACEKHRPROC)load("eglUnlockSurfaceKHR");
  glad_eglQuerySurface64KHR = (PFNEGLQUERYSURFACE64KHRPROC)load("eglQuerySurface64KHR");
}
static void load_EGL_KHR_partial_update(GLADloadproc load) {
  glad_eglSetDamageRegionKHR = (PFNEGLSETDAMAGEREGIONKHRPROC)load("eglSetDamageRegionKHR");
}
static void load_EGL_KHR_reusable_sync(GLADloadproc load) {
  glad_eglCreateSyncKHR = (PFNEGLCREATESYNCKHRPROC)load("eglCreateSyncKHR");
  glad_eglDestroySyncKHR = (PFNEGLDESTROYSYNCKHRPROC)load("eglDestroySyncKHR");
  glad_eglClientWaitSyncKHR = (PFNEGLCLIENTWAITSYNCKHRPROC)load("eglClientWaitSyncKHR");
  glad_eglSignalSyncKHR = (PFNEGLSIGNALSYNCKHRPROC)load("eglSignalSyncKHR");
  glad_eglGetSyncAttribKHR = (PFNEGLGETSYNCATTRIBKHRPROC)load("eglGetSyncAttribKHR");
}
static void load_EGL_KHR_stream(GLADloadproc load) {
  glad_eglCreateStreamKHR = (PFNEGLCREATESTREAMKHRPROC)load("eglCreateStreamKHR");
  glad_eglDestroyStreamKHR = (PFNEGLDESTROYSTREAMKHRPROC)load("eglDestroyStreamKHR");
  glad_eglStreamAttribKHR = (PFNEGLSTREAMATTRIBKHRPROC)load("eglStreamAttribKHR");
  glad_eglQueryStreamKHR = (PFNEGLQUERYSTREAMKHRPROC)load("eglQueryStreamKHR");
  glad_eglQueryStreamu64KHR = (PFNEGLQUERYSTREAMU64KHRPROC)load("eglQueryStreamu64KHR");
}
static void load_EGL_KHR_stream_attrib(GLADloadproc load) {
  glad_eglCreateStreamAttribKHR = (PFNEGLCREATESTREAMATTRIBKHRPROC)load("eglCreateStreamAttribKHR");
  glad_eglSetStreamAttribKHR = (PFNEGLSETSTREAMATTRIBKHRPROC)load("eglSetStreamAttribKHR");
  glad_eglQueryStreamAttribKHR = (PFNEGLQUERYSTREAMATTRIBKHRPROC)load("eglQueryStreamAttribKHR");
  glad_eglStreamConsumerAcquireAttribKHR = (PFNEGLSTREAMCONSUMERACQUIREATTRIBKHRPROC)load("eglStreamConsumerAcquireAttribKHR");
  glad_eglStreamConsumerReleaseAttribKHR = (PFNEGLSTREAMCONSUMERRELEASEATTRIBKHRPROC)load("eglStreamConsumerReleaseAttribKHR");
}
static void load_EGL_KHR_stream_consumer_gltexture(GLADloadproc load) {
  glad_eglStreamConsumerGLTextureExternalKHR = (PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALKHRPROC)load("eglStreamConsumerGLTextureExternalKHR");
  glad_eglStreamConsumerAcquireKHR = (PFNEGLSTREAMCONSUMERACQUIREKHRPROC)load("eglStreamConsumerAcquireKHR");
  glad_eglStreamConsumerReleaseKHR = (PFNEGLSTREAMCONSUMERRELEASEKHRPROC)load("eglStreamConsumerReleaseKHR");
}
static void load_EGL_KHR_stream_cross_process_fd(GLADloadproc load) {
  glad_eglGetStreamFileDescriptorKHR = (PFNEGLGETSTREAMFILEDESCRIPTORKHRPROC)load("eglGetStreamFileDescriptorKHR");
  glad_eglCreateStreamFromFileDescriptorKHR = (PFNEGLCREATESTREAMFROMFILEDESCRIPTORKHRPROC)load("eglCreateStreamFromFileDescriptorKHR");
}
static void load_EGL_KHR_stream_fifo(GLADloadproc load) {
  glad_eglQueryStreamTimeKHR = (PFNEGLQUERYSTREAMTIMEKHRPROC)load("eglQueryStreamTimeKHR");
}
static void load_EGL_KHR_stream_producer_eglsurface(GLADloadproc load) {
  glad_eglCreateStreamProducerSurfaceKHR = (PFNEGLCREATESTREAMPRODUCERSURFACEKHRPROC)load("eglCreateStreamProducerSurfaceKHR");
}
static void load_EGL_KHR_swap_buffers_with_damage(GLADloadproc load) {
  glad_eglSwapBuffersWithDamageKHR = (PFNEGLSWAPBUFFERSWITHDAMAGEKHRPROC)load("eglSwapBuffersWithDamageKHR");
}
static void load_EGL_KHR_wait_sync(GLADloadproc load) {
  glad_eglWaitSyncKHR = (PFNEGLWAITSYNCKHRPROC)load("eglWaitSyncKHR");
}
static void load_EGL_MESA_drm_image(GLADloadproc load) {
  glad_eglCreateDRMImageMESA = (PFNEGLCREATEDRMIMAGEMESAPROC)load("eglCreateDRMImageMESA");
  glad_eglExportDRMImageMESA = (PFNEGLEXPORTDRMIMAGEMESAPROC)load("eglExportDRMImageMESA");
}
static void load_EGL_MESA_image_dma_buf_export(GLADloadproc load) {
  glad_eglExportDMABUFImageQueryMESA = (PFNEGLEXPORTDMABUFIMAGEQUERYMESAPROC)load("eglExportDMABUFImageQueryMESA");
  glad_eglExportDMABUFImageMESA = (PFNEGLEXPORTDMABUFIMAGEMESAPROC)load("eglExportDMABUFImageMESA");
}
static void load_EGL_MESA_query_driver(GLADloadproc load) {
  glad_eglGetDisplayDriverConfig = (PFNEGLGETDISPLAYDRIVERCONFIGPROC)load("eglGetDisplayDriverConfig");
  glad_eglGetDisplayDriverName = (PFNEGLGETDISPLAYDRIVERNAMEPROC)load("eglGetDisplayDriverName");
}
static void load_EGL_NOK_swap_region(GLADloadproc load) {
  glad_eglSwapBuffersRegionNOK = (PFNEGLSWAPBUFFERSREGIONNOKPROC)load("eglSwapBuffersRegionNOK");
}
static void load_EGL_NOK_swap_region2(GLADloadproc load) {
  glad_eglSwapBuffersRegion2NOK = (PFNEGLSWAPBUFFERSREGION2NOKPROC)load("eglSwapBuffersRegion2NOK");
}
static void load_EGL_NV_native_query(GLADloadproc load) {
  glad_eglQueryNativeDisplayNV = (PFNEGLQUERYNATIVEDISPLAYNVPROC)load("eglQueryNativeDisplayNV");
  glad_eglQueryNativeWindowNV = (PFNEGLQUERYNATIVEWINDOWNVPROC)load("eglQueryNativeWindowNV");
  glad_eglQueryNativePixmapNV = (PFNEGLQUERYNATIVEPIXMAPNVPROC)load("eglQueryNativePixmapNV");
}
static void load_EGL_NV_post_sub_buffer(GLADloadproc load) {
  glad_eglPostSubBufferNV = (PFNEGLPOSTSUBBUFFERNVPROC)load("eglPostSubBufferNV");
}
static void load_EGL_NV_stream_consumer_eglimage(GLADloadproc load) {
  glad_eglStreamImageConsumerConnectNV = (PFNEGLSTREAMIMAGECONSUMERCONNECTNVPROC)load("eglStreamImageConsumerConnectNV");
  glad_eglQueryStreamConsumerEventNV = (PFNEGLQUERYSTREAMCONSUMEREVENTNVPROC)load("eglQueryStreamConsumerEventNV");
  glad_eglStreamAcquireImageNV = (PFNEGLSTREAMACQUIREIMAGENVPROC)load("eglStreamAcquireImageNV");
  glad_eglStreamReleaseImageNV = (PFNEGLSTREAMRELEASEIMAGENVPROC)load("eglStreamReleaseImageNV");
}
static void load_EGL_NV_stream_consumer_gltexture_yuv(GLADloadproc load) {
  glad_eglStreamConsumerGLTextureExternalAttribsNV = (PFNEGLSTREAMCONSUMERGLTEXTUREEXTERNALATTRIBSNVPROC)load("eglStreamConsumerGLTextureExternalAttribsNV");
}
static void load_EGL_NV_stream_flush(GLADloadproc load) {
  glad_eglStreamFlushNV = (PFNEGLSTREAMFLUSHNVPROC)load("eglStreamFlushNV");
}
static void load_EGL_NV_stream_metadata(GLADloadproc load) {
  glad_eglQueryDisplayAttribNV = (PFNEGLQUERYDISPLAYATTRIBNVPROC)load("eglQueryDisplayAttribNV");
  glad_eglSetStreamMetadataNV = (PFNEGLSETSTREAMMETADATANVPROC)load("eglSetStreamMetadataNV");
  glad_eglQueryStreamMetadataNV = (PFNEGLQUERYSTREAMMETADATANVPROC)load("eglQueryStreamMetadataNV");
}
static void load_EGL_NV_stream_reset(GLADloadproc load) {
  glad_eglResetStreamNV = (PFNEGLRESETSTREAMNVPROC)load("eglResetStreamNV");
}
static void load_EGL_NV_stream_sync(GLADloadproc load) {
  glad_eglCreateStreamSyncNV = (PFNEGLCREATESTREAMSYNCNVPROC)load("eglCreateStreamSyncNV");
}
static void load_EGL_NV_sync(GLADloadproc load) {
  glad_eglCreateFenceSyncNV = (PFNEGLCREATEFENCESYNCNVPROC)load("eglCreateFenceSyncNV");
  glad_eglDestroySyncNV = (PFNEGLDESTROYSYNCNVPROC)load("eglDestroySyncNV");
  glad_eglFenceNV = (PFNEGLFENCENVPROC)load("eglFenceNV");
  glad_eglClientWaitSyncNV = (PFNEGLCLIENTWAITSYNCNVPROC)load("eglClientWaitSyncNV");
  glad_eglSignalSyncNV = (PFNEGLSIGNALSYNCNVPROC)load("eglSignalSyncNV");
  glad_eglGetSyncAttribNV = (PFNEGLGETSYNCATTRIBNVPROC)load("eglGetSyncAttribNV");
}
static void load_EGL_NV_system_time(GLADloadproc load) {
  glad_eglGetSystemTimeFrequencyNV = (PFNEGLGETSYSTEMTIMEFREQUENCYNVPROC)load("eglGetSystemTimeFrequencyNV");
  glad_eglGetSystemTimeNV = (PFNEGLGETSYSTEMTIMENVPROC)load("eglGetSystemTimeNV");
}
static void load_EGL_WL_bind_wayland_display(GLADloadproc load) {
  glad_eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWLPROC)load("eglBindWaylandDisplayWL");
  glad_eglUnbindWaylandDisplayWL = (PFNEGLUNBINDWAYLANDDISPLAYWLPROC)load("eglUnbindWaylandDisplayWL");
  glad_eglQueryWaylandBufferWL = (PFNEGLQUERYWAYLANDBUFFERWLPROC)load("eglQueryWaylandBufferWL");
}
static void load_EGL_WL_create_wayland_buffer_from_image(GLADloadproc load) {
  glad_eglCreateWaylandBufferFromImageWL = (PFNEGLCREATEWAYLANDBUFFERFROMIMAGEWLPROC)load("eglCreateWaylandBufferFromImageWL");
}
static int find_extensionsEGL(void) {
  return 1;
}

void gladLoadEGL(GLADloadproc loader) {

  // EGL Core Functions
  eglChooseConfig = (EGLBoolean (*)(EGLDisplay, const EGLint *, EGLConfig *, EGLint, EGLint *))loader("eglChooseConfig");
  eglCopyBuffers = (EGLBoolean (*)(EGLDisplay, EGLSurface, EGLNativePixmapType))loader("eglCopyBuffers");
  eglCreateContext = (EGLContext (*)(EGLDisplay, EGLConfig, EGLContext, const EGLint *))loader("eglCreateContext");
  eglCreatePbufferSurface = (EGLSurface (*)(EGLDisplay, EGLConfig, const EGLint *))loader("eglCreatePbufferSurface");
  eglCreatePixmapSurface = (EGLSurface (*)(EGLDisplay, EGLConfig, EGLNativePixmapType, const EGLint *))loader("eglCreatePixmapSurface");
  eglCreateWindowSurface = (EGLSurface (*)(EGLDisplay, EGLConfig, EGLNativeWindowType, const EGLint *))loader("eglCreateWindowSurface");
  eglDestroyContext = (EGLBoolean (*)(EGLDisplay, EGLContext))loader("eglDestroyContext");
  eglDestroySurface = (EGLBoolean (*)(EGLDisplay, EGLSurface))loader("eglDestroySurface");
  eglGetConfigAttrib = (EGLBoolean (*)(EGLDisplay, EGLConfig, EGLint, EGLint *))loader("eglGetConfigAttrib");
  eglGetConfigs = (EGLBoolean (*)(EGLDisplay, EGLConfig *, EGLint, EGLint *))loader("eglGetConfigs");
  eglGetCurrentDisplay = (EGLDisplay (*)(void))loader("eglGetCurrentDisplay");
  eglGetCurrentSurface = (EGLSurface (*)(EGLint))loader("eglGetCurrentSurface");
  eglGetDisplay = (EGLDisplay (*)(EGLNativeDisplayType))loader("eglGetDisplay");
  eglGetError = (EGLint (*)(void))loader("eglGetError");
  eglInitialize = (EGLBoolean (*)(EGLDisplay, EGLint *, EGLint *))loader("eglInitialize");
  eglMakeCurrent = (EGLBoolean (*)(EGLDisplay, EGLSurface, EGLSurface, EGLContext))loader("eglMakeCurrent");
  eglQueryContext = (EGLBoolean (*)(EGLDisplay, EGLContext, EGLint, EGLint *))loader("eglQueryContext");
  eglQueryString = (const char *(*)(EGLDisplay, EGLint))loader("eglQueryString");
  eglQuerySurface = (EGLBoolean (*)(EGLDisplay, EGLSurface, EGLint, EGLint *))loader("eglQuerySurface");
  eglSwapBuffers = (EGLBoolean (*)(EGLDisplay, EGLSurface))loader("eglSwapBuffers");
  eglTerminate = (EGLBoolean (*)(EGLDisplay))loader("eglTerminate");
  eglWaitGL = (EGLBoolean (*)(void))loader("eglWaitGL");
  eglWaitNative = (EGLBoolean (*)(EGLint))loader("eglWaitNative");

  // EGL 1.1 Functions
  eglBindTexImage = (EGLBoolean (*)(EGLDisplay, EGLSurface, EGLint))loader("eglBindTexImage");
  eglReleaseTexImage = (EGLBoolean (*)(EGLDisplay, EGLSurface, EGLint))loader("eglReleaseTexImage");
  eglSurfaceAttrib = (EGLBoolean (*)(EGLDisplay, EGLSurface, EGLint, EGLint))loader("eglSurfaceAttrib");
  eglSwapInterval = (EGLBoolean (*)(EGLDisplay, EGLint))loader("eglSwapInterval");

  // EGL 1.2 Functions
  eglBindAPI = (EGLBoolean (*)(EGLenum))loader("eglBindAPI");
  eglQueryAPI = (EGLenum (*)(void))loader("eglQueryAPI");
  eglCreatePbufferFromClientBuffer = (EGLSurface (*)(EGLDisplay, EGLenum, EGLClientBuffer, EGLConfig, const EGLint *))loader("eglCreatePbufferFromClientBuffer");
  eglReleaseThread = (EGLBoolean (*)(void))loader("eglReleaseThread");
  eglWaitClient = (EGLBoolean (*)(void))loader("eglWaitClient");

  // EGL 1.4 Functions
  eglGetCurrentContext = (EGLContext (*)(void))loader("eglGetCurrentContext");
  /*
{
  if (!find_extensionsEGL())
    return 0;
  load_EGL_ANDROID_blob_cache(load);
  load_EGL_ANDROID_create_native_client_buffer(load);
  load_EGL_ANDROID_get_frame_timestamps(load);
  load_EGL_ANDROID_get_native_client_buffer(load);
  load_EGL_ANDROID_native_fence_sync(load);
  load_EGL_ANDROID_presentation_time(load);
  load_EGL_ANGLE_query_surface_pointer(load);
  load_EGL_ANGLE_sync_control_rate(load);
  load_EGL_EXT_client_sync(load);
  load_EGL_EXT_compositor(load);
  load_EGL_EXT_device_base(load);
  load_EGL_EXT_device_enumeration(load);
  load_EGL_EXT_device_persistent_id(load);
  load_EGL_EXT_device_query(load);
  load_EGL_EXT_display_alloc(load);
  load_EGL_EXT_image_dma_buf_import_modifiers(load);
  load_EGL_EXT_output_base(load);
  load_EGL_EXT_platform_base(load);
  load_EGL_EXT_stream_consumer_egloutput(load);
  load_EGL_EXT_surface_compression(load);
  load_EGL_EXT_swap_buffers_with_damage(load);
  load_EGL_EXT_sync_reuse(load);
  load_EGL_HI_clientpixmap(load);
  load_EGL_KHR_cl_event2(load);
  load_EGL_KHR_debug(load);
  load_EGL_KHR_display_reference(load);
  load_EGL_KHR_fence_sync(load);
  load_EGL_KHR_image(load);
  load_EGL_KHR_image_base(load);
  load_EGL_KHR_lock_surface(load);
  load_EGL_KHR_lock_surface3(load);
  load_EGL_KHR_partial_update(load);
  load_EGL_KHR_reusable_sync(load);
  load_EGL_KHR_stream(load);
  load_EGL_KHR_stream_attrib(load);
  load_EGL_KHR_stream_consumer_gltexture(load);
  load_EGL_KHR_stream_cross_process_fd(load);
  load_EGL_KHR_stream_fifo(load);
  load_EGL_KHR_stream_producer_eglsurface(load);
  load_EGL_KHR_swap_buffers_with_damage(load);
  load_EGL_KHR_wait_sync(load);
  load_EGL_MESA_drm_image(load);
  load_EGL_MESA_image_dma_buf_export(load);
  load_EGL_MESA_query_driver(load);
  load_EGL_NOK_swap_region(load);
  load_EGL_NOK_swap_region2(load);
  load_EGL_NV_native_query(load);
  load_EGL_NV_post_sub_buffer(load);
  load_EGL_NV_stream_consumer_eglimage(load);
  load_EGL_NV_stream_consumer_gltexture_yuv(load);
  load_EGL_NV_stream_flush(load);
  load_EGL_NV_stream_metadata(load);
  load_EGL_NV_stream_reset(load);
  load_EGL_NV_stream_sync(load);
  load_EGL_NV_sync(load);
  load_EGL_NV_system_time(load);
  load_EGL_WL_bind_wayland_display(load);
  load_EGL_WL_create_wayland_buffer_from_image(load);
}
*/
}
