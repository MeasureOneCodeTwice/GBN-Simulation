cmake_minimum_required(VERSION 3.23)

project(GbnApplications)

add_library(GbnApplications)
target_sources(GbnApplications
    PRIVATE
      utils.cc
      packet.cc
      receiver.cc
      sender.cc
    PUBLIC
      receiver.h
      sender.h
)
