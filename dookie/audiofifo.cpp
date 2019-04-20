#include "audiofifo.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>



struct FifoAudio : public IAudioSource
{
  FifoAudio() : fd(-1)
  {
  }

  ~FifoAudio()
  {
    if(fd != -1)
      close(fd);
  }
    

  bool Attach(const std::string & dev) override
  {
    if(fd == -1)
      fd = ::open(dev.c_str(), O_RDONLY);
    return fd != -1;
  }

  ssize_t Poll(uint16_t * samps, size_t sz) override
  {
    auto amount = ::read(fd, samps, sz);
    if (amount > 0) amount /= sizeof(uint16_t);
    return amount;
  }
  
  int fd;

  int GetFD() const override
  {
    return fd;
  }
};

std::unique_ptr<IAudioSource> IAudioSource::Create()
{
  return std::make_unique<FifoAudio>();
}
