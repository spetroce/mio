cmake_minimum_required(VERSION 2.8.11)

file(GLOB IPC_SERVER_TYPE_SRC lcm_types/*.c)
add_library(mio_ipcserver ipcServer.cpp ${IPC_SERVER_TYPE_SRC})
target_link_libraries(mio_ipcserver mio_ipc ${LCM_LIBRARIES})

add_executable(ipc_server_main main.cpp)
target_link_libraries(ipc_server_main mio_ipcserver mio_ipc ${LCM_LIBRARIES})

