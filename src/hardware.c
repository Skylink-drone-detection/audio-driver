#include "hardware.h"

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <gpiod.h>
#include <limits.h>
#include <linux/spi/spidev.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>

#define GPIO_REQUESTS_CAPACITY 256U
#define GPIO_SCAN_LIMIT 16
#define GPIO_CONSUMER "audio-driver"
#define SPI_MODE_DEFAULT SPI_MODE_0
#define SPI_BITS_PER_WORD 8
#define SPI_BASE_CLOCK_HZ 250000000U

static struct gpiod_chip *gpio_chip = NULL;
static char gpio_chip_path[PATH_MAX];
static struct gpiod_line_request *gpio_requests[GPIO_REQUESTS_CAPACITY];

static int spi_fd = -1;
static uint32_t spi_speed_hz = 0;

static bool string_contains_ignore_case(const char *haystack, const char *needle)
{
    if (!haystack || !needle) {
        return false;
    }

    const size_t needle_len = strlen(needle);
    if (needle_len == 0) {
        return true;
    }

    for (const char *p = haystack; *p != '\0'; ++p) {
        size_t matched = 0;
        while (matched < needle_len && p[matched] != '\0' &&
               (char)tolower((unsigned char)p[matched]) ==
                   (char)tolower((unsigned char)needle[matched])) {
            ++matched;
        }
        if (matched == needle_len) {
            return true;
        }
    }

    return false;
}

static int gpio_chip_score(const char *name, const char *label, size_t num_lines)
{
    int score = (int)num_lines;

    if (string_contains_ignore_case(name, "rp1") ||
        string_contains_ignore_case(label, "rp1")) {
        score += 1000;
    }

    if (string_contains_ignore_case(name, "gpio") ||
        string_contains_ignore_case(label, "gpio")) {
        score += 100;
    }

    return score;
}

static bool gpio_open_chip(const char *path)
{
    struct gpiod_chip *chip = gpiod_chip_open(path);
    if (!chip) {
        return false;
    }

    if (gpio_chip) {
        gpiod_chip_close(gpio_chip);
    }

    gpio_chip = chip;
    snprintf(gpio_chip_path, sizeof(gpio_chip_path), "%s", path);
    return true;
}

static bool gpio_select_chip(unsigned int required_offset)
{
    const char *env_path = getenv("AUDIO_DRIVER_GPIO_CHIP");
    if (env_path && env_path[0] != '\0') {
        if (!gpio_open_chip(env_path)) {
            fprintf(stderr, "Failed to open GPIO chip %s: %s\n",
                    env_path, strerror(errno));
            return false;
        }
        return true;
    }

    if (gpio_chip) {
        struct gpiod_chip_info *info = gpiod_chip_get_info(gpio_chip);
        if (info) {
            const size_t num_lines = gpiod_chip_info_get_num_lines(info);
            gpiod_chip_info_free(info);
            if (required_offset < num_lines) {
                return true;
            }
        }
    }

    int best_score = -1;
    char best_path[PATH_MAX] = {0};

    for (int i = 0; i < GPIO_SCAN_LIMIT; ++i) {
        char candidate[PATH_MAX];
        snprintf(candidate, sizeof(candidate), "/dev/gpiochip%d", i);

        struct gpiod_chip *chip = gpiod_chip_open(candidate);
        if (!chip) {
            continue;
        }

        struct gpiod_chip_info *info = gpiod_chip_get_info(chip);
        if (!info) {
            gpiod_chip_close(chip);
            continue;
        }

        const size_t num_lines = gpiod_chip_info_get_num_lines(info);
        if (required_offset < num_lines) {
            const char *name = gpiod_chip_info_get_name(info);
            const char *label = gpiod_chip_info_get_label(info);
            const int score = gpio_chip_score(name, label, num_lines);
            if (score > best_score) {
                best_score = score;
                snprintf(best_path, sizeof(best_path), "%s", candidate);
            }
        }

        gpiod_chip_info_free(info);
        gpiod_chip_close(chip);
    }

    if (best_score < 0) {
        fprintf(stderr,
                "No GPIO chip exposes line offset %u. Set AUDIO_DRIVER_GPIO_CHIP.\n",
                required_offset);
        return false;
    }

    if (!gpio_open_chip(best_path)) {
        fprintf(stderr, "Failed to open GPIO chip %s: %s\n",
                best_path, strerror(errno));
        return false;
    }

    return true;
}

