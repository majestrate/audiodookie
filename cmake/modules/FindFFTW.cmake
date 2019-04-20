find_package(PkgConfig)

pkg_check_modules(FFTW REQUIRED fftw3)

find_package_handle_standard_args(FFTW
    FOUND_VAR
         FFTW_FOUND
    REQUIRED_VARS
         FFTW_LIBRARIES
    VERSION_VAR
         FFTW_VERSION
    HANDLE_COMPONENTS
)
