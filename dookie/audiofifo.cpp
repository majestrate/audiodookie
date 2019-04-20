#include "audiofifo.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>



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

  ssize_t Poll(std::vector<uint16_t> & left, std::vector<uint16_t> &right) override
  {
    if(left.size() == 0)
      return -1;
    if(left.size() != right.size())
      return -1;
    size_t expect = left.size() + right.size();
    size_t amount = 0;
    size_t left_idx = 0;
    size_t right_idx = 0;
    while(expect)
    {
      if(expect % 2)
      {
        if(::read(fd, left.data() + left_idx, sizeof(uint16_t)) == -1)
          return 0;
        amount ++;
        left_idx ++;
      }
      else
      {
        if(::read(fd, right.data() + right_idx, sizeof(uint16_t)) == -1)
          return 0;
        amount ++;
        right_idx ++;
      }
      expect -- ;
    }
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
