macro(singularity_enable_hdf5 target)
  if(NOT HDF5_ROOT)
    find_path(HDF5_INCLUDE_DIRS hdf5.h
      HINTS ENV HDF5_ROOT
      PATH_SUFFIXES include)
    get_filename_component(HDF5_ROOT ${HDF5_INCLUDE_DIRS} DIRECTORY)
  endif()

  # mauneyc 20230525
  # SOOOOO.....
  # HDF5 detection has a lot of unremarked on dependencies and build checkpoints
  # that turn this into an inscrutable failure at times.
  # here's what I've found, though I can't really make sense of when/how/where this applies 
  #
  # 1.) if 'C' component is requested, 'enable_language(C)' needs to be done beforehand.
  #     otherwise, FindHDF5.cmake _may_ fail to detect anything.
  #   1a.) I say _may_ because this behavior is sporadic across CMake/HDF5 versions.
  # 2.) the 'C' component _may_ be required. On some installs, 'HL' is sufficent; on others,
  #     both are needed.
  # 3.) MPI behavior: rather than just *tell* us what MPI libraries it needs, we have to 
  #     just guess. Using both MPI_C and MPI_CXX as a 'kitchen-sink' approach is what works
  #     (currently) although this may be an issue in other platforms.
  #     Unfortunately, this won't give a configuration error, as the target check isn't performed
  #     until export. Will have to think about trying to catch these issues.
  if(NOT HDF5_LIBRARIES)
    find_package(HDF5 COMPONENTS C HL REQUIRED)
  endif()

  target_include_directories(${target} SYSTEM PUBLIC ${HDF5_INCLUDE_DIRS})
  target_link_libraries(${target} PUBLIC ${HDF5_LIBRARIES})

  if(HDF5_IS_PARALLEL)
    find_package(MPI COMPONENTS C CXX REQUIRED)
    target_link_libraries(${target} PUBLIC MPI::MPI_C MPI::MPI_CXX)
  endif()

  target_compile_definitions(${target} PUBLIC SPINER_USE_HDF5)
  target_compile_definitions(${target} PUBLIC SINGULARITY_USE_HDF5)

endmacro()

