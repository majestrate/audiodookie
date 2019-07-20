#ifndef DOOKIE_WAYLAND_HPP
#define DOOKIE_WAYLAND_HPP

#include "output.hpp"
#include <vector>

namespace dookie
{

struct Context;
struct Wayland
{
  Wayland();
  ~Wayland();

  static void HandleRegistry(void * data,
                             wl_registry * reg,
                             uint32_t name,
                             const char * iface,
                             uint32_t version);
  static void HandleRemove(void * data,
                           wl_registry *reg,
                           uint32_t name);  
  bool OpenDisplay();

  void Register(Context * ctx);

  bool CreateOutput(Context * ctx, wl_output * out);

  void Close();

  void BeforeDraw(Context * ctx);
  void DamageFull(Context * ctx);

  std::vector<DisplayContext_ptr> outputs;
  wl_display * display = nullptr;
  wl_registry * regis = nullptr;
  wl_compositor * compositor = nullptr;
  wl_shm * shm = nullptr;
  zxdg_output_manager_v1 * output_manager = nullptr;
  zwlr_layer_shell_v1 * layer_shell = nullptr;

  wl_registry_listener listener;

};
}

#endif
