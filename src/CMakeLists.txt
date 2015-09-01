project(ChartForge)
cmake_minimum_required(VERSION 2.8)

add_definitions("-Wall -std=c++14 -DUNICODE")
set(CMAKE_EXE_LINKER_FLAGS	${CMAKE_EXE_LINKER_FLAGS} -Wl,--subsystem,windows)

add_subdirectory(debugger)

add_executable(chartforge
	main.cpp
	appContext.cpp
)
target_link_libraries(chartforge debugger)

# なぜかこれがないとダメ
target_link_libraries(chartforge libstdc++)