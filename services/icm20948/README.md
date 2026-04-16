# services/icm20948/

ICM20948 9-axis IMU service (3-axis accelerometer + 3-axis gyroscope + 3-axis magnetometer via AK09916). Supports both raw sensor polling and the on-chip Digital Motion Processor (DMP) for quaternion and step-count output.

APID: `0x0102`

---

## Files

| File | Description |
|---|---|
| `icm20948.h` | Public API, register map, config structs |
| `icm20948.c` | Driver implementation, DMP loading, service task |
| `dmp_image.h` | DMP firmware binary image (do not edit) |

---

## Register banks

The ICM20948 has four register banks (0–3) selected via register `0x7F`. The driver handles bank switching transparently — do not write to `0x7F` directly.

---

## DMP support

The DMP firmware (`dmp_image.h`) is loaded into the sensor's RAM at init time. When DMP is enabled:
- Outputs: rotation quaternion (Q30 fixed-point), step count, activity classification
- DMP runs independently on the sensor — the host only reads results

Disable DMP at build time with `-DSPP_ICM20948_NO_DMP=1` to save ~14 KB of flash.

---

## Registration

```c
extern const SPP_ServiceDesc_t g_icm20948ServiceDesc;

static ICM20948_ServiceCtx_t s_icmCtx;
static ICM20948_ServiceCfg_t s_icmCfg = {
    .csPin  = 21U,
    .useDmp = true,
};

SPP_Service_register(&g_icm20948ServiceDesc, &s_icmCtx, &s_icmCfg);
```

---

## Hardware configuration (ESP32-S3)

- SPI bus: SPI2_HOST
- CS GPIO: 21
- Clock: 1 MHz, MODE0
- INT pin: application-defined
