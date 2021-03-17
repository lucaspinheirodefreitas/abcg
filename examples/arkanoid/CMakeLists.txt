project(arkanoid)
add_executable(${PROJECT_NAME} main.cpp openglwindow.cpp paddle.cpp ball.cpp rectangle.cpp)
enable_abcg(${PROJECT_NAME})