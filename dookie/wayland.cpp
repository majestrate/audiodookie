#include "wayland.hpp"
#include "context.hpp"
#include <cassert>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cairo.h>


Wayland::Wayland()
  : display(nullptr),
    regis(nullptr)
{
  listener.global = &Wayland::HandleRegistry;
  listener.global_remove = &Wayland::HandleRemove;
  
}

Wayland::~Wayland()
{
  if(display)
  {
    wl_display_disconnect(display);
    display = nullptr;
  }
}

bool Wayland::OpenDisplay()
{
  if(display)
    return true;
  display = wl_display_connect(nullptr);
  return display != nullptr;
}

void Wayland::HandleRegistry(void * data,
                             wl_registry * reg,
                             uint32_t name,
                             const char * iface,
                             uint32_t version)
{
  static_cast<Context *>(data)->HandleRegistry(reg, name, iface, version);
}


void Wayland::Close()
{
  outputs.clear();
}

void Wayland::HandleRemove(void * data,
                           wl_registry * reg,
                           uint32_t name)
{
  (void) data;
  (void) reg;
  (void) name;
}


void Wayland::Register(Context * ctx)
{
  regis = wl_display_get_registry(display);
  wl_registry_add_listener(regis, &listener, ctx);
  ctx->RoundTrip();
}

bool Wayland::CreateOutput(Context * ctx, wl_output * output)
{
  outputs.emplace_back(ctx, output);
  if(outputs.back().Init())
  {
    return true;
  }

  outputs.pop_back();
  return false;
}
