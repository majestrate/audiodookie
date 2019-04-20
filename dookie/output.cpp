#include "output.hpp"
#include "context.hpp"
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cairo.h>

DisplayContext::DisplayContext(Context * c, wl_output * out) :
  ctx(c),
  output(out)
{
  output_listener.scale = [](void *data, wl_output *, int32_t factor) {
    static_cast<DisplayContext *>(data)->scale = factor;
  };
  output_listener.geometry = [](void*, wl_output*, int, int, int, int, int, const char*, const char*, int) {};
  output_listener.mode = [](void*, wl_output*, unsigned int, int, int, int) {};
  output_listener.done = [](void* data, wl_output *) {
    static_cast<DisplayContext*>(data)->OutputDone();
  };
  layer_surface_listener.configure = [](void * data, zwlr_layer_surface_v1* surface, uint32_t serial, uint32_t w, uint32_t h) {
    static_cast<DisplayContext*>(data)->ConfigureLayerSurface(serial, w, h);
  };
  layer_surface_listener.closed = [](void * data, zwlr_layer_surface_v1 *)
  {
    static_cast<DisplayContext*>(data)->ClosedLayerSurface();
  };
  xdg_out_listener.name = [](void*, zxdg_output_v1*, const char*) {};
  xdg_out_listener.description =[](void*, zxdg_output_v1*, const char*) {};
  xdg_out_listener.done = [](void * data, zxdg_output_v1 *)
  {
    static_cast<DisplayContext*>(data)->HandleXDGDone();
  };
  xdg_out_listener.logical_position = [](void*, zxdg_output_v1*, int, int) {};
  xdg_out_listener.logical_size = [](void*, zxdg_output_v1*, int, int) {};
}

void
DisplayContext::ClosedLayerSurface()
{
  zwlr_layer_surface_v1_destroy(layer_surface);
}

void
DisplayContext::OutputDone()
{
  
}

bool
DisplayContext::Init()
{
  wl_output_add_listener(output, &output_listener, this);
  surface = wl_compositor_create_surface(ctx->wl.compositor);
  if(surface == nullptr)
  {
    std::cout << "could not create surface for output" << std::endl;
    return false;
  }
  wl_region * input_region = wl_compositor_create_region(ctx->wl.compositor);
  if(input_region == nullptr)
  {
    std::cout << "could not create input region for output" << std::endl;
    return false;
  }
  wl_surface_set_input_region(surface, input_region);
  wl_region_destroy(input_region);
  layer_surface = zwlr_layer_shell_v1_get_layer_surface(ctx->wl.layer_shell, surface, output, ZWLR_LAYER_SHELL_V1_LAYER_BACKGROUND, "wallpaper");
  if(layer_surface == nullptr)
  {
    std::cout << "failed to create layer surface for output" << std::endl;
    return false;
  }
  zxdg_output_manager_v1 * output_manager = ctx->wl.output_manager;
  if(output_manager)
  {
    xdg_out = zxdg_output_manager_v1_get_xdg_output(output_manager, output);
    zxdg_output_v1_add_listener(xdg_out, &xdg_out_listener, this);
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
  ctx->RoundTrip();
  return true;
}

void DisplayContext::HandleXDGDone()
{
  if(xdg_out)
    zxdg_output_v1_destroy(xdg_out);
  xdg_out = nullptr;
}

DisplayContext::~DisplayContext()
{ 
  if (surface) 
		wl_surface_destroy(surface);
	
	if (layer_surface) 
		zwlr_layer_surface_v1_destroy(layer_surface);
}

void DisplayContext::ConfigureLayerSurface(uint32_t serial, uint32_t w, uint32_t h)
{
  width = w;
  height = h;
  wl_region * opaque = wl_compositor_create_region(ctx->wl.compositor);
  wl_region_add(opaque, 0, 0, w, h);
  wl_surface_set_opaque_region(surface, opaque);
  wl_region_destroy(opaque);
  zwlr_layer_surface_v1_ack_configure(layer_surface, serial);
  if(CreateBuffers())
    std::cout << "layer surface configured" << std::endl;
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
  wl_surface_set_buffer_scale(surface, scale);
}

void DisplayContext::DamageFull()
{
  wl_surface_attach(surface, buffer, 0, 0);
  wl_surface_damage(surface, 0, 0, width, height);
  wl_surface_commit(surface);
}


bool DisplayContext::CreateBuffers()
{
  if(buffer)
    return true;
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
  wl_shm_pool * pool = wl_shm_create_pool(ctx->wl.shm, fd, sz);
  buffer = wl_shm_pool_create_buffer(pool, 0, width, height, stride, WL_SHM_FORMAT_ARGB8888);
  wl_shm_pool_destroy(pool);
  ::close(fd);
  cairo_surface = cairo_image_surface_create_for_data(static_cast<uint8_t*>(data), CAIRO_FORMAT_ARGB32, width, height, stride);
  cairo = cairo_create(cairo_surface);
  std::cout << "made buffer of size " << sz << std::endl;
  return true;
}
