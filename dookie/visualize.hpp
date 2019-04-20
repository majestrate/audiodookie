#ifndef DOOKIE_VISUALIZE_HPP
#define DOOKIE_VISUALIZE_HPP
#include <vector>
#include <cstdint>
#include "output.hpp"

struct Context;
struct Visualizer
{
  void Visualize(const std::vector<uint16_t> & samps, const std::vector<double> &freqs, DisplayContext & display);
  void BeatStart();
  double R = 0.0;
};


#endif
