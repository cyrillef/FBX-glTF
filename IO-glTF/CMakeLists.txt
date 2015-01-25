
set (IOGLTF_MAJOR_VERSION 0)
set (IOGLTF_MINOR_VERSION 1)
set (IOGLTF_PATCH_VERSION 0)
set (IOGLTF_VERSION ${IOGLTF_MAJOR_VERSION}.${IOGLTF_MINOR_VERSION}.${IOGLTF_PATCH_VERSION})

# IO-glTF dynamic libray
file (GLOB_RECURSE IO-glTF-src ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
include_directories (
	${FBX_SDK_INCLUDES}
	/usr/local/include
)
link_directories (
	${FBX_SDK_LIBS}
	/usr/local/lib
)
add_library (IO-glTF SHARED ${IO-glTF-src})
target_link_libraries (
	IO-glTF
	${FBX_SDK_LIBRARY}
	${CASABLANCA_LIBRARY}
	${Boost_LIBRARIES} ${Boost_FRAMEWORK}
)

# Support Files
file (
	COPY ${CMAKE_CURRENT_SOURCE_DIR}/glTF-0-8-defaults.json
	DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
)
file (
	COPY ${CMAKE_CURRENT_SOURCE_DIR}/glTF-0-8.json
	DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/
)

