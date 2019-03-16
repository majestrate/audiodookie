x#ifndef BAKA_HPP
#define BAKA_HPP

struct State
{
  static void handle_register_cb(void *, wl_registry *, uint32_t, const char *, uint32_t)

  void HandleRegistery(wl_registry * wl, uint32_t name, std::string_view interface, uint32_t ver);
};


struct Context
{
  Context(const char * output, const char * fifopath);
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
  
private:
  State state;
  wl_display * display;
  int fifofd;
  int timerfd;
};

#endif
