cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project ("glTF")

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
	set (LIBRARY_EXT dylib)
elseif ( UNIX )
	set (LIBRARY_EXT so)
else ()
	set (LIBRARY_EXT lib)
endif ()

string (TOLOWER ${CMAKE_BUILD_TYPE} glTF_BUILD_TYPE)

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
	set (CASABLANCA_LIBRARY libcpprest.2.4.${LIBRARY_EXT})
elseif ( UNIX )
	set (CASABLANCA_LIBRARY libcpprest.${LIBRARY_EXT})
else ()
	set (CASABLANCA_LIBRARY libcpprest.2.4.${LIBRARY_EXT})
endif ()
message (STATUS "Using Casablanca: " ${CASABLANCA})

set (FBX_SDK_INCLUDES ${FBX_SDK}/include)
if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
	set (FBX_SDK_LIBS ${FBX_SDK}/lib/clang/${glTF_BUILD_TYPE})
elseif ( UNIX )
	set (FBX_SDK_LIBS ${FBX_SDK}/lib/gcc4/x64/${glTF_BUILD_TYPE})
else ()
	set (FBX_SDK_LIBS ${FBX_SDK}/lib/vs2013/x64/${glTF_BUILD_TYPE})
endif ()
set (FBX_SDK_LIBRARY libfbxsdk.${LIBRARY_EXT})
message (STATUS "Using FBX: " ${FBX_SDK})

# Reconfigure final output directory
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_BUILD_TYPE})
message (STATUS "Ouput Directory: " ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Platform (not compiler) specific settings
if ( WIN32 ) # Check if we are on Windows
	if ( MSVC ) # Check if we are using the Visual Studio compiler
		set_target_properties (FileMenu PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
	endif ()
	option (BUILD_SHARED_LIBS "Build shared Libraries." ON)
	add_definitions (-DUNICODE -D_WIN64 -D_GLTF_EXPORTS_ -D_WINDOWS -D_USRDLL)
elseif ( UNIX ) # This includes OSX
	option (BUILD_SHARED_LIBS "Build shared Libraries." ON)
	find_package (Boost REQUIRED COMPONENTS random chrono system thread locale regex filesystem)
else ()
	message (FATAL_ERROR "-- Unsupported Build Platform.")
endif ()

# Compiler (not platform) specific settings
if ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" )
	message ("-- Setting clang options")

	set (WARNINGS "-Wall -Wextra -Wcast-qual -Wconversion -Wformat=2 -Winit-self -Winvalid-pch -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked -Wredundant-decls")
	set (OSX_SUPPRESSIONS "-Wno-overloaded-virtual -Wno-sign-conversion -Wno-deprecated -Wno-unknown-pragmas -Wno-reorder -Wno-char-subscripts -Wno-switch -Wno-unused-parameter -Wno-unused-variable -Wno-deprecated -Wno-unused-value -Wno-unknown-warning-option -Wno-return-type-c-linkage -Wno-unused-function -Wno-sign-compare -Wno-shorten-64-to-32 -Wno-reorder")
	set (WARNINGS "${WARNINGS} ${OSX_SUPPRESSIONS}")

	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-return-type-c-linkage -Wno-unneeded-internal-declaration")
	set (CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
	set (CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")

	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing")
elseif ( "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" )
	message ("-- Setting gcc options")

	set (WARNINGS "-Wall -Wextra -Wunused-parameter -Wcast-align -Wcast-qual -Wconversion -Wformat=2 -Winit-self -Winvalid-pch -Wmissing-format-attribute -Wmissing-include-dirs -Wpacked -Wredundant-decls -Wunreachable-code")
	set (LINUX_SUPPRESSIONS "-Wno-deprecated -Wno-unknown-pragmas -Wno-reorder -Wno-unused-function -Wno-char-subscripts -Wno-switch -Wno-unused-but-set-parameter -Wno-unused-value -Wno-unused-local-typedefs")

	set (WARNINGS "${WARNINGS} ${LINUX_SUPPRESSIONS}")
	set (LD_FLAGS "${LD_FLAGS} -Wl,-z,defs")

	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-strict-aliasing")
	set (LD_LIBRARY_PATH ${CASABLANCA_LIBS})
else ()
	message ("-- Unknown compiler, success is doubtful.")
endif ()


add_subdirectory (IO-glTF)
add_subdirectory (glTF)

