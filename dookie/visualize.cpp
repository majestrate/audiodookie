#include "visualize.hpp"
#include "context.hpp"
#include <iostream>

void Visualizer::Visualize(const std::vector<uint16_t> & samps, const std::vector<double> & freqs, DisplayContext & ctx)
{
  // this is a heavy as shit codepath yo
  if(ctx.cairo)
  {
    ctx.BeforeDraw();
    cairo_surface_flush(ctx.cairo_surface);
    double amp = 0;
    double sum = 1.0;
    cairo_set_source_rgb(ctx.cairo, 0, 0, 0);
    cairo_rectangle(ctx.cairo, 0,0, ctx.width,ctx.height);
    cairo_fill(ctx.cairo);
    cairo_set_source_rgb(ctx.cairo, sum, sum, sum);
    const auto w = 10.0;
    const size_t num = 128;
    for(size_t idx = 1; idx < num; ++idx)
    {
      const auto freq = abs(freqs[idx]);
      cairo_move_to(ctx.cairo, idx * w, 0);
      cairo_line_to(ctx.cairo, idx * w, freq * 5);
      cairo_set_line_width(ctx.cairo, w);
      cairo_stroke(ctx.cairo);
      amp += freq;
    }
    amp /= (num * 100);
    sum /= (100);
    std::cout << sum << std::endl;
    std::cout << amp << std::endl;
    //cairo_paint(ctx.cairo);
    cairo_surface_mark_dirty(ctx.cairo_surface);
    ctx.DamageFull();
  }
}
