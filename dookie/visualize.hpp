#ifndef DOOKIE_VISUALIZE_HPP
#define DOOKIE_VISUALIZE_HPP
#include <vector>
#include <cstdint>
#include "output.hpp"

namespace dookie 
{
struct Context;
struct BaseVisualizer
{
  BaseVisualizer() = default;
  virtual ~BaseVisualizer() = default;
  virtual void Visualize(const std::vector<uint8_t> & samps, const std::vector<double> &freqs, DisplayContext & display) = 0;
  double R = 0.0;

};


struct CairoVisualizer : public BaseVisualizer
{
  ~CairoVisualizer();
  cairo_pattern_t *P = nullptr;
  void Visualize(const std::vector<uint8_t> & samps, const std::vector<double> &freqs, DisplayContext & display) override;
};

struct EGLVisualizer : public BaseVisualizer
{
  EGLVisualizer();
  void Visualize(const std::vector<uint8_t> & samps, const std::vector<double> &freqs, DisplayContext & display) override;
};


using Visualizer = CairoVisualizer;
}
#endif
