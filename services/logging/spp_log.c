#include "spp_log.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/**  Variables estáticas del módulo */
static spp_log_level_t current_log_level = SPP_LOG_INFO;
static spp_log_output_fn_t output_callback = NULL;
static spp_bool_t is_initialized = false;

/** Nombres legibles para los niveles de log */
static const char *log_level_strings[] = {
    [SPP_LOG_NONE]    = "NONE",
    [SPP_LOG_ERROR]   = "ERROR",
    [SPP_LOG_WARN]    = "WARN",
    [SPP_LOG_INFO]    = "INFO",
    [SPP_LOG_DEBUG]   = "DEBUG",
    [SPP_LOG_VERBOSE] = "VERBOSE"
};

/** Salida por defecto (consola) */
static void default_log_output(const char *tag, spp_log_level_t level, const char *format, va_list args) {
    const char *level_str = log_level_strings[level];
    
    /**  Formato: [NIVEL] [TAG] mensaje */
    printf("[%s] [%s] ", level_str, tag);
    vprintf(format, args);
    printf("\n");
}

retval_t SPP_LOG_Init(void) {
    if (is_initialized) {
        return SPP_ERROR_ALREADY_INITIALIZED;
    }
    
    current_log_level = SPP_LOG_INFO;
    output_callback = NULL;
    is_initialized = true;  
    SPP_LOGI("LOG", "Logging system initialized");
    
    return SPP_OK;
}

void SPP_LOG_SetLevel(spp_log_level_t level) {
    if (level <= SPP_LOG_VERBOSE) {
        current_log_level = level;
        SPP_LOGI("LOG", "Log level set to: %s", log_level_strings[level]);
    }
}

spp_log_level_t SPP_LOG_GetLevel(void) {
    return current_log_level;
}

void SPP_LOG_RegisterOutputCallback(spp_log_output_fn_t callback) {
    output_callback = callback;
    SPP_LOGI("LOG", "Custom output callback registered");
}

/** Función interna para obtener el nivel actual */
spp_log_level_t spp_log_current_level_get(void) {
    return current_log_level;
}

/**  Función interna que realiza el logging real */
void spp_log_write(spp_log_level_t level, const char *tag, const char *format, ...) {
    if (!is_initialized) {
        /**  Fallback silencioso si no está inicializado */
        return;
    }
    
    if (tag == NULL || format == NULL) {
        /** Error silencioso para evitar problemas */
        return;
    }
    
    va_list args;
    va_start(args, format);
    
    if (output_callback != NULL) {
        /**  Usar callback personalizado si está registrado */
        output_callback(tag, level, format, args);
    } else {
        /**  Usar salida por defecto (consola)*/
        default_log_output(tag, level, format, args);
    }
    
    va_end(args);
}