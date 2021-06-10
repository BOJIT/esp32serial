/**
 * @file
 * @brief headers for esp32serial library
 *
 * @author @htmlonly &copy; @endhtmlonly 2021 James Bennion-Pedley
 *
 * @date 09 June 2021
 */


#ifndef __ESP32SERIAL__
#define __ESP32SERIAL__

# include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/*-------------------------------- Public Macros -----------------------------*/


/** @brief Debug Levels */
#define DEBUG_OFF       0
#define DEBUG_ERRORS    1
#define DEBUG_WARNINGS  2
#define DEBUG_INFO      3

/** @brief Debug Level Colours (overrideable) */
#ifndef DEBUG_COLOUR_INFO
    #define DEBUG_COLOUR_INFO "32"
#endif
#ifndef DEBUG_COLOUR_WARNINGS
    #define DEBUG_COLOUR_WARNINGS "33"
#endif
#ifndef DEBUG_COLOUR_ERRORS
    #define DEBUG_COLOUR_ERRORS "31"
#endif

/** @brief Set defaults (can be overridden by platformio.ini file) */
#ifndef DEBUG_LEVEL
    #define DEBUG_LEVEL DEBUG_INFO
#endif
#ifndef DEBUG_DELIMITER
    #define DEBUG_DELIMITER " - "
#endif
#ifndef DEBUG_COLOUR
    #define DEBUG_COLOUR 1
#endif


/*----------------------------- Class Declaration ----------------------------*/


class Esp32Serial {
private:
    QueueHandle_t _debugQueue;

    static void _debugWrapper(void* q) {
        static_cast<Esp32Serial*>(q)->_debugHandler(q);
    }

    void _debugHandler(void *q);

    int _debugSend(int type, char colour[2], const char* format, va_list args);

public:
    TaskHandle_t debugTask;

    Esp32Serial(const uint baud, size_t queue_length = 10, bool colour = false);
    ~Esp32Serial(void);

    int info(const char* format, ...);
    int warning(const char* format, ...);
    int error(const char* format, ...);
};

/* External declaration - optional, but is convenientway to log in >1 c files */
extern Esp32Serial esp32serial;

#endif /* __ESP32SERIAL__ */
