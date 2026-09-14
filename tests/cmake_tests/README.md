A set of tests for PDI cmake.

configtst
: a test of PDI installed CMake config that checks that all targets are made available, and that multiple calls to `find_package()` work, each adding
  the requested components, never removing previously requested ones, including when called from a subdirectory added by `add_subdirectory()`.

