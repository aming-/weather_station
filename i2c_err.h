
#define printI2cErr(txt, code) \
{ \
    fprintf(stderr, "%s %d %s: ", __FILE__, __LINE__, txt); \
    switch (code) { \
    case BCM2835_I2C_REASON_OK: \
        fprintf(stderr, "Success\n"); \
        break; \
    case BCM2835_I2C_REASON_ERROR_NACK: \
        fprintf(stderr, "Received a NACK\n"); \
        break; \
    case BCM2835_I2C_REASON_ERROR_CLKT: \
        fprintf(stderr, "Received Clock Stretch Timeout\n"); \
        break; \
    case BCM2835_I2C_REASON_ERROR_DATA: \
        fprintf(stderr, "Not all data is sent / received\n"); \
        break; \
    default: \
        fprintf(stderr, "Unknown code %d\n", code); \
        break; \
    } \
    fflush(stderr); \
}
