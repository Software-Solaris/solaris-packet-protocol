#ifndef SPP_LOG_H
#define SPP_LOG_H

#include "../../core/types.h"
#include "../../core/returntypes.h"

typedef enum {
    SPP_LOG_NONE = 0,
    SPP_LOG_ERROR,
    SPP_LOG_WARN,
    SPP_LOG_INFO,
    SPP_LOG_DEBUG,
    SPP_LOG_VERBOSE
} spp_log_level_t;

/**  Inicialización del sistema de logging */
retval_t SPP_LOG_Init(void);

/**  Configuración del nivel de logging */
void SPP_LOG_SetLevel(spp_log_level_t level);
spp_log_level_t SPP_LOG_GetLevel(void);

/**  Callback para salida personalizada */
typedef void (*spp_log_output_fn_t)(const char *tag, spp_log_level_t level, const char *format, ...);
void SPP_LOG_RegisterOutputCallback(spp_log_output_fn_t callback);

/** Macros de logging principal */ 
#define SPP_LOGE(tag, format, ...) \
    do { \
        if (SPP_LOG_ERROR <= spp_log_current_level_get()) { \
            spp_log_write(SPP_LOG_ERROR, tag, format, ##__VA_ARGS__); \
        } \
    } while (0)

#define SPP_LOGW(tag, format, ...) \
    do { \
        if (SPP_LOG_WARN <= spp_log_current_level_get()) { \
            spp_log_write(SPP_LOG_WARN, tag, format, ##__VA_ARGS__); \
        } \
    } while (0)

#define SPP_LOGI(tag, format, ...) \
    do { \
        if (SPP_LOG_INFO <= spp_log_current_level_get()) { \
            spp_log_write(SPP_LOG_INFO, tag, format, ##__VA_ARGS__); \
        } \
    } while (0)

#define SPP_LOGD(tag, format, ...) \
    do { \
        if (SPP_LOG_DEBUG <= spp_log_current_level_get()) { \
            spp_log_write(SPP_LOG_DEBUG, tag, format, ##__VA_ARGS__); \
        } \
    } while (0)

#define SPP_LOGV(tag, format, ...) \
    do { \
        if (SPP_LOG_VERBOSE <= spp_log_current_level_get()) { \
            spp_log_write(SPP_LOG_VERBOSE, tag, format, ##__VA_ARGS__); \
        } \
    } while (0)

// Funciones internas (no usar directamente)
spp_log_level_t spp_log_current_level_get(void);
void spp_log_write(spp_log_level_t level, const char *tag, const char *format, ...);

#endif // SPP_LOG_H