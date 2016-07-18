cmake_minimum_required(VERSION 2.8.11)

file(GLOB IPC_SRC *.cpp)
add_library(mio_ipc ${IPC_SRC})
target_link_libraries(mio_ipc pthread)

