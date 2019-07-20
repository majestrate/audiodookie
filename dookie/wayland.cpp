#include "wayland.hpp"
#include "context.hpp"
#include <cassert>
#include <iostream>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cairo.h>

namespace dookie
{


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
    ::wl_display_disconnect(display);
    display = nullptr;
  }
}

bool Wayland::OpenDisplay()
{
  if(display)
    return true;
  display = ::wl_display_connect(nullptr);
  return display != nullptr;
}

void Wayland::HandleRegistry(void * data,
                             wl_registry * reg,
                             uint32_t name,
                             const char * iface,
                             uint32_t version)
{
  auto * ctx = static_cast<Context *>(data);
  ctx->HandleRegistry(reg, name, iface, version);
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

bool Wayland::CreateOutput(Context * ctx, wl_output * output)
{
  outputs.emplace_back(std::make_shared<DisplayContext>(ctx, output));
  auto & out = outputs.back();
  if(out->Init())
  {
    return true;
  }
  std::cout << "failed to make output" << std::endl;
  outputs.pop_back();
  return false;
}
}