static bool gpio_request_line_output(uint8_t gpio, uint8_t initial_value)
{
    if (gpio >= GPIO_REQUESTS_CAPACITY) {
        fprintf(stderr, "GPIO offset %u exceeds request capacity.\n", gpio);
        return false;
    }

    if (gpio_requests[gpio]) {
        return gpio_write_output(gpio, initial_value);
    }

    if (!gpio_select_chip(gpio)) {
        return false;
    }

    struct gpiod_line_settings *settings = gpiod_line_settings_new();
    struct gpiod_line_config *line_config = gpiod_line_config_new();
    struct gpiod_request_config *request_config = gpiod_request_config_new();
    if (!settings || !line_config || !request_config) {
        fprintf(stderr, "Failed to allocate GPIO request state.\n");
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(line_config);
        gpiod_request_config_free(request_config);
        return false;
    }

    gpiod_request_config_set_consumer(request_config, GPIO_CONSUMER);
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);
    gpiod_line_settings_set_output_value(
        settings,
        initial_value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);

    const unsigned int offsets[] = {gpio};
    if (gpiod_line_config_add_line_settings(line_config, offsets, 1, settings) < 0) {
        fprintf(stderr, "Failed to configure GPIO %u: %s\n", gpio, strerror(errno));
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(line_config);
        gpiod_request_config_free(request_config);
        return false;
    }

    struct gpiod_line_request *request =
        gpiod_chip_request_lines(gpio_chip, request_config, line_config);
    if (!request) {
        fprintf(stderr, "Failed to request GPIO %u on %s: %s\n",
                gpio, gpio_chip_path[0] ? gpio_chip_path : "<unknown>",
                strerror(errno));
        gpiod_line_settings_free(settings);
        gpiod_line_config_free(line_config);
        gpiod_request_config_free(request_config);
        return false;
    }

    gpio_requests[gpio] = request;

    gpiod_line_settings_free(settings);
    gpiod_line_config_free(line_config);
    gpiod_request_config_free(request_config);
    return true;
}

static void gpio_release_all(void)
{
    for (size_t i = 0; i < GPIO_REQUESTS_CAPACITY; ++i) {
        if (gpio_requests[i]) {
            gpiod_line_request_release(gpio_requests[i]);
            gpio_requests[i] = NULL;
        }
    }
}

static uint32_t spi_speed_from_divider(uint32_t clock_divider)
{
    if (clock_divider <= 1U) {
        return SPI_BASE_CLOCK_HZ;
    }

    return SPI_BASE_CLOCK_HZ / clock_divider;
}

static const char *spi_device_path_for_cs(uint8_t cs_gpio)
{
    const char *env_path = getenv("AUDIO_DRIVER_SPI_DEVICE");
    if (env_path && env_path[0] != '\0') {
        return env_path;
    }

    switch (cs_gpio) {
    case 8:
        return "/dev/spidev0.0";
    case 7:
        return "/dev/spidev0.1";
    default:
        fprintf(stderr,
                "No default spidev mapping for GPIO CS %u. Set AUDIO_DRIVER_SPI_DEVICE.\n",
                cs_gpio);
        return NULL;
    }
}

bool hardware_init(void)
{
    return true;
}

void hardware_cleanup(void)
{
    spi_close();
    gpio_release_all();

    if (gpio_chip) {
        gpiod_chip_close(gpio_chip);
        gpio_chip = NULL;
    }

    gpio_chip_path[0] = '\0';
}

