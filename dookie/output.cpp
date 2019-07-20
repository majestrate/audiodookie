#include "output.hpp"
#include "context.hpp"
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cairo.h>

namespace dookie
{

static void ConfigureSurface(void * data, zwlr_layer_surface_v1* surface, uint32_t serial, uint32_t w, uint32_t h) {
  std::cout << "configure layer surface" << std::endl;
  static_cast<DisplayContext*>(data)->ConfigureLayerSurface(surface, serial, w, h);
};

static void CloseSurface(void *, zwlr_layer_surface_v1 *)
{
}

static void GotScale(void *data, wl_output *, int32_t factor) {
    std::cout << "got scale" << std::endl;
    static_cast<DisplayContext *>(data)->scale = factor;
};

static void CloseOutput(void *, wl_output *)
{
};

static void OnMode(void *,  wl_output*, uint32_t, int32_t, int32_t, int32_t)
{
};

static void OnIDKLOL(void *,  wl_output*, int32_t, int32_t, int32_t, int32_t, int32_t, const char*, const char*, int32_t)
{
};

static void XDGName(void * data, struct zxdg_output_v1 *, const char * name)
{
  static_cast<DisplayContext*>(data)->name = name;
}

static void XDGDesc(void *, struct zxdg_output_v1 *, const char *)
{
}

static void XDGDone(void *, struct zxdg_output_v1 * out)
{
  zxdg_output_v1_destroy(out);
}

static void XDGNOP(void * , struct zxdg_output_v1 *, int32_t, int32_t) {}


DisplayContext::DisplayContext(Context * c, wl_output * out) :
  ctx(c),
  output(out)
{
  output_listener = {
    &OnIDKLOL,
    &OnMode,
    &CloseOutput,
    &GotScale,
  };
  xdg_out_listener = {
    &XDGNOP,
    &XDGNOP,
    &XDGDone,
    &XDGName,
    &XDGDesc
  };
  layer_surface_listener = {
    &ConfigureSurface,
    &CloseSurface
  };
}

void
DisplayContext::ClosedLayerSurface()
{
  std::cout << "closed layer surface" << std::endl;
  zwlr_layer_surface_v1_destroy(layer_surface);
}

bool
DisplayContext::Init()
{
  std::cout << "init display" << std::endl;
  if(ctx->wl->layer_shell == nullptr)
  {
    std::cout << "no layer shell" << std::endl;
                                     return false;
  }
  wl_output_add_listener(output, &output_listener, this);
  surface = wl_compositor_create_surface(ctx->wl->compositor);
  if(surface == nullptr)
  {
    std::cout << "could not create surface for output" << std::endl;
    return false;
  }
  wl_region * input_region = wl_compositor_create_region(ctx->wl->compositor);
  if(input_region == nullptr)
  {
    std::cout << "could not create input region for output" << std::endl;
    return false;
  }
  wl_surface_set_input_region(surface, input_region);
  wl_region_destroy(input_region);
  layer_surface = zwlr_layer_shell_v1_get_layer_surface(ctx->wl->layer_shell, surface, output, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "dookie");
  if(layer_surface == nullptr)
  {
    std::cout << "failed to create layer surface for output" << std::endl;
    return false;
  }
  zwlr_layer_surface_v1_set_size(layer_surface, 0, 0);
	zwlr_layer_surface_v1_set_anchor(layer_surface,
			ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
			ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT |
			ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
			ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT);
	zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
	zwlr_layer_surface_v1_add_listener(layer_surface,
			&layer_surface_listener, this);
  wl_surface_commit(surface);
  return true;
}

void DisplayContext::HandleXDGDone()
{
  if(xdg_out)
    zxdg_output_v1_destroy(xdg_out);
  xdg_out = nullptr;
}

void DisplayContext::OutputDone()
{ 
  if (surface) 
		wl_surface_destroy(surface);
	if (layer_surface) 
		zwlr_layer_surface_v1_destroy(layer_surface);
}

void DisplayContext::ConfigureLayerSurface(zwlr_layer_surface_v1 * lsurface, uint32_t serial, uint32_t w, uint32_t h)
{
  layer_surface = lsurface;
  std::cout << "configure layer surface" << std::endl;
 
  width = w;
  height = h;
  std::cout << "we have " << width << "x" << height << std::endl;
  
  wl_region * opaque = wl_compositor_create_region(ctx->wl->compositor);
  wl_region_add(opaque, 0, 0, w, h);
  wl_surface_set_opaque_region(surface, opaque);
  wl_region_destroy(opaque);
  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
  if(CreateBuffers())
  {
    std::cout << "layer surface configured" << std::endl;
  }
  else
    std::cout << " failed to make buffers" << std::endl;
}


static int pid_shm_open(const char * prefix, int flags, mode_t mode)
{
  std::vector<char> path(strlen(prefix));
  const static char fmt[] = "%s-%d";
  pid_t pid = getpid();
  auto l = snprintf(path.data(), path.size(), fmt, prefix, pid);
  if(l < 0)
    return -1;
  int fd = shm_open(path.data(), flags, mode);
  if(fd < 0)
    return -1;
  shm_unlink(path.data());
  return fd;
}

void DisplayContext::BeforeDraw()
{
  if(surface && buffer)
  {
    wl_surface_set_buffer_scale(surface, scale);
  }
}

void DisplayContext::DamageFull()
{
  if(surface && buffer)
  {
    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage(surface, 0, 0, width, height);
    wl_surface_commit(surface);
  }
}


bool DisplayContext::CreateBuffers()
{
  
  uint32_t stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
  size_t sz = stride * height;
  if(sz < 1)
    return false;
  errno = 0;
  int fd = pid_shm_open("/audiodookie-buffer", O_RDWR | O_CREAT | O_EXCL, 0600);
  if(fd < 0)
    return false;
  errno = 0;
  if(ftruncate(fd, sz) < 0)
  {
    ::close(fd);
    return false;
  }
  errno = 0;
  void * data = ::mmap(nullptr, sz, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if(data == nullptr)
  {
    ::close(fd);
    return false;
  }
  wl_shm_pool * pool = wl_shm_create_pool(ctx->wl->shm, fd, sz);
  buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(pool);
  ::close(fd);
  cairo_surface = cairo_image_surface_create_for_data(static_cast<uint8_t*>(data), CAIRO_FORMAT_ARGB32, width, height, stride);
  cairo = cairo_create(cairo_surface);
  std::cout << "made buffer of size " << sz << std::endl;
  return true;
}

}