cmake_minimum_required(VERSION 3.30)
project(so_project)

set(CMAKE_CXX_STANDARD 20)

add_executable(so_project main.c
        scheduler.c
        shared_memory.c
        shared_memory.h
        process_manager.c
        process_manager.h)
