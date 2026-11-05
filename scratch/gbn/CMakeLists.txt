cmake_minimum_required(VERSION 3.23)

project(GbnApplications)

add_library(GbnApplications)
target_sources(GbnApplications
    PRIVATE
      utils/sequence-number.cc
      utils/packet.cc
      receiver.cc
      sender.cc
    PUBLIC
      receiver.h
      sender.h
)
