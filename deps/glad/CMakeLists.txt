cmake_minimum_required(VERSION 3.16)
project(Glad)

add_library(glad STATIC include/glad/gl.h src/gl.c)