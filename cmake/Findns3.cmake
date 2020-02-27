# CMake script to find ns3 

# Tunable cache variables:
# - NS3_USE_BUILD_TREE:  If this is enabled, it will try to link against
#   libraries from an ns3 build tree, rather than an installed ns3.
#   This can be useful if one may change and rebuild ns3; since it
#   allows skipping the steps of installing it each time.  Note that
#   this may not quite work as expected if NS3 was installed at the same
#   time, since the linker may then pick the wrong libraries.
# - NS3_BUILD_TREE:  If enabled, this should point to an ns3 build tree,
#   i.e. the build/ subdirectory of an ns3 checkout that has been built.

set(NS3_USE_BUILD_TREE OFF CACHE BOOL
  "Use ns3 from an ns3 build tree rather than installed")
set(NS3_BUILD_TREE "" CACHE FILEPATH
  "Location of the ns3 build tree")

# add the given suffix to each list element of baselist and return the
# resulting new list in the result list
function(ns3_cartesian_product result list_a list_b)
  set(result_)
  foreach(a ${list_a})
    foreach(b ${list_b})
      list(APPEND result_ "${a}${b}")
    endforeach()
  endforeach()

  # Export result_
  set(${result} ${result_} PARENT_SCOPE)
endfunction()

# Version numbers supported by this find script
set(ns3_version_names 3.29 3-dev)

#  List of all the ns3 library names
set(ns3_libs
	antenna
	aodv
	applications
	bridge
	buildings
	config-store
	core
	csma
	csma-layout
	dsdv
	dsr
	energy
	fd-net-device
	flow-monitor
	internet-apps
	internet
	lr-wpan
	lte
	mesh
	mobility
	mpi
	netanim
	network
	nix-vector-routing
	olsr
	point-to-point
	point-to-point-layout
	propagation
	sixlowpan
	spectrum
	stats
	tap-bridge
	topology-read
	traffic-control
	uan
	virtual-net-device
	wave
	wifi
	wimax
)

# Find location of header files
if(NOT NS3_USE_BUILD_TREE)
  ns3_cartesian_product(ns3_include_suffixes "ns" "${ns3_version_names}")
  find_path(ns3_include_dir
    NAME ns3/core-module.h
    PATH_SUFFIXES ${ns3_include_suffixes}
  )
else()
  find_path(ns3_include_dir
    NAME ns3/core-module.h
    PATHS ${NS3_BUILD_TREE}
    NO_DEFAULT_PATH
  )
endif()
message(-- " ns3 include directory: ${ns3_include_dir}")

# Find all the NS3 libraries
foreach(ns3_lib ${ns3_libs})
  ns3_cartesian_product(libnames "ns" "${ns3_version_names}")
  ns3_cartesian_product(libnames "${libnames}" "-")
  ns3_cartesian_product(libnames "${libnames}" ${ns3_lib})
  ns3_cartesian_product(libnames "${libnames}" "-debug;-optimized")
  if(NOT NS3_USE_BUILD_TREE)
    find_library(NS3_LOCATION_${ns3_lib}
      NAMES ${libnames}
    )
  else()
    find_library(NS3_LOCATION_${ns3_lib}
      NAMES ${libnames}
      PATHS ${NS3_BUILD_TREE}/lib
      NO_DEFAULT_PATH
    )
  endif()
endforeach()

# Add a library target for each found library
set(ns3_all 0)
if(ns3_include_dir)
  foreach(ns3_lib ${ns3_libs})
    if(NS3_LOCATION_${ns3_lib})
      add_library(ns3::${ns3_lib} SHARED IMPORTED)
      set_target_properties(ns3::${ns3_lib} PROPERTIES
        IMPORTED_LOCATION ${NS3_LOCATION_${ns3_lib}}
        INTERFACE_INCLUDE_DIRECTORIES ${ns3_include_dir}
      )
      if(NS3_USE_BUILD_TREE)
        get_filename_component(ns3_libdir
          "${NS3_LOCATION_${ns3_lib}}" DIRECTORY)
        set_target_properties(ns3::${ns3_lib} PROPERTIES
          BUILD_RPATH ${ns3_libdir}
        )
      endif()
      message(-- " Created ns3::${ns3_lib} target")
      set(ns3_all 1)
    endif()
  endforeach()
endif()

# Add the general NS3 library
if(ns3_all)
  add_library(ns3 INTERFACE)
  set_target_properties(ns3 PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES ${ns3_include_dir}
  )
  foreach(ns3_lib ${ns3_libs})
    if(NS3_LOCATION_${ns3_lib})
      target_link_libraries(ns3 INTERFACE ns3::${ns3_lib})
    endif()
  endforeach()
  message(-- " Created ns3 target")
endif()

# vim:et:sw=2:ts=2
