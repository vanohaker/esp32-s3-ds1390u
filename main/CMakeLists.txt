set(SOURCES
    main.c
    button.c
    display.c
)

set(EXTRA_COMPONENT_DIRS ${CMAKE_SOURCE_DIR}/components/epaper-29-dke)

idf_component_register(SRCS  ${SOURCES}
                    INCLUDE_DIRS "."
                    REQUIRES driver epaper-29-dke esp_timer)
