add_library(
    scha6xx
    scha63x-runner.c
    scha63x-runner.h
    scha63x_spi.c
    scha63x_spi.h
    scha63x_spi_frame.c
    scha63x_spi_frame.h
    scha63x_driver.c
    scha63x_driver.h
    )

target_include_directories(
    scha6xx PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
    )

target_link_libraries(scha6xx pico_stdlib hardware_spi)
