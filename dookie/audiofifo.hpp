#ifndef DOOKIE_AUDIOFIFO_HPP
#define DOOKIE_AUDIOFIFO_HPP

#include <string>
#include <memory>
#include <vector>

struct IAudioSource
{
  virtual ~IAudioSource() {};

  static std::unique_ptr<IAudioSource> Create();

  /// attach to device
  virtual bool Attach(const std::string & devname) = 0;

  /// poll for audio frames
  /// frames is filled give its size()
  virtual ssize_t Poll(std::vector<uint16_t> & left, std::vector<uint16_t> & right) = 0;

  /// get pollable fd
  virtual int GetFD() const = 0;
  
};



#endif
