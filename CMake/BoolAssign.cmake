include_guard(DIRECTORY)

# Macro for setting value of the boolean variable to some expression
#   VAR = name of the variable (followed by boolean expression)
macro(set_bool VAR)
    if(${ARGN})
        set(${VAR} TRUE)
    else()
        set(${VAR} FALSE)
    endif()
endmacro()