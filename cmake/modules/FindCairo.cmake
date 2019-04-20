find_package(PkgConfig)

pkg_check_modules(CAIRO REQUIRED cairo)

find_package_handle_standard_args(CAIRO
    FOUND_VAR
         CAIRO_FOUND
    REQUIRED_VARS
         CAIRO_INCLUDE_DIRS
         CAIRO_LIBRARIES
    VERSION_VAR
         CAIRO_VERSION
    HANDLE_COMPONENTS
)
