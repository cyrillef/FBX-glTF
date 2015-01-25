
set (GLTF_MAJOR_VERSION 0)
set (GLTF_MINOR_VERSION 1)
set (GLTF_PATCH_VERSION 0)
set (GLTF_VERSION ${GLTF_MAJOR_VERSION}.${GLTF_MINOR_VERSION}.${GLTF_PATCH_VERSION})

# glTF executable
file (GLOB_RECURSE glTF-src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
include_directories (
	${CASABLANCA_INCLUDES}
	${FBX_SDK_INCLUDES}
	/usr/local/include
	../IO-glTF
)
link_directories (
	${CASABLANCA_LIBS}
	${FBX_SDK_LIBS}
)
add_executable (glTF ${glTF-src})
target_link_libraries (
	glTF
	${FBX_SDK_LIBRARY}
	${CASABLANCA_LIBS}/${CASABLANCA_LIBRARY}
	${Boost_LIBRARIES} ${Boost_FRAMEWORK}
)

