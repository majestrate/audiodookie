#include "audiofifo.hpp"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <iostream>
#include <pulse/simple.h>
#include <pulse/sample.h>
#include <cstring>
#include <thread>
#include <functional>

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

  ssize_t Poll(uint8_t * samps, size_t sz) override
  {
    return ::read(fd, samps, sz);
  }
  
  int fd;

  int GetFD() const override
  {
    return fd;
  }
};


struct PA_Fifo : public IAudioSource
{
  int fds[2] = {-1, -1};

  uint8_t buffer[128];
  pa_simple * s = nullptr;
  std::thread * m_thread = nullptr;

  PA_Fifo()
  {
  }

  ~PA_Fifo()
  {
    if(fds[0] != -1)
      ::close(fds[0]);
    if(fds[1] != -1)
      ::close(fds[1]);
    if(m_thread)
    {
      m_thread->join();
      delete m_thread;
    }
  }

  void Run()
  {
    int err;
    while(::pa_simple_read(s, buffer, sizeof(buffer), &err) == 0)
    {
      if(::write(fds[1], buffer, sizeof(buffer)) == -1)
      {
        std::cout << "bad write on pipe" << std::endl;
        break;
      }
      if(::pa_simple_flush(s, &err))
      {
        std::cout << "bad pa flush" << std::endl;
      }
    }
    ::pa_simple_free(s);
    std::cout << "pa done" << std::endl;
  }

  bool Attach(const std::string & name) override
  {
    if(fds[0] == -1 || fds[1] == -1)
    {
      if(pipe(fds) == -1)
        return false;
    }
    int err;
    pa_sample_spec ss;
    ss.channels = 1;
    ss.rate = 44100;
    ss.format = PA_SAMPLE_U8;
    // std::string dev = name.substr(::strlen("pulse:") );
    // std::cout << "open " << dev << std::endl;
    s = ::pa_simple_new(nullptr, "audiodookie", PA_STREAM_RECORD, nullptr, "wallpaper", &ss, nullptr, nullptr, &err);
    if(s == nullptr)
    {
      
      return false;
    }
    m_thread = new std::thread(std::bind(&PA_Fifo::Run, this));
    return true;
  }

  ssize_t Poll(uint8_t * samps, size_t sz) override 
  {
    return ::read(fds[0], samps, sz);
  }

  int GetFD() const override
  {
    return fds[0];
  };
};

std::unique_ptr<IAudioSource> CreateAudioByName(const char * cname)
{
  std::string name(cname);
  if(name.find("pulse:") == 0)
    return std::make_unique<PA_Fifo>();
  else
    return std::make_unique<FifoAudio>();
}
