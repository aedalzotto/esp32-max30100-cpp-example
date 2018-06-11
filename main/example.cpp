/**
 * @file example.cpp
 * 
 * @author
 * Angelo Elias Dalzotto (150633@upf.br)
 * GEPID - Grupo de Pesquisa em Cultura Digital (gepid.upf.br)
 * Universidade de Passo Fundo (upf.br)
 * 
 * @copyright
 * Copyright 2018 Angelo Elias Dalzotto & Gabriel Boni Vicari
 * 
 * @brief This is an example file to the MAX30100 C++ library for the ESP32.
 * It initializes the I2C driver and then initializes the sensor.
 * One task is created to update the reading at the rate of 100Hz and then
 * prints the bpm and oxigen saturation values detected.
 */

/**
 * Pin assignment:
 * - i2c:
 *    GPIO12: SDA
 *    GPIO14: SDL
 * - no need to add external pull-up resistors.
 */

#include <freertos/FreeRTOS.h>
#include <driver/i2c.h>
#include "max30100.h"

#define I2C_SDA 12
#define I2C_SCL 14
#define I2C_FRQ 100000
#define I2C_PORT I2C_NUM_0

esp_err_t i2c_master_init(i2c_port_t i2c_port)
{
    i2c_config_t conf;
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = (gpio_num_t)I2C_SDA;
    conf.scl_io_num = (gpio_num_t)I2C_SCL;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = I2C_FRQ;
    i2c_param_config(i2c_port, &conf);
    return i2c_driver_install(i2c_port, I2C_MODE_MASTER, 0, 0, 0);
}

void get_bpm(void *max30100)
{
    while(true){
        try {
            ((Max30100::Device*)max30100)->update();
            ESP_LOGI( ( (Max30100::Device*)max30100 )->get_tag(), "BPM: %lf ", ((Max30100::Device*)max30100)->get_bpm());
            ESP_LOGI( ( (Max30100::Device*)max30100 )->get_tag(), "SPO2: %lf\n", ((Max30100::Device*)max30100)->get_spo2());
        } catch(I2CExcept::CommandFailed &ex){
            ESP_LOGE(( (Max30100::Device*)max30100 )->get_tag(), "%s", ex.what());
            continue;
        }
        vTaskDelay(10/portTICK_PERIOD_MS);
    }
}

extern "C" void app_main()
{
    ESP_ERROR_CHECK(i2c_master_init(I2C_PORT));
    static Max30100::Device max30100(I2C_PORT);
    
    try {
        max30100.init();
        xTaskCreate(get_bpm, "Get BPM", 8192, &max30100, 1, NULL);
    } catch(std::exception &ex){
        ESP_LOGE(max30100.get_tag(), "%s", ex.what());
    }
}