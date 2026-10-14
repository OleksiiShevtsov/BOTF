# Resolve third-party root (LIB) and recommended build root (BUILD).
#
# Canonical layout (standalone BOTF repo):
#   <parent>/
#   ├── BOTF/     ← this repository (CMake source root)
#   ├── LIB/
#   └── BUILD/
#
# Legacy fallback (old WorkTraining nesting):
#   <parent>/WorkTraining/Junior/BattleOfTheFlatland/ → ../../../LIB
#
# Overrides:
#   -DBOTF_LIB_ROOT=...
#   env BOTF_LIB_ROOT
#   env CMAKE_BUILDS_ROOT  (same idea as CrashCourse presets)

function(_botf_dir_looks_like_lib dir)
	if(EXISTS "${dir}/SFML" OR EXISTS "${dir}/sfml" OR EXISTS "${dir}/README.md"
	   OR EXISTS "${dir}/googletest" OR EXISTS "${dir}/protobuf" OR EXISTS "${dir}/spdlog")
		set(_botf_dir_looks_like_lib_result TRUE PARENT_SCOPE)
	else()
		set(_botf_dir_looks_like_lib_result FALSE PARENT_SCOPE)
	endif()
endfunction()

if(NOT BOTF_LIB_ROOT AND DEFINED ENV{BOTF_LIB_ROOT} AND NOT "$ENV{BOTF_LIB_ROOT}" STREQUAL "")
	file(TO_CMAKE_PATH "$ENV{BOTF_LIB_ROOT}" BOTF_LIB_ROOT)
endif()

if(NOT BOTF_LIB_ROOT)
	get_filename_component(_botf_lib_sibling "${CMAKE_SOURCE_DIR}/../LIB" ABSOLUTE)
	get_filename_component(_botf_lib_legacy "${CMAKE_SOURCE_DIR}/../../../LIB" ABSOLUTE)

	_botf_dir_looks_like_lib("${_botf_lib_sibling}")
	if(_botf_dir_looks_like_lib_result)
		set(_botf_lib_resolved "${_botf_lib_sibling}")
	else()
		_botf_dir_looks_like_lib("${_botf_lib_legacy}")
		if(_botf_dir_looks_like_lib_result)
			set(_botf_lib_resolved "${_botf_lib_legacy}")
		else()
			# Prefer sibling of this repo; setup.bat / setup.sh create it.
			set(_botf_lib_resolved "${_botf_lib_sibling}")
		endif()
	endif()

	set(BOTF_LIB_ROOT "${_botf_lib_resolved}" CACHE PATH
		"Third-party libraries directory (sibling of BOTF)")
endif()

# Recommended out-of-tree build root (do not build inside the project folder).
if(NOT BOTF_BUILD_ROOT)
	if(DEFINED ENV{CMAKE_BUILDS_ROOT} AND NOT "$ENV{CMAKE_BUILDS_ROOT}" STREQUAL "")
		file(TO_CMAKE_PATH "$ENV{CMAKE_BUILDS_ROOT}" _botf_build_resolved)
	else()
		get_filename_component(_botf_build_sibling "${CMAKE_SOURCE_DIR}/../BUILD" ABSOLUTE)
		get_filename_component(_botf_build_legacy "${CMAKE_SOURCE_DIR}/../../../BUILD" ABSOLUTE)
		get_filename_component(_botf_lib_parent "${BOTF_LIB_ROOT}/.." ABSOLUTE)
		set(_botf_build_beside_lib "${_botf_lib_parent}/BUILD")

		if(EXISTS "${_botf_build_beside_lib}")
			set(_botf_build_resolved "${_botf_build_beside_lib}")
		elseif(EXISTS "${_botf_build_sibling}")
			set(_botf_build_resolved "${_botf_build_sibling}")
		elseif(EXISTS "${_botf_build_legacy}")
			set(_botf_build_resolved "${_botf_build_legacy}")
		else()
			set(_botf_build_resolved "${_botf_build_beside_lib}")
		endif()
	endif()

	set(BOTF_BUILD_ROOT "${_botf_build_resolved}" CACHE PATH
		"Out-of-tree build root (sibling of LIB / BOTF)")
endif()

message(STATUS "[botf] BOTF_LIB_ROOT=${BOTF_LIB_ROOT}")
message(STATUS "[botf] BOTF_BUILD_ROOT=${BOTF_BUILD_ROOT}  (from CMAKE_BUILDS_ROOT or sibling BUILD)")
message(STATUS "[botf] Prefer CMake preset debug/release: binaryDir = \$env{CMAKE_BUILDS_ROOT}/\${sourceDirName}/...")
