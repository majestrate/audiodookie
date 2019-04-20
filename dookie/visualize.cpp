#include "visualize.hpp"
#include "context.hpp"
#include <iostream>

void Visualizer::Visualize(const std::vector<uint16_t> & samps, const std::vector<double> & freqs, DisplayContext & ctx)
{
  // this is a heavy as shit codepath yo
  ctx.BeforeDraw();
  cairo_t * cairo = ctx.cairo;
  double amp = freqs[2] / 64.0;
  cairo_set_source_rgb(cairo, 0, amp, 0);
  cairo_rectangle(cairo, 0.25, 0.25, 0.5,0.5);
  cairo_fill(cairo);
  cairo_paint(cairo);
  ctx.DamageFull();
}
