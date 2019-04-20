#ifndef DOOKIE_OUTPUT_HPP
#define DOOKIE_OUTPUT_HPP

#include <wayland-client.h>
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#include "xdg-output-unstable-v1-client-protocol.h"
#include <cairo.h>

struct Wayland;
struct Context;

/// a signle wayland output
struct DisplayContext
{

  void ConfigureLayerSurface(zwlr_layer_surface_v1 * surface, uint32_t serial, uint32_t w, uint32_t h);
  void ClosedLayerSurface();
  void HandleXDGDone();
  
  wl_output * output = nullptr;
  wl_surface * surface = nullptr;
  wl_buffer * buffer = nullptr;
  cairo_t * cairo = nullptr;
  cairo_surface_t * cairo_surface = nullptr;
  zwlr_layer_surface_v1 * layer_surface = nullptr;
  zxdg_output_v1 * xdg_out = nullptr;
  Context * ctx = nullptr;
  
  static zxdg_output_v1_listener xdg_out_listener;
  static wl_output_listener output_listener;
  static zwlr_layer_surface_v1_listener layer_surface_listener;

  uint32_t width = 0;
  uint32_t height = 0;
  int32_t scale  = 0;

  DisplayContext(Context * context, wl_output * out);
  
  bool Init();
  bool CreateBuffers();
  void OutputDone();
  void BeforeDraw();
  void DamageFull();
};

#endif
