#ifndef DOOKIE_ANALYZE_HPP
#define DOOKIE_ANALYZE_HPP
#include <vector>
#include <fftw3.h>
#include <cstdint>

namespace dookie
{

struct ComplexArray
{
  fftw_complex * data;
  const size_t sz;

  ComplexArray(size_t N);
  ~ComplexArray();

  void ToVector(std::vector<double> & out) const;
  void Put(const uint8_t * frames);

  operator fftw_complex * () { return data; };
  
};

struct Analyzer
{
  Analyzer(size_t N);
  ~Analyzer();

  void Analyze(uint8_t * frames, std::vector<double> & out);

private:
  std::vector<double> m_input;
  ComplexArray m_output;
  fftw_plan m_plan;
  
};
}

#endif
