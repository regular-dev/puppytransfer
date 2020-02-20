cmake_minimum_required(VERSION 3.7)

project(puppytransfer)
project(UDT)

set (CMAKE_CXX_STANDARD 17)

set (PUPPY_SRC
	app/main.cpp
        lib/flags.hpp
	app/UdpSocket.cpp
	app/UdpSocket.hpp
	app/UdtSocket.cpp
        app/UdtListener.cpp
        app/UdtListener.hpp
	app/UdtSocket.hpp
        app/IpAddress.cpp
        app/IpAddress.hpp
	)

set (UDT_SRC
	lib/udt/window.cpp
        lib/udt/queue.cpp
	lib/udt/packet.cpp
	lib/udt/md5.cpp
	lib/udt/list.cpp
	lib/udt/epoll.cpp
	lib/udt/core.cpp
	lib/udt/common.cpp
	lib/udt/channel.cpp
	lib/udt/ccc.cpp
	lib/udt/cache.cpp
	lib/udt/buffer.cpp
	lib/udt/api.cpp
	)

include_directories(. ../ lib/)


add_compile_options("-Wall" "-DLINUX" "-finline-functions" "-fno-strict-aliasing")

add_library(UDT STATIC ${UDT_SRC})
target_link_libraries(UDT m pthread)

# install sdl2lib-image via apt install
add_executable(puppytransfer ${PUPPY_SRC})
target_link_libraries(puppytransfer
			UDT)
install(TARGETS puppytransfer DESTINATION bin)
