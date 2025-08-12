set(req esp_driver_gpio esp_timer freertos)

idf_component_register(
        SRCS "hcsr04_driver.c"
        INCLUDE_DIRS "include"
        REQUIRES esp_driver_gpio esp_timer freertos)
