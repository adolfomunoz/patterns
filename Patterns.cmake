function(add_plugin target)
    add_library(${target} SHARED ${ARGN})
    if(WIN32 AND MINGW)
        set_target_properties(${target} PROPERTIES PREFIX "")
    endif()
endfunction()

function(add_plugin_on_folder target folder)
    add_plugin(${target} ${ARGN})
    set_target_properties(${target}
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${folder}"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${folder}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${folder}"
  )
endfunction()