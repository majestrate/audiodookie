
#include "dookie/context.hpp"
#include <iostream>


int main(int argc, char * argv[])
{
  if(argc < 3)
  {
    std::cout << "usage: "<< argv[0] << " <output> <fifopath>" << std::endl;
    return 1;
  }
  Context ctx(argv[2]);

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
