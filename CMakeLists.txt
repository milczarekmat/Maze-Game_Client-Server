cmake_minimum_required(VERSION 3.20)
project(GRA_PROJEKT C)

set(CMAKE_C_STANDARD 99)

add_executable(GRA_PROJEKT server.c server_defs.c server_defs.h server_threads.c server_threads.h)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries(GRA_PROJEKT Threads::Threads)
target_link_libraries(GRA_PROJEKT ncurses)
