# Conserva a afinação do utilizador nas compilações seguintes.
if(NOT DEFINED SOURCE OR NOT DEFINED DESTINATION)
    message(FATAL_ERROR "Visual config copy requires SOURCE and DESTINATION")
endif()
if(NOT EXISTS "${DESTINATION}")
    file(COPY_FILE "${SOURCE}" "${DESTINATION}" ONLY_IF_DIFFERENT)
endif()
