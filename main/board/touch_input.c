#include "touch_input.h"

#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TOUCH_I2C_PORT I2C_NUM_0
#define TOUCH_SDA_GPIO GPIO_NUM_8
#define TOUCH_SCL_GPIO GPIO_NUM_9
#define TOUCH_INT_GPIO GPIO_NUM_4
#define TOUCH_ADDRESS 0x5D
#define TOUCH_STATUS_REGISTER 0x814E
#define TOUCH_MAX_X 1023
#define TOUCH_MAX_Y 599

static i2c_master_dev_handle_t touch_device;
static uint16_t last_x;
static uint16_t last_y;

static esp_err_t touch_read_register(uint16_t reg, uint8_t *data, size_t length)
{
    uint8_t address[2] = {(uint8_t)(reg >> 8), (uint8_t)(reg & 0xff)};
    return i2c_master_transmit_receive(touch_device, address, sizeof(address), data, length, 100);
}

static esp_err_t touch_clear_status(void)
{
    const uint8_t command[] = {
        (uint8_t)(TOUCH_STATUS_REGISTER >> 8),
        (uint8_t)(TOUCH_STATUS_REGISTER & 0xff),
        0,
    };
    return i2c_master_transmit(touch_device, command, sizeof(command), 100);
}

esp_err_t touch_input_init(void)
{
    const i2c_master_bus_config_t bus_config = {
        .i2c_port = TOUCH_I2C_PORT,
        .sda_io_num = TOUCH_SDA_GPIO,
        .scl_io_num = TOUCH_SCL_GPIO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_config, &bus_handle), "touch", "I2C bus init failed");

    const i2c_device_config_t device_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = TOUCH_ADDRESS,
        .scl_speed_hz = 400000,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus_handle, &device_config, &touch_device),
                        "touch", "GT911 device init failed");

    gpio_config_t interrupt_config = {
        .pin_bit_mask = 1ULL << TOUCH_INT_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_RETURN_ON_ERROR(gpio_config(&interrupt_config), "touch", "GT911 interrupt pin init failed");
    vTaskDelay(pdMS_TO_TICKS(10));

    uint8_t status;
    ESP_RETURN_ON_ERROR(touch_read_register(TOUCH_STATUS_REGISTER, &status, 1),
                        "touch", "GT911 status read failed");
    return ESP_OK;
}

void touch_input_read(lv_indev_drv_t *driver, lv_indev_data_t *data)
{
    (void)driver;
    uint8_t point_data[9] = {0};

    data->point.x = last_x;
    data->point.y = last_y;
    data->state = LV_INDEV_STATE_REL;

    if (touch_device == NULL || touch_read_register(TOUCH_STATUS_REGISTER, point_data, sizeof(point_data)) != ESP_OK) {
        return;
    }

    const uint8_t status = point_data[0];
    const uint8_t point_count = status & 0x0f;
    if ((status & 0x80) == 0 || point_count == 0) {
        return;
    }

    last_x = (uint16_t)(point_data[1] | ((uint16_t)point_data[2] << 8));
    last_y = (uint16_t)(point_data[3] | ((uint16_t)point_data[4] << 8));
    if (last_x > TOUCH_MAX_X) {
        last_x = TOUCH_MAX_X;
    }
    if (last_y > TOUCH_MAX_Y) {
        last_y = TOUCH_MAX_Y;
    }

    data->point.x = last_x;
    data->point.y = last_y;
    data->state = LV_INDEV_STATE_PR;
    (void)touch_clear_status();
}
