
set (GLTF_MAJOR_VERSION 0)
set (GLTF_MINOR_VERSION 1)
set (GLTF_PATCH_VERSION 0)
set (GLTF_VERSION ${GLTF_MAJOR_VERSION}.${GLTF_MINOR_VERSION}.${GLTF_PATCH_VERSION})

# glTF executable
file (GLOB_RECURSE glTF-src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
include_directories (
	${FBX_SDK_INCLUDES}
	/usr/local/include
	../IO-glTF
)
link_directories (
	${FBX_SDK_LIBS}
	/usr/local/lib
)
add_executable (glTF ${glTF-src})
target_link_libraries (
	glTF
	${FBX_SDK_LIBRARY}
	${CASABLANCA_LIBRARY}
	${Boost_LIBRARIES} ${Boost_FRAMEWORK}
)

