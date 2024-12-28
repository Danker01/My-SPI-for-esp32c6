#include <stdint.h>
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "hal/gpio_types.h"
#include "esp_log.h"
#include "portmacro.h"

uint8_t rx; 
#define SPI_MOSI_GPIO 3
#define SPI_MISO_GPIO 2
#define SPI_CLK_GPIO 0
#define SPI_CS_GPIO 1

void spi_init() {
    gpio_set_direction(SPI_MOSI_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(SPI_MISO_GPIO, GPIO_MODE_INPUT);
    gpio_set_direction(SPI_CLK_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_direction(SPI_CS_GPIO, GPIO_MODE_OUTPUT);
    
    gpio_set_level(SPI_CS_GPIO, 1);
}

void spi_send(uint8_t registr, uint8_t data) {
	uint16_t regD = data | (registr << 8);
    gpio_set_level(SPI_CS_GPIO, 0);
    for (int i = 0; i < 32; i++) {
        if (i%2==0) {
			gpio_set_level(SPI_CLK_GPIO, 1);
		}else{
        	gpio_set_level(SPI_MOSI_GPIO, (regD & (1 << (15 - i/2))) ? 1 : 0);
        	gpio_set_level(SPI_CLK_GPIO, 0);
        }
	}
    gpio_set_level(SPI_CS_GPIO, 1);
}

uint8_t spi_read(uint8_t reg) {
	uint8_t ret = 0x00;
    gpio_set_level(SPI_CS_GPIO, 0);
    for (int i = 0; i < 32; i++) {
        if (i%2==0) {
			gpio_set_level(SPI_CLK_GPIO, 1);
		}else{
			if (i<16){
	        	gpio_set_level(SPI_MOSI_GPIO, (reg & (1 << (7 - i/2))) ? 1 : 0);
        		gpio_set_level(SPI_CLK_GPIO, 0);
        	}else{
				gpio_set_level(SPI_MOSI_GPIO, 1);
				ret |= gpio_get_level(SPI_MISO_GPIO) << (7 - (i-16)/2);
				gpio_set_level(SPI_CLK_GPIO, 0);
			}

        }
	}
    gpio_set_level(SPI_CS_GPIO, 1); 
    return ret;
}

void app_main() {
    spi_init();
    
    vTaskDelay(1000/portTICK_PERIOD_MS);
	spi_send(0x80, 0x80);
	vTaskDelay(1000/portTICK_PERIOD_MS);
	ESP_LOGI("SPI_READ", "ret: %u",spi_read(0x00));
}
