#ifndef DOOKIE_AUDIOFIFO_HPP
#define DOOKIE_AUDIOFIFO_HPP

#include <string>
#include <memory>
#include <vector>


namespace dookie
{

static constexpr size_t AudioBufferSize = 1024;

struct IAudioSource
{
  virtual ~IAudioSource() {};


  /// attach to device
  virtual bool Attach(const std::string & devname) = 0;

  /// poll for audio frames
  virtual ssize_t Poll(uint8_t * samps, size_t sz) = 0;

  /// get pollable fd
  virtual int GetFD() const = 0;
  
};

std::unique_ptr<IAudioSource> CreateAudioByName(const char * name);

}

#endif
