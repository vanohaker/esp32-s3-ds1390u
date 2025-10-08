set(SOURCES
    main.c
    button.c
    display.c
    menu.c
)

idf_component_register(SRCS  ${SOURCES}
                    INCLUDE_DIRS "."
                    REQUIRES driver u8g2 esp_timer)
