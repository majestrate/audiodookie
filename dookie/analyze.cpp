#include "analyze.hpp"
#include <iostream>

Analyzer::Analyzer(size_t N) : m_input(N), m_output(N)
{
  m_plan = fftw_plan_dft_r2c_1d(N, m_input.data(), m_output, FFTW_MEASURE);
}

Analyzer::~Analyzer()
{
  fftw_destroy_plan(m_plan);
}


ComplexArray::ComplexArray(size_t N) :
  data(fftw_alloc_complex(N)), sz(N)
{
}

ComplexArray::~ComplexArray()
{
  fftw_free(data);
}

void ComplexArray::ToVector(std::vector<double> & out) const
{
  for(size_t idx = 1; idx < (sz - 1); ++idx)
    out[idx-1] = abs(data[idx][0]); // real
}

void ComplexArray::Put(const uint16_t * frames)
{
  for(size_t idx = 0; idx < sz; ++idx)
  {
    data[idx][0] = frames[idx];
    data[idx][1] = 0.0;
  }
}

void Analyzer::Analyze(const uint16_t *frames, std::vector<double> & out)
{
  for(size_t idx = 0; idx < m_input.size(); idx ++)
  {
    m_input[idx] = frames[idx] / 65636.0;
  }
  fftw_execute(m_plan);
  m_output.ToVector(out);
}
