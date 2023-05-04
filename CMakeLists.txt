cmake_minimum_required(VERSION 3.12)

project(dns-checker)

function(cpp17 target)
    set_target_properties(${target}
        PROPERTIES
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF
    )

    target_compile_options(${target}
        PRIVATE
            -Wall -Wextra -Wconversion -pedantic -Wno-sign-conversion
    )
endfunction()

add_executable(dns-checker src/main.cpp)
cpp17(dns-checker)

if(WIN32)
    target_link_libraries(dns-checker ws2_32)
endif()
