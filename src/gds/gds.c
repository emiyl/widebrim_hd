#include "gds.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool gds_read_record(gds_reader_t *reader, gds_record_t *record) {
    uint16_t type;
    uint16_t size;

    if (reader == NULL || record == NULL) {
        return false;
    }

    if (!gds_reader_read_uint16(reader, &type)) {
        return false;
    }

    record->type = type;

    switch (type) {
    case GDS_RECORD_COMMAND:
        return gds_reader_read_uint16(reader, &record->payload.opcode);

    case GDS_RECORD_VALUE_1:
    case GDS_RECORD_VALUE_2:
    case GDS_RECORD_VALUE_6:
    case GDS_RECORD_VALUE_7:
        return gds_reader_read_uint32(reader, &record->payload.value);

    case GDS_RECORD_BYTES_1:
    case GDS_RECORD_BYTES_2:
        if (!gds_reader_read_uint16(reader, &size)) {
            return false;
        }

        record->payload.bytes.size = size;

        return gds_reader_read_bytes(reader, &record->payload.bytes.data, size);

    case GDS_RECORD_EMPTY_5:
    case GDS_RECORD_EMPTY_8:
    case GDS_RECORD_EMPTY_9:
    case GDS_RECORD_EMPTY_10:
    case GDS_RECORD_EMPTY_11:
    case GDS_RECORD_EMPTY_12:
        return true;

    default:
        return false;
    }
}

bool gds_extract_payload(const uint8_t *file, size_t file_size,
                         const uint8_t **payload, size_t *payload_size) {
    uint32_t size;

    if (file == NULL || payload == NULL || payload_size == NULL ||
        file_size < 4) {
        return false;
    }

    size = (uint32_t)file[0] | ((uint32_t)file[1] << 8) |
           ((uint32_t)file[2] << 16) | ((uint32_t)file[3] << 24);

    if ((size_t)size > file_size - 4) {
        return false;
    }

    *payload = file + 4;
    *payload_size = size;

    return true;
}

bool gds_load_from_file_path(const char *file_path, const uint8_t **payload,
                             size_t *payload_size) {
    uint8_t *file_data = NULL;
    uint8_t *payload_data = NULL;
    const uint8_t *extracted_payload = NULL;
    bool result = false;

    if (file_path == NULL || payload == NULL || payload_size == NULL) {
        return false;
    }

    *payload = NULL;
    *payload_size = 0;

    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        return false;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return false;
    }

    long file_size = ftell(file);
    if (file_size < 0) {
        fclose(file);
        return false;
    }

    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return false;
    }

    file_data = (uint8_t *)malloc((size_t)file_size);
    if (file_data == NULL) {
        fclose(file);
        return false;
    }

    if (fread(file_data, 1, (size_t)file_size, file) != (size_t)file_size) {
        free(file_data);
        fclose(file);
        return false;
    }

    fclose(file);

    if (!gds_extract_payload(file_data, (size_t)file_size, &extracted_payload,
                             payload_size)) {
        free(file_data);
        return false;
    }

    payload_data = (uint8_t *)malloc(*payload_size);
    if (payload_data == NULL && *payload_size != 0) {
        free(file_data);
        return false;
    }

    if (*payload_size != 0) {
        memcpy(payload_data, extracted_payload, *payload_size);
    }

    free(file_data);
    *payload = payload_data;
    result = true;
    return result;
}

void gds_free_payload(const uint8_t *payload) { free((void *)payload); }
