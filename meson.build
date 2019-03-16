project(
	'audiodookie',
	'c++',
	version: '0.0.1',
	license: 'MIT',
	meson_version: '>=0.46.0',
	default_options: [
		'std=c++14',
	],
)

cairo = dependency('cairo')
ftw = dependency('fftw')
wayland_client = dependency('wayland-client')
wayland_protocols = dependency('wayland-protocols', version: '>=1.14')

subdir('protocols')

cxx = meson.get_compiler('c++')

executable(
	'audiodookie',
	files([
    'context.cpp',
		'julia.cpp',
		'main.cpp',
		'output.cpp',
	]),
	dependencies: [
		cairo,
    fftw,
		wayland_client,
		client_protos,
		cxx.find_library('rt'),  # For shm_open
	],
	install: true,
)
