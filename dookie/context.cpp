#include "context.hpp"
#include <cstring>
#include <sys/timerfd.h>
#include <poll.h>
#include <cstdio>
#include <unistd.h>
#include <iostream>

namespace dookie
{


Context::Context(const char * display, const char * fifopath) : audio(nullptr), analyzer(AudioBufferSize), timerfd(timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC)), _run(false), monitor(display), audiodev(fifopath)
{
}

Context::~Context()
{
}

bool Context::Connect()
{
  wl = std::make_shared<Wayland>();
  return wl->OpenDisplay();
}

void Context::RoundTrip()
{
  ::wl_display_roundtrip(wl->display);
}

void Context::Register()
{
  wl->regis = ::wl_display_get_registry(wl->display);
  ::wl_registry_add_listener(wl->regis, &wl->listener, this);
  RoundTrip();
}


bool Context::SetUpMainLoop()
{
  return true;
}

void Context::TryOpenAudio()
{
  if(audio)
    return;
  std::cout << "try open " << audiodev << std::endl;
  audio = CreateAudioByName(audiodev);
  if(audio->Attach(audiodev))
  {
    std::cout << "we have audio" << std::endl;
    return;
  }
  audio.reset();
}

int Context::RenderFrame()
{
  //std::cout << "render frame" << std::endl;
  return 10;
}

static bool set_timer_milliseconds(int timer_fd, unsigned int delay) {
	struct itimerspec spec = {0};
  spec.it_value.tv_sec = delay / 1000;
  spec.it_value.tv_nsec = (delay % 1000) * (1000000L);
	int ret = timerfd_settime(timer_fd, 0, &spec, NULL);
	if (ret < 0) {
    std::cout << "set timeout failed: " << strerror(errno)  << std::endl;
    errno = 0;
		return false;
	}
	return true;
}


void Context::Stop()
{
  _run = false;
  ::close(timerfd);
  wl->Close();
}

bool 
Context::WantsDisplay(const char * display) const
{
  return strcmp(display, monitor) == 0;
}

int Context::RunMainLoop()
{
  constexpr size_t _WAYLAND_EVENT = 0; 
  constexpr size_t _TIMER_EVENT = 1;
  constexpr size_t _AUDIO_EVENT = 2;
  int polled = 0;
	pollfd events[3] = {0};
  events[_WAYLAND_EVENT].fd = wl_display_get_fd(wl->display);
  events[_WAYLAND_EVENT].events = POLLIN;
  events[_TIMER_EVENT].fd = timerfd;
  events[_TIMER_EVENT].events = POLLIN;
  set_timer_milliseconds(timerfd, 100);
  _run = true;
  // audio buffer
  std::vector<uint8_t> samps(AudioBufferSize);
  std::vector<double> freqs(AudioBufferSize);
  uint8_t * ptr = samps.data();
  while(_run)
  {
    while (wl_display_prepare_read(wl->display) != 0)
    {
      wl_display_dispatch_pending(wl->display);
    }
    wl_display_flush(wl->display);
    int polled;
    TryOpenAudio();
    if(audio)
    {
      events[_AUDIO_EVENT].fd = audio->GetFD();
      events[_AUDIO_EVENT].events = POLLIN;
      polled = poll(events, 3, -1);
    }
    else
      polled = poll(events, 2, -1);
		if (polled < 0)
    {
			wl_display_cancel_read(wl->display);
			if (errno == EINTR)
      	continue;
			break;
		}
		if (events[_WAYLAND_EVENT].revents & POLLIN)
    {
			if (wl_display_read_events(wl->display) != 0)
      {
        std::cout << "wl_display read events != 0" << std::endl;
				if (errno == 104)
        {
					// Compositor disconnected us
				}
				else
        {
					fprintf(stderr, "Failed to read Wayland events: %s\n",
							strerror(errno));
				}
				_run = false;
      }
		}
		else
			wl_display_cancel_read(wl->display);

		if (!_run) 
			break;
    if (audio)
    {
      if (events[_AUDIO_EVENT].revents & POLLIN)
      {
        auto dlt = (samps.data() + samps.size()) - ptr;
        if(dlt)
        {
          ssize_t frameCount = audio->Poll(ptr, dlt);
          if(frameCount > 0)
          {
            ptr += frameCount;
            dlt = (samps.data() + samps.size()) - ptr;
          }
        }
        if(dlt == 0)
        {
          if(wl->outputs.size())
          {
            analyzer.Analyze(samps.data(), freqs);
            for(auto & out : wl->outputs)
              visualizer.Visualize(samps, freqs, *out);
            RoundTrip();
          }
          else
          {
            std::cout << "no outputs" << std::endl;
          }
          std::fill(samps.begin(), samps.end(), 0);
          ptr = samps.data();
        }
      }
    }
		if (events[_TIMER_EVENT].revents & POLLIN)
    {
			int fd = events[_TIMER_EVENT].fd;

			uint64_t expirations;
			ssize_t n = ::read(
					events[_TIMER_EVENT].fd,
					&expirations,
					sizeof(expirations));

			
      set_timer_milliseconds(fd, 100);
			
		}
	}

  return 1;
}



void Context::HandleRegistry(wl_registry * reg,
                    uint32_t name,
                    const char * interface,
                    uint32_t ver)
{
  std::cout << "handle registry: " << interface << std::endl;
  if(strcmp(interface, wl_compositor_interface.name) == 0)
  {
    wl->compositor = static_cast<wl_compositor*>(wl_registry_bind(reg, name, &wl_compositor_interface, 3));
  }
  else if (strcmp(interface, wl_shm_interface.name) == 0)
  {
		wl->shm = static_cast<wl_shm*>(wl_registry_bind(reg, name, &wl_shm_interface, 1));
	}
	else if (strcmp(interface, wl_output_interface.name) == 0)
  {
		wl_output * output = static_cast<wl_output*>(wl_registry_bind(
                                                   reg, name, &wl_output_interface, ver));
		if(wl->CreateOutput(this, output))
    {
      std::cout << "we created an output" << std::endl;
    }
    else
    {
      std::cout << "failed to create output" << std::endl;
      ::exit(1);
    }
	}
	else if (strcmp(interface, zxdg_output_manager_v1_interface.name) == 0)
  {
		wl->output_manager = static_cast<zxdg_output_manager_v1*>(wl_registry_bind(
                                                            reg, name, &zxdg_output_manager_v1_interface, 2));
	}
	else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0)
  {
		wl->layer_shell = static_cast<zwlr_layer_shell_v1*>(wl_registry_bind(
                                                         reg, name, &zwlr_layer_shell_v1_interface, 1));
	}
}
}