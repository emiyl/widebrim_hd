#include "gds.h"

#include <inttypes.h>
#include <stdio.h>

static void dump_bytes(const uint8_t *data, uint16_t size) {
    uint16_t i;

    for (i = 0; i < size; i++) {
        printf("%02" PRIx8 " ", data[i]);
    }

    putchar('\n');
}

static void dump_record(const gds_record_t *record) {
    printf("type: %" PRIu16 "\n", record->type);

    switch (record->type) {
    case GDS_RECORD_COMMAND:
        printf("opcode: %" PRIu16 "\n", record->payload.opcode);
        break;

    case GDS_RECORD_VALUE_1:
    case GDS_RECORD_VALUE_2:
    case GDS_RECORD_VALUE_6:
    case GDS_RECORD_VALUE_7:
        printf(" value=%" PRIu32 " (0x%08" PRIx32 ")", record->payload.value,
               record->payload.value);
        break;

    case GDS_RECORD_BYTES_1:
    case GDS_RECORD_BYTES_2:
        printf(" size=%zu data=", record->payload.bytes.size);

        dump_bytes(record->payload.bytes.data, record->payload.bytes.size);
        return;

    default:
        break;
    }

    putchar('\n');
}

bool dump_gds(const uint8_t *data, size_t size) {
    gds_reader_t reader;
    gds_record_t record;

    gds_reader_init(&reader, data, size);

    while (gds_reader_remaining(&reader) > 0) {
        size_t old_offset = reader.offset;

        if (!gds_read_record(&reader, &record)) {
            fprintf(stderr, "Invalid record at offset %zu\n", old_offset);
            return false;
        }

        printf("offset: %zu\n", old_offset);

        dump_record(&record);
    }

    return true;
}
