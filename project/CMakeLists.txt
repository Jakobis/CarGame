# Add executables (to different projects)
add_executable(SRE-node-editor node-editor.cpp)
add_executable(SRE-box2d-example box2d-example.cpp)
add_executable(SRE-box2d-platform-example box2d-platform-example.cpp)
add_executable(SRE-project project.cpp)
add_executable(SRE-sdl2_mixer-example sdl2_mixer-example.cpp)
add_executable(SRE-lua-example lua-example.cpp)
add_executable(SRE-lua-example-lines lua-example-lines.cpp)
add_executable(SRE-scene-graph scene-graph.cpp)
IF (USE_BULLET)
add_executable(SRE-bullet-example bullet-example.cpp)
ENDIF(USE_BULLET)
IF (USE_assimp)
add_executable(SRE-assimp assimp.cpp)
ENDIF(USE_assimp)


# Link projects with project libraries
target_link_libraries(SRE-node-editor ${all_libs})
target_link_libraries(SRE-box2d-example ${all_libs})
target_link_libraries(SRE-box2d-platform-example ${all_libs})
target_link_libraries(SRE-project ${all_libs})
target_link_libraries(SRE-sdl2_mixer-example ${all_libs})
target_link_libraries(SRE-scene-graph ${all_libs})
IF (USE_BULLET)
target_link_libraries(SRE-bullet-example ${all_libs})
ENDIF (USE_BULLET)
IF (USE_assimp)
	target_link_libraries(SRE-assimp ${all_libs})
ENDIF(USE_assimp)
if (UNIX)
target_link_libraries(SRE-lua-example ${all_libs} -ldl)
target_link_libraries(SRE-lua-example-lines ${all_libs}  -ldl)
else (UNIX)
target_link_libraries(SRE-lua-example ${all_libs})
target_link_libraries(SRE-lua-example-lines ${all_libs})
endif(UNIX)

# Copy project data to output path
file(COPY ../project_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
file(COPY ../project_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
file(COPY ../project_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})


IF (WIN32)
	## Copy dll's to output path
	file(COPY ${DLLFileList} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
	file(COPY ${DLLFileList} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Release)
ENDIF(WIN32)

add_subdirectory(Box2DTestbed)
add_subdirectory(platformer)
add_subdirectory(particle_system)
add_subdirectory(gui)
IF (USE_BULLET)
	add_subdirectory(3D_Scenegraph)
ENDIF(USE_BULLET)
add_subdirectory(Car)
# add_subdirectory(BussIK)