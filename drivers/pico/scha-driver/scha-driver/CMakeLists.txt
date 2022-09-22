add_library(
    scha6xx
    scha63x-runner.cpp
    scha63x-runner.h
    scha63x_spi.c
    scha63x_spi.h
    scha63x_spi_frame.cpp
    scha63x_spi_frame.h
    scha63x_driver.cpp
    scha63x_driver.h
    )

target_include_directories(
    scha6xx PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(scha6xx pico_stdlib hardware_spi)
