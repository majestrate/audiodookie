#include "baka.hpp"

void BakaState::handle_registery_cb(void * data, wl_registry * reg,
                             uint32_t name, const char * interface,
                             uint32_t version)
{
  BakaState * state = static_cast<BakaState *>(data);
  state->HandleRegistry(reg, name, interface, version);
}
