/**
 * @file
 * @brief Functions for esp32-serial library
 *
 * @author @htmlonly &copy; @endhtmlonly 2021 James Bennion-Pedley
 *
 * @date 09 June 2021
 */

#include "esp32serial.h"

/*------------------------ Private Macros and Structs ------------------------*/


/** @brief Debug Types */
#define DEBUG_TYPE_INFO      'I'
#define DEBUG_TYPE_WARNINGS  'W'
#define DEBUG_TYPE_ERRORS    'E'

#define DEBUG_COLOUR_PREFIX "\u001b["
#define DEBUG_COLOUR_SUFFIX "m"
#define DEBUG_COLOUR_RESET  "\u001b[0m"

/** @brief Debug struct that is added to the message queue */
typedef struct {
    char type;
    char colour[2];
    TaskHandle_t task;
    char* message;
} debug_t;


/*------------------------------ Private Methods -----------------------------*/


void Esp32Serial::_debugHandler(void *q)
{
    debug_t debug;

    for(;;) {
        /* Block until there is an item in the queue */
        xQueueReceive(q, &debug, portMAX_DELAY);

        #if DEBUG_COLOUR
            Serial.write(DEBUG_COLOUR_PREFIX);
            Serial.write(debug.colour, 2);
            Serial.write(DEBUG_COLOUR_SUFFIX);
        #endif


        /* Print Type header and task name */
        Serial.print(debug.type);
        Serial.print(DEBUG_DELIMITER);
        char* task_ptr = pcTaskGetTaskName(debug.task);
        Serial.print(task_ptr);
        Serial.print(DEBUG_DELIMITER);

        /* Print message */
        Serial.print(debug.message);
        vPortFree(debug.message);

        #if DEBUG_COLOUR
            Serial.write(DEBUG_COLOUR_RESET);
        #endif

        Serial.println();
    }
}

int Esp32Serial::_debugSend(int type, char colour[2], const char* format, va_list args)
{
    int err;
    debug_t debug;

    /* Populate debug message queue */
    debug.type = type;
    debug.colour[0] = colour[0];
    debug.colour[1] = colour[1];

    switch(uxQueueSpacesAvailable(_debugQueue)) {
        case 0:
            err = -2;
            break;
        case 1:
        {
            debug.task = debugTask;

            /* Warn if queue has only one space left */
            char full_message[] = "Queue Full!";
            debug.message = (char*)pvPortMalloc(sizeof(full_message));
            strcpy(debug.message, full_message);

            xQueueSend(_debugQueue, &debug, 0);
            err = -1;
            break;
        }
        default:
            debug.task = xTaskGetCurrentTaskHandle();

            //Serial.println("testln");
            size_t len = vsnprintf(NULL, 0, format, args) + 1;
            //Serial.println(len);
            debug.message = (char*)pvPortMalloc(len);
            vsnprintf(debug.message, len, format, args);

            xQueueSend(_debugQueue, &debug, 0);
            err = 0;
            break;
    }
    return err;
}


/*------------------------------- Public Methods -----------------------------*/


Esp32Serial::Esp32Serial(const uint baud, size_t queue_length, bool colour)
{
    /* Initialise Serial communication */
    Serial.begin(baud);

    /* Initialise message queue */
    _debugQueue = xQueueCreate(queue_length, sizeof(debug_t));

    /* Create debug task and pass handle back to the user application */
    xTaskCreate(_debugWrapper, "debug", 2048, _debugQueue, 1, &debugTask);
}

Esp32Serial::~Esp32Serial(void)
{
    // TODO free entire queue messages

    vQueueDelete(_debugQueue);

    vTaskDelete(debugTask);

    Serial.end();
}

int Esp32Serial::info(const char* format, ...)
{
    #if DEBUG_LEVEL >= DEBUG_INFO
        va_list args;
        va_start(args, format);
        char colour[] = DEBUG_COLOUR_INFO;
        int err = _debugSend(DEBUG_TYPE_INFO, colour, format, args);
        va_end(args);
        return err;
    #else
        return 0;
    #endif
}

int Esp32Serial::warning(const char* format, ...)
{
    #if DEBUG_LEVEL >= DEBUG_WARNING
        va_list args;
        va_start(args, format);
        char colour[] = DEBUG_COLOUR_WARNINGS;
        int err = _debugSend(DEBUG_TYPE_WARNINGS, colour, format, args);
        va_end(args);
        return err;
    #else
        return 0;
    #endif
}

int Esp32Serial::error(const char* format, ...)
{
    #if DEBUG_LEVEL >= DEBUG_ERROR
        va_list args;
        va_start(args, format);
        char colour[] = DEBUG_COLOUR_ERRORS;
        int err = _debugSend(DEBUG_TYPE_ERRORS, colour, format, args);
        va_end(args);
        return err;
    #else
        return 0;
    #endif
}