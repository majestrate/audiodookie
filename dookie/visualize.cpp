#include "visualize.hpp"
#include "context.hpp"
#include <iostream>

void Visualizer::Visualize(const std::vector<uint8_t> & samps, const std::vector<double> & freqs, DisplayContext & ctx)
{
  // this is a heavy as shit codepath yo
  if(ctx.cairo)
  {
    ctx.BeforeDraw();
    
    double amp = 0;
    double sum = 1.0;
    const size_t numAmps = 6;
    for(size_t idx = 1; idx < numAmps ; ++idx)
      amp += abs(freqs[idx]);
    amp /= numAmps;
    if(amp > 15)
    {
      R = std::max(1.0, R + 0.1);
    }
    else
      R = std::max(0.0, R - 0.1);
    cairo_set_source_rgb(ctx.cairo, R, 0, 0);
    cairo_rectangle(ctx.cairo, 0,0, ctx.width,ctx.height);
    cairo_fill(ctx.cairo);
    for(const auto & samp : samps)
    {
      sum += uint32_t{samp} * uint32_t{samp};
    }
    sum = sqrt(sum);
    sum /= samps.size();
    sum /= 256;
    sum = 1.0 - sum;
    cairo_set_source_rgb(ctx.cairo, sum, sum, sum);
    const auto w = 10.0 ;
    const auto h = 10.0 * sum;
    const size_t num = 128;
    for(size_t idx = 1; idx < num; ++idx)
    {
      const auto freq = abs(freqs[idx]);
      cairo_move_to(ctx.cairo, idx * w, 0);
      cairo_line_to(ctx.cairo, idx * w, freq * h);
      cairo_set_line_width(ctx.cairo, w);
      cairo_stroke(ctx.cairo);
      amp += freq;
    }
    cairo_surface_mark_dirty(ctx.cairo_surface);
    ctx.DamageFull();
  }
}
