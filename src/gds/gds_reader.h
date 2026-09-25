#ifndef GDS_READER_H
#define GDS_READER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t offset;
} gds_reader_t;

void gds_reader_init(gds_reader_t *reader, const void *data, size_t size);
bool gds_reader_read_uint8(gds_reader_t *reader, uint8_t *value);
bool gds_reader_read_uint16(gds_reader_t *reader, uint16_t *value);
bool gds_reader_read_uint32(gds_reader_t *reader, uint32_t *value);
bool gds_reader_read_bytes(gds_reader_t *reader, const uint8_t **buffer,
                           size_t length);
size_t gds_reader_remaining(const gds_reader_t *reader);

#endif // GDS_READER_H
