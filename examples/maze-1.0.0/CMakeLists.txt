project(maze-1.0.0)
add_executable(${PROJECT_NAME} main.cpp model.cpp openglwindow.cpp
                               camera.cpp)
enable_abcg(${PROJECT_NAME})
