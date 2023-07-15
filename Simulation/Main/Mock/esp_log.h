#ifndef _ESP_LOG_H_
#define _ESP_LOG_H_

#include <stdio.h>

// Define color codes for different log levels
#define LOG_COLOR_RESET "\x1B[0m"
#define LOG_COLOR_RED "\x1B[31m"
#define LOG_COLOR_ORANGE "\x1B[33m"
#define LOG_COLOR_BLUE "\x1B[34m"
#define LOG_COLOR_WHITE "\x1B[37m"

// Logging macros with color and formatting
#define ESP_LOGI(tag, format, ...) printf(LOG_COLOR_WHITE "[%s] " format LOG_COLOR_RESET "\n", tag, ##__VA_ARGS__)
#define ESP_LOGE(tag, format, ...) printf(LOG_COLOR_RED "[%s] " format LOG_COLOR_RESET "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...) printf(LOG_COLOR_ORANGE "[%s] " format LOG_COLOR_RESET "\n", tag, ##__VA_ARGS__)
#define ESP_LOGD(tag, format, ...) printf(LOG_COLOR_BLUE "[%s] " format LOG_COLOR_RESET "\n", tag, ##__VA_ARGS__)
#define ESP_LOGV(tag, format, ...) printf(LOG_COLOR_WHITE "[%s] " format LOG_COLOR_RESET "\n", tag, ##__VA_ARGS__)

#endif