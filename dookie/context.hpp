#ifndef DOOKIE_CONTEXT_HPP
#define DOOKIE_CONTEXT_HPP
#include "wayland.hpp"
#include "audiofifo.hpp"
#include "analyze.hpp"
#include "visualize.hpp"

struct Context
{
  Context(const char * fifopath);
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

  Wayland wl;
  uint32_t width;
  uint32_t height;
  int32_t scale;
  
  void Stop();
  
private:

  void TryOpenAudio();
  
  int RenderFrame();
  
  std::unique_ptr<IAudioSource> audio;
  Analyzer analyzer;
  Visualizer visualizer;
  int timerfd;
  bool _run;
  const char * audiodev;
};

#endif
