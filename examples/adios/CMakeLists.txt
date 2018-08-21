###-------------------------------------------------------------------------###
### CMakeList.txt for examples using pio
###-------------------------------------------------------------------------###

# Include PIO include and lib directories
INCLUDE_DIRECTORIES(${PIO_INCLUDE_DIRS})
include_directories("${CMAKE_SOURCE_DIR}/examples/adios")
LINK_DIRECTORIES(${PIO_LIB_DIR})

set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -g -O0")
# Compiler-specific compiler options
if ("${CMAKE_C_COMPILER_ID}" STREQUAL "GNU")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "PGI")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -c99")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -c99")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Intel")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")
elseif ("${CMAKE_C_COMPILER_ID}" STREQUAL "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")
endif()
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

ADD_EXECUTABLE(example_adios1 example_adios1.c)
TARGET_LINK_LIBRARIES(example_adios1 pioc)
add_dependencies(tests example_adios1)

ADD_EXECUTABLE(example_adios2 example_adios2.c)
TARGET_LINK_LIBRARIES(example_adios2 pioc)
add_dependencies(tests example_adios2)

ADD_EXECUTABLE(timesteps timesteps.c)
TARGET_LINK_LIBRARIES(timesteps pioc)
add_dependencies(tests timesteps)

ADD_EXECUTABLE(timesteps_double timesteps_double.c)
TARGET_LINK_LIBRARIES(timesteps_double pioc)
add_dependencies(tests timesteps_double)

ADD_EXECUTABLE(example_typeconvert example_typeconvert.c)
TARGET_LINK_LIBRARIES(example_typeconvert pioc)
add_dependencies(tests example_typeconvert)

ADD_EXECUTABLE(example_shiftdecomp example_shiftdecomp.c)
TARGET_LINK_LIBRARIES(example_shiftdecomp pioc)
add_dependencies(tests example_shiftdecomp)

ADD_EXECUTABLE(example_double example_double.c)
TARGET_LINK_LIBRARIES(example_double pioc)
add_dependencies(tests example_double)

ADD_EXECUTABLE(example_filename example_filename.c)
TARGET_LINK_LIBRARIES(example_filename pioc)
add_dependencies(tests example_filename)

ADD_EXECUTABLE(example_noclobber example_noclobber.c)
TARGET_LINK_LIBRARIES(example_noclobber pioc)
add_dependencies(tests example_noclobber)

ADD_EXECUTABLE(example_decomphole example_decomphole.c)
TARGET_LINK_LIBRARIES(example_decomphole pioc)
add_dependencies(tests example_decomphole)

ADD_EXECUTABLE(example_put_var example_put_var.c)
TARGET_LINK_LIBRARIES(example_put_var pioc)
add_dependencies(tests example_put_var)

ADD_EXECUTABLE(example_weird_a example_weird_a.c)
TARGET_LINK_LIBRARIES(example_weird_a pioc)
add_dependencies(tests example_weird_a)

ADD_EXECUTABLE(example_fillvalue example_fillvalue.c)
TARGET_LINK_LIBRARIES(example_fillvalue pioc)
add_dependencies(tests example_fillvalue)

###-------------------------------------------------------------------------###
### CMakeList.txt for examples using pio
###-------------------------------------------------------------------------###

if (PIO_ENABLE_FORTRAN)
  if (WITH_ADIOS)

    # Adding PIO definitions - defined in the root directory
    ADD_DEFINITIONS(${PIO_DEFINITIONS})

    # Include PIO include and lib directories
    INCLUDE_DIRECTORIES(${PIO_INCLUDE_DIRS})
    LINK_DIRECTORIES(${PIO_LIB_DIR})

    set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -g -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    if(${PIO_BUILD_TIMING})
      SET(TIMING_LINK_LIB timing)
    endif()

    SET(SRC examplePioADIOS.f90)
    ADD_EXECUTABLE(examplePioADIOS_f90 ${SRC})
    TARGET_LINK_LIBRARIES(examplePioADIOS_f90 piof pioc ${TIMING_LINK_LIB})
  endif(WITH_ADIOS)

endif(PIO_ENABLE_FORTRAN)
