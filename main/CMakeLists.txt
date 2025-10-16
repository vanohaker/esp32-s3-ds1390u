set(SOURCES
    main.c
    button.c
    display.c
    menu.c
    i2cscan.c
)

idf_component_register(SRCS  ${SOURCES}
                    INCLUDE_DIRS "."
                    REQUIRES driver u8g2 esp_timer)
