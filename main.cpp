
#include "context.hpp"
#include <iostream>


int main(int argc, char * argv[])
{
  if(argc < 3)
  {
    std::cout << "usage: "<< argv[0] << " <output> <fifopath>" << std::endl;
    return 1;
  }
  Context ctx(argv[1], argv[2]);

  if(!ctx.Connect())
    return 1;

  ctx.Register();
  ctx.RoundTrip();
  if(!ctx.SetUpMainLoop())
    return 2;
  
  return ctx.RunMainLoop();
}
