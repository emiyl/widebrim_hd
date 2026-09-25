#include "gds_reader.h"

#include <stdio.h>
#include <string.h>

static bool gds_reader_can_read(const gds_reader_t *reader, size_t size) {
    if (reader == NULL) {
        return false;
    }

    if (reader->offset > reader->size) {
        return false;
    }

    return size <= reader->size - reader->offset;
}

void gds_reader_init(gds_reader_t *reader, const void *data, size_t size) {
    if (reader == NULL) {
        return;
    }

    reader->data = (const uint8_t *)data;
    reader->size = size;
    reader->offset = 0;
}

bool gds_reader_read_uint8(gds_reader_t *reader, uint8_t *value) {
    if (!gds_reader_can_read(reader, sizeof(uint8_t))) {
        return false;
    }

    *value = reader->data[reader->offset];
    reader->offset += sizeof(uint8_t);
    return true;
}

bool gds_reader_read_uint16(gds_reader_t *reader, uint16_t *value) {
    const uint8_t *data;

    if (!gds_reader_can_read(reader, sizeof(uint16_t))) {
        return false;
    }

    data = reader->data + reader->offset;
    *value = (uint16_t)data[0] | ((uint16_t)data[1] << 8);
    reader->offset += sizeof(uint16_t);
    return true;
}

bool gds_reader_read_uint32(gds_reader_t *reader, uint32_t *value) {
    const uint8_t *data;

    if (!gds_reader_can_read(reader, sizeof(uint32_t))) {
        return false;
    }

    data = reader->data + reader->offset;
    *value = (uint32_t)data[0] | ((uint32_t)data[1] << 8) |
             ((uint32_t)data[2] << 16) | ((uint32_t)data[3] << 24);
    reader->offset += sizeof(uint32_t);
    return true;
}

bool gds_reader_read_opcode(gds_reader_t *reader, gds_opcode_t *value) {
    uint16_t bits;

    if (!gds_reader_read_uint16(reader, &bits)) {
        return false;
    }

    if (!gds_is_valid_opcode((gds_opcode_t)bits)) {
        fprintf(stderr, "gds: Invalid opcode: %u\n", bits);
        return false;
    }

    *value = (gds_opcode_t)bits;
    return true;
}

bool gds_reader_read_int32(gds_reader_t *reader, int32_t *value) {
    uint32_t bits;

    if (!gds_reader_read_uint32(reader, &bits)) {
        return false;
    }

    memcpy(value, &bits, sizeof(bits));
    return true;
}

bool gds_reader_read_float32(gds_reader_t *reader, float *value) {
    uint32_t bits;

    if (!gds_reader_read_uint32(reader, &bits)) {
        return false;
    }

    memcpy(value, &bits, sizeof(bits));
    return true;
}

bool gds_reader_read_bytes(gds_reader_t *reader, const uint8_t **buffer,
                           size_t length) {
    if (!gds_reader_can_read(reader, length)) {
        return false;
    }

    *buffer = reader->data + reader->offset;
    reader->offset += length;
    return true;
}

size_t gds_reader_remaining(const gds_reader_t *reader) {
    if (reader == NULL || reader->offset > reader->size) {
        return 0;
    }
    return reader->size - reader->offset;
}