bool gpio_request_output(uint8_t gpio, uint8_t initial_value)
{
    return gpio_request_line_output(gpio, initial_value);
}

bool gpio_write_output(uint8_t gpio, uint8_t value)
{
    if (gpio >= GPIO_REQUESTS_CAPACITY) {
        fprintf(stderr, "GPIO offset %u exceeds request capacity.\n", gpio);
        return false;
    }

    if (!gpio_requests[gpio] && !gpio_request_line_output(gpio, value)) {
        return false;
    }

    if (gpiod_line_request_set_value(gpio_requests[gpio], gpio,
                                     value ? GPIOD_LINE_VALUE_ACTIVE
                                           : GPIOD_LINE_VALUE_INACTIVE) < 0) {
        fprintf(stderr, "Failed to set GPIO %u: %s\n", gpio, strerror(errno));
        return false;
    }

    return true;
}

void gpio_release(uint8_t gpio)
{
    if (gpio >= GPIO_REQUESTS_CAPACITY || !gpio_requests[gpio]) {
        return;
    }

    gpiod_line_request_release(gpio_requests[gpio]);
    gpio_requests[gpio] = NULL;
}

void hardware_delay_ms(uint32_t milliseconds)
{
    const struct timespec delay = {
        .tv_sec = milliseconds / 1000U,
        .tv_nsec = (long)(milliseconds % 1000U) * 1000000L,
    };

    nanosleep(&delay, NULL);
}

void hardware_delay_us(uint32_t microseconds)
{
    const struct timespec delay = {
        .tv_sec = microseconds / 1000000U,
        .tv_nsec = (long)(microseconds % 1000000U) * 1000L,
    };

    nanosleep(&delay, NULL);
}

bool spi_open_for_cs(uint8_t cs_gpio, uint32_t clock_divider)
{
    const char *device_path = spi_device_path_for_cs(cs_gpio);
    if (!device_path) {
        return false;
    }

    spi_close();

    spi_fd = open(device_path, O_RDWR);
    if (spi_fd < 0) {
        fprintf(stderr, "Failed to open SPI device %s: %s\n",
                device_path, strerror(errno));
        return false;
    }

    uint8_t mode = SPI_MODE_DEFAULT;
    uint8_t bits_per_word = SPI_BITS_PER_WORD;
    spi_speed_hz = spi_speed_from_divider(clock_divider);

    fprintf(stdout,
            "SPI init: cs_gpio=%u device=%s target_speed=%uHz\n",
            cs_gpio, device_path, spi_speed_hz);

    if (ioctl(spi_fd, SPI_IOC_WR_MODE, &mode) < 0 ||
        ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits_per_word) < 0 ||
        ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed_hz) < 0) {
        fprintf(stderr, "Failed to configure SPI device %s: %s\n",
                device_path, strerror(errno));
        spi_close();
        return false;
    }

    fprintf(stdout,
            "SPI configured: mode=%u bits=%u speed=%uHz\n",
            (unsigned int)mode, (unsigned int)bits_per_word, spi_speed_hz);

    return true;
}

bool spi_transfer_bytes(const uint8_t *tx, uint8_t *rx, size_t len)
{
    if (spi_fd < 0) {
        fprintf(stderr, "SPI device is not initialized.\n");
        return false;
    }

    struct spi_ioc_transfer transfer = {
        .tx_buf = (uintptr_t)tx,
        .rx_buf = (uintptr_t)rx,
        .len = (uint32_t)len,
        .speed_hz = spi_speed_hz,
        .bits_per_word = SPI_BITS_PER_WORD,
    };

    if (ioctl(spi_fd, SPI_IOC_MESSAGE(1), &transfer) < 0) {
        fprintf(stderr, "SPI transfer failed: %s\n", strerror(errno));
        return false;
    }

    return true;
}

void spi_close(void)
{
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
        spi_speed_hz = 0;
    }
}
