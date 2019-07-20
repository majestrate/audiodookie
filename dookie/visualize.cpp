#include "visualize.hpp"
#include "context.hpp"
#include <iostream>

namespace dookie 
{

CairoVisualizer::~CairoVisualizer()
{
  if(P)
    cairo_pattern_destroy(P);
}

void CairoVisualizer::Visualize(const std::vector<uint8_t> & samps, const std::vector<double> & freqs, DisplayContext & ctx)
{
  // this is a heavy as shit codepath yo
  if(ctx.cairo)
  {
    ctx.BeforeDraw();
    
    double amp = 0;
    double sum = 0;
    const double radius = 50;
    const double center_x = (ctx.width / 2) - radius / 2;
    const double center_y = (ctx.height / 2) - radius / 2;
    if(P == nullptr)
    {
        P = cairo_pattern_reference (cairo_pattern_create_radial(center_x, center_y, radius, center_x, center_y, radius *2));
    }
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
    
    cairo_set_source_rgb(ctx.cairo, 0,0,0);
    
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
    cairo_move_to(ctx.cairo, 0,0);
    for(size_t idx = 1; idx < num; ++idx)
    {
      const auto freq = abs(freqs[idx]);
      cairo_move_to(ctx.cairo, idx * w, 0);
      cairo_line_to(ctx.cairo, idx * w, freq * h);
      cairo_set_line_width(ctx.cairo, w);
      cairo_stroke(ctx.cairo);
      amp += freq;
    }

    cairo_set_source_rgb(ctx.cairo, R,R,R);
    cairo_arc(ctx.cairo, center_x, center_y, amp, 0, 2 * M_PI);
    cairo_stroke_preserve(ctx.cairo);
    cairo_set_source_rgb(ctx.cairo, R, 0, 0); 
    cairo_fill(ctx.cairo);

    cairo_surface_mark_dirty(ctx.cairo_surface);
    ctx.DamageFull();
  }
}

}