#ifndef DOOKIE_CONTEXT_HPP
#define DOOKIE_CONTEXT_HPP
#include "wayland.hpp"
#include "audiofifo.hpp"
#include "analyze.hpp"
#include "visualize.hpp"

namespace dookie
{

struct Context
{
  Context(const char * monitorName, const char * fifopath);
  ~Context();

  /// connect to wayland display
  bool Connect();

  /// trigger wayland registery
  void Register();
  /// do wayland round trip
  void RoundTrip();

  /// prepare for mainloop
  bool SetUpMainLoop();

  /// run mainloop
  int RunMainLoop();

  void HandleRegistry(wl_registry * reg,
                      uint32_t name,
                      const char * iface,
                      uint32_t version);

  std::shared_ptr<Wayland> wl;
  
  void Stop();
  
  bool 
  WantsDisplay(const char * name) const;

private:

  void TryOpenAudio();
  
  int RenderFrame();
  
  std::unique_ptr<IAudioSource> audio;
  Analyzer analyzer;
  Visualizer visualizer;
  int timerfd;
  bool _run;
  const char * monitor;
  const char * audiodev;
};
}
#endif
