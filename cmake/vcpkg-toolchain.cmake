# Locates a global vcpkg install and forwards to its toolchain file.
# Lookup order: -DVCPKG_ROOT_DIR=..., user-level VCPKG_ROOT from the Windows registry,
# $ENV{VCPKG_ROOT}, $ENV{VCPKG_INSTALLATION_ROOT}, vcpkg on PATH.
# The registry is checked first because Visual Studio overrides $ENV{VCPKG_ROOT}
# with its bundled vcpkg.

# Propagate a manual override into try_compile projects
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES VCPKG_ROOT_DIR)

if(NOT VCPKG_ROOT_DIR)
	set(_vcpkg_candidates "")

	if(CMAKE_HOST_WIN32 AND CMAKE_VERSION VERSION_GREATER_EQUAL 3.24)
		cmake_host_system_information(RESULT _vcpkg_user_root
			QUERY WINDOWS_REGISTRY "HKCU/Environment" VALUE VCPKG_ROOT)
		list(APPEND _vcpkg_candidates "${_vcpkg_user_root}")
	endif()

	list(APPEND _vcpkg_candidates "$ENV{VCPKG_ROOT}" "$ENV{VCPKG_INSTALLATION_ROOT}")

	find_program(_vcpkg_exe vcpkg NO_CACHE)
	if(_vcpkg_exe)
		get_filename_component(_vcpkg_exe_dir "${_vcpkg_exe}" DIRECTORY)
		list(APPEND _vcpkg_candidates "${_vcpkg_exe_dir}")
	endif()

	foreach(_candidate IN LISTS _vcpkg_candidates)
		if(_candidate AND EXISTS "${_candidate}/scripts/buildsystems/vcpkg.cmake")
			file(TO_CMAKE_PATH "${_candidate}" _candidate)
			set(VCPKG_ROOT_DIR "${_candidate}" CACHE PATH "Root of the vcpkg installation")
			break()
		endif()
	endforeach()
endif()

if(NOT VCPKG_ROOT_DIR OR NOT EXISTS "${VCPKG_ROOT_DIR}/scripts/buildsystems/vcpkg.cmake")
	message(FATAL_ERROR
		"Could not find a vcpkg installation.\n"
		"Install vcpkg once and point to it:\n"
		"  git clone https://github.com/microsoft/vcpkg\n"
		"  vcpkg\\bootstrap-vcpkg.bat   (or ./vcpkg/bootstrap-vcpkg.sh)\n"
		"then either set the VCPKG_ROOT environment variable to that folder,\n"
		"or pass -DVCPKG_ROOT_DIR=<path> when configuring.")
endif()

if(NOT _vcpkg_toolchain_reported)
	set(_vcpkg_toolchain_reported TRUE)
	message(STATUS "Using vcpkg at ${VCPKG_ROOT_DIR}")
endif()

include("${VCPKG_ROOT_DIR}/scripts/buildsystems/vcpkg.cmake")
