# Add as a subdirectory SimpleRenderEngineProject using add_subdirectory
IF (NOT DEFINED all_libs)
    message( FATAL_ERROR "This CMakeLists.txt must be included using the command add_subdirectory(name)" )
ENDIF (NOT DEFINED all_libs)
# Ensure SimpleRenderEngineProject is up-to-date
IF (WIN32)
    IF (NOT DEFINED DLLFileList)
        message( FATAL_ERROR "The SimpleRenderEngineProject has been updated. Clone the project (https://github.com/mortennobel/SimpleRenderEngineProject) again the project again and start over." )
    ENDIF(NOT DEFINED DLLFileList)
ENDIF(WIN32)

# Add hpp and cpp files to Snake variable
file(GLOB Car
        "*.hpp"
        "*.h"
        "*.cpp"
        )

# Compile cpp files (from Snake variable)
add_executable(Car-Game ${Car})
target_link_libraries(Car-Game ${all_libs})
set(rawFiles car.json car.png assets)

# copy files to dest
file(COPY ${rawFiles} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
file(COPY ${rawFiles} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
file(COPY ${rawFiles} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

IF (WIN32)
    # Copy dll files to ${CMAKE_CURRENT_BINARY_DIR}/Debug
    file(COPY ${DLLFileList} DESTINATION Debug)
    file(COPY ${DLLFileList} DESTINATION Release)
    file(COPY ${DLLFileList} DESTINATION .)

    # Set working directory to ${CMAKE_CURRENT_BINARY_DIR}/Debug
    set_target_properties(Car-Game PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Debug)
ENDIF(WIN32)
