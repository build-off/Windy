# Jolt configuration

# When turning this option on, the library will be compiled using
#doubles for positions. This allows for much bigger worlds.
set(DOUBLE_PRECISION, OFF)

set(GENERATE_DEBUG_SYMBOLS ON)
set(OVERRIDE_CXX_FLAGS OFF) # allow to override jolt the default flags

# jolt is going to be compiled trying to keep the simulations
# deterministic across platforms if on
set(CROSS_PLATFORM_DETERMINISTIC OFF)
# When turning this on, in Debug and Release mode, the library will emit extra code
# to ensure that the 4th component of a 3-vector is kept the same as the 3rd component 
# and will enable floating point exceptions during simulation to detect divisions by zero. 
# Note that this currently only works using MSVC. Clang turns Float2 into a SIMD vector
# sometimes causing floating point exceptions (the option is ignored).
set(FLOATING_POINT_EXCEPTIONS_ENABLED OFF)
# When turning this on, the library will be compiled with C++ exceptions enabled.
# This adds some overhead and Jolt doesn't use exceptions so by default it is off.
set(CPP_EXCEPTIONS_ENABLED OFF)
# Number of bits to use in ObjectLayer. Can be 16 or 32.
set(OBJECT_LAYER_BITS 16)
