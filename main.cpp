
#include "dookie/context.hpp"
#include <iostream>


int main(int argc, char * argv[])
{
  const char * dev = "pulse:default";
  if(argc > 1)
    dev = argv[1];
  auto ctx= std::make_shared<dookie::Context>("", dev);

  if(!ctx->Connect())
  {
    std::cout << "failed to connect to wayland" << std::endl;
    return 1;
  }

  ctx->Register();
  if(!ctx->SetUpMainLoop())
  {
    std::cout << "failed to set up mainloop" << std::endl;
    return 2;
  }
  return ctx->RunMainLoop();
}
