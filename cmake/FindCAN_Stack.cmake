if(NOT TARGET isobus::isobus)
  include(FetchContent)
  FetchContent_Declare(
    CAN_Stack
    GIT_REPOSITORY https://github.com/Open-Agriculture/AgIsoStack-plus-plus.git
    GIT_TAG 0c5e3d2f264270f0a750cc3fdedaaa5861d71a59)
  FetchContent_MakeAvailable(CAN_Stack)
  include(${CMAKE_CURRENT_LIST_DIR}/ApplyCANStackPatch.cmake)
  apply_can_stack_patch(${can_stack_SOURCE_DIR}
                        ${CMAKE_CURRENT_LIST_DIR}/../patches/can_stack_vt_server_compat.patch)
endif()
