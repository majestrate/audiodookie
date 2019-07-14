
#include "dookie/context.hpp"
#include <iostream>


int main(int argc, char * argv[])
{
  static std::string dev = "pulse:default";
  Context ctx("", dev.c_str());

  if(!ctx.Connect())
  {
    std::cout << "failed to connect to wayland" << std::endl;
    return 1;
  }

  ctx.Register();
  ctx.RoundTrip();
  if(!ctx.SetUpMainLoop())
  {
    std::cout << "failed to set up mainloop" << std::endl;
    return 2;
  }
  return ctx.RunMainLoop();
}
