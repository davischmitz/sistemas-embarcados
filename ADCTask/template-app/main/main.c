#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "driver/ledc.h"
#include "esp_err.h"

#define PINO_LED 2

#define DEFAULT_VREF 1100 // Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES 50  // Multisampling

SemaphoreHandle_t sinaleira = NULL;

// Configuração do ADC
static const adc_channel_t channel = ADC_CHANNEL_6; // GPIO34 if ADC1, GPIO14 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
static const adc_atten_t atten = ADC_ATTEN_DB_11; // ver https://docs.espressif.com/projects/esp-idf/en/v4.2/esp32/api-reference/peripherals/adc.html#_CPPv425adc1_config_channel_atten14adc1_channel_t11adc_atten_t

// Configuração do LED
uint32_t tempo_delay = 1000; // Tempo de delay em ms
bool estado_led = 0;

// LEDC
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO (2) // Define the output GPIO
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_FREQUENCY (5000)           // Frequency in Hertz. Set frequency at 5 kHz

// Inicialização do PWM do LED
static void example_ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY, // Set output frequency at 5 kHz
        .clk_cfg = LEDC_AUTO_CLK};
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = LEDC_OUTPUT_IO,
        .duty = 0, // Set duty to 0%
        .hpoint = 0};
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void tarefaADC(void *parametros)
{
    // Configure ADC
    adc1_config_width(width);
    adc1_config_channel_atten(channel, atten);
    for (;;)
    {
        uint32_t adc_reading = 0;
        // Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++)
        {
            adc_reading += adc1_get_raw((adc1_channel_t)channel);
        }
        adc_reading /= NO_OF_SAMPLES;
        printf("ADC: %d\n", adc_reading);

        if (sinaleira != NULL)
        {
            if (xSemaphoreTake(sinaleira, 10))
            {
                tempo_delay = adc_reading;
                xSemaphoreGive(sinaleira);
            }
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

void tarefaLED(void *parametros)
{
    uint32_t tempo_led = 0;
    example_ledc_init();

    for (;;)
    {
        estado_led = !estado_led;
        gpio_set_level(PINO_LED, estado_led);

        if (sinaleira != NULL)
        {
            if (xSemaphoreTake(sinaleira, 10))
            {
                tempo_led = tempo_delay;
                xSemaphoreGive(sinaleira);
            }
        }
        ESP_ERROR_CHECK(ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, tempo_led * 2));
        ESP_ERROR_CHECK(ledc_update_duty(LEDC_MODE, LEDC_CHANNEL));
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    sinaleira = xSemaphoreCreateBinary();
    xSemaphoreGive(sinaleira);

    xTaskCreate(tarefaLED, "led", 2048, NULL, 0, NULL);
    xTaskCreate(tarefaADC, "adc", 2048, NULL, 0, NULL);
}