function(apply_can_stack_patch source_dir patch_file)
  if(NOT EXISTS "${patch_file}")
    message(FATAL_ERROR "CAN stack patch file not found: ${patch_file}")
  endif()

  execute_process(
    COMMAND git apply --check "${patch_file}"
    WORKING_DIRECTORY "${source_dir}"
    RESULT_VARIABLE patch_check_result
    OUTPUT_QUIET
    ERROR_QUIET)

  if(patch_check_result EQUAL 0)
    message(STATUS "Applying local CAN stack compatibility patch")
    execute_process(
      COMMAND git apply "${patch_file}"
      WORKING_DIRECTORY "${source_dir}"
      RESULT_VARIABLE patch_apply_result)

    if(NOT patch_apply_result EQUAL 0)
      message(FATAL_ERROR "Failed to apply CAN stack compatibility patch")
    endif()
    return()
  endif()

  execute_process(
    COMMAND git apply --reverse --check "${patch_file}"
    WORKING_DIRECTORY "${source_dir}"
    RESULT_VARIABLE reverse_check_result
    OUTPUT_QUIET
    ERROR_QUIET)

  if(reverse_check_result EQUAL 0)
    message(STATUS "CAN stack compatibility patch already applied")
    return()
  endif()

  message(FATAL_ERROR "CAN stack compatibility patch cannot be applied cleanly")
endfunction()
