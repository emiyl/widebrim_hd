#include "../src/gds/gds.h"
#include "../src/gds/gds_opcode.h"

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint8_t *data;
    size_t size;
    size_t capacity;
} bytebuf_t;

typedef struct {
    const char *text;
    size_t length;
    size_t offset;
    size_t line;
} parser_t;

static void bytebuf_free(bytebuf_t *buf) {
    free(buf->data);
    buf->data = NULL;
    buf->size = 0U;
    buf->capacity = 0U;
}

static void bytebuf_ensure(bytebuf_t *buf, size_t needed) {
    size_t new_capacity;
    uint8_t *new_data;

    if (needed <= buf->capacity) {
        return;
    }

    new_capacity = buf->capacity == 0U ? 256U : buf->capacity;
    while (new_capacity < needed) {
        if (new_capacity > SIZE_MAX / 2U) {
            new_capacity = needed;
            break;
        }
        new_capacity *= 2U;
    }

    new_data = (uint8_t *)realloc(buf->data, new_capacity);
    if (new_data == NULL) {
        fprintf(stderr, "gds_compile: out of memory\n");
        exit(EXIT_FAILURE);
    }

    buf->data = new_data;
    buf->capacity = new_capacity;
}

static void bytebuf_append(bytebuf_t *buf, const void *data, size_t size) {
    if (size == 0U) {
        return;
    }
    bytebuf_ensure(buf, buf->size + size);
    memcpy(buf->data + buf->size, data, size);
    buf->size += size;
}

static void bytebuf_append_u16_le(bytebuf_t *buf, uint16_t value) {
    uint8_t bytes[2];
    bytes[0] = (uint8_t)(value & 0xFFU);
    bytes[1] = (uint8_t)((value >> 8U) & 0xFFU);
    bytebuf_append(buf, bytes, sizeof(bytes));
}

static void bytebuf_append_u32_le(bytebuf_t *buf, uint32_t value) {
    uint8_t bytes[4];
    bytes[0] = (uint8_t)(value & 0xFFU);
    bytes[1] = (uint8_t)((value >> 8U) & 0xFFU);
    bytes[2] = (uint8_t)((value >> 16U) & 0xFFU);
    bytes[3] = (uint8_t)((value >> 24U) & 0xFFU);
    bytebuf_append(buf, bytes, sizeof(bytes));
}

static void emit_record_type(bytebuf_t *buf, uint16_t type) {
    bytebuf_append_u16_le(buf, type);
}

static void patch_u32_at(bytebuf_t *buf, size_t offset, uint32_t value) {
    uint8_t *cursor;
    if (offset + 4U > buf->size) {
        fprintf(stderr, "gds_compile: invalid patch offset\n");
        exit(EXIT_FAILURE);
    }
    cursor = buf->data + offset;
    cursor[0] = (uint8_t)(value & 0xFFU);
    cursor[1] = (uint8_t)((value >> 8U) & 0xFFU);
    cursor[2] = (uint8_t)((value >> 16U) & 0xFFU);
    cursor[3] = (uint8_t)((value >> 24U) & 0xFFU);
}

static void emit_block(bytebuf_t *buf, uint16_t type) {
    emit_record_type(buf, type);
    bytebuf_append_u32_le(buf, 0U);
}

static void emit_command(bytebuf_t *buf, gds_opcode_t opcode) {
    emit_record_type(buf, GDS_RECORD_COMMAND);
    bytebuf_append_u16_le(buf, (uint16_t)opcode);
}

static void emit_int32(bytebuf_t *buf, int32_t value) {
    emit_record_type(buf, GDS_RECORD_VALUE_S32);
    bytebuf_append_u32_le(buf, (uint32_t)value);
}

static void emit_string(bytebuf_t *buf, const char *text) {
    const size_t length = strlen(text);
    emit_record_type(buf, GDS_RECORD_STRING);
    bytebuf_append_u16_le(buf, (uint16_t)length);
    bytebuf_append(buf, text, length);
}

static void emit_operator(bytebuf_t *buf, uint16_t operator_type) {
    emit_record_type(buf, operator_type);
}

static int ci_char_compare(char a, char b) {
    const unsigned char ca = (unsigned char)a;
    const unsigned char cb = (unsigned char)b;
    const unsigned char la = (unsigned char)tolower(ca);
    const unsigned char lb = (unsigned char)tolower(cb);
    return (int)la - (int)lb;
}

static bool ci_strcmp(const char *left, const char *right) {
    while (*left != '\0' && *right != '\0') {
        if (ci_char_compare(*left, *right) != 0) {
            return false;
        }
        left++;
        right++;
    }
    return *left == '\0' && *right == '\0';
}

static bool parser_has_more(const parser_t *parser) {
    return parser->offset < parser->length;
}

static bool parser_match_char(parser_t *parser, char expected) {
    if (!parser_has_more(parser) || parser->text[parser->offset] != expected) {
        return false;
    }
    parser->offset++;
    return true;
}

static void parser_skip_ws(parser_t *parser) {
    while (parser_has_more(parser)) {
        char c = parser->text[parser->offset];

        if (c == '\r') {
            parser->line++;
            parser->offset++;
            if (parser_has_more(parser) &&
                parser->text[parser->offset] == '\n') {
                parser->offset++;
            }
            continue;
        }
        if (c == '\n') {
            parser->line++;
            parser->offset++;
            continue;
        }
        if (isspace((unsigned char)c)) {
            parser->offset++;
            continue;
        }
        if (c == '/' && parser->offset + 1U < parser->length) {
            char next = parser->text[parser->offset + 1U];
            if (next == '/') {
                parser->offset += 2U;
                while (parser_has_more(parser) &&
                       parser->text[parser->offset] != '\n' &&
                       parser->text[parser->offset] != '\r') {
                    parser->offset++;
                }
                continue;
            }
            if (next == '*') {
                parser->offset += 2U;
                while (parser->offset + 1U < parser->length) {
                    if (parser->text[parser->offset] == '*' &&
                        parser->text[parser->offset + 1U] == '/') {
                        parser->offset += 2U;
                        break;
                    }
                    if (parser->text[parser->offset] == '\n' ||
                        parser->text[parser->offset] == '\r') {
                        parser->line++;
                        if (parser->text[parser->offset] == '\r' &&
                            parser->offset + 1U < parser->length &&
                            parser->text[parser->offset + 1U] == '\n') {
                            parser->offset++;
                        }
                    }
                    parser->offset++;
                }
                continue;
            }
        }
        break;
    }
}

static bool parser_read_identifier(parser_t *parser, char *buffer,
                                   size_t buffer_size) {
    size_t length = 0U;

    parser_skip_ws(parser);
    if (!parser_has_more(parser)) {
        return false;
    }

    if (!isalpha((unsigned char)parser->text[parser->offset]) &&
        parser->text[parser->offset] != '_') {
        return false;
    }

    while (parser_has_more(parser)) {
        char c = parser->text[parser->offset];
        if (isalnum((unsigned char)c) || c == '_') {
            if (length + 1U >= buffer_size) {
                fprintf(stderr,
                        "gds_compile: identifier too long on line %zu\n",
                        parser->line + 1U);
                return false;
            }
            buffer[length++] = c;
            parser->offset++;
            continue;
        }
        break;
    }

    buffer[length] = '\0';
    return length > 0U;
}

static bool parser_read_string(parser_t *parser, char **output) {
    size_t capacity = 32U;
    size_t length = 0U;
    char *buffer;

    parser_skip_ws(parser);
    if (!parser_match_char(parser, '"')) {
        return false;
    }

    buffer = (char *)malloc(capacity);
    if (buffer == NULL) {
        fprintf(stderr, "gds_compile: out of memory\n");
        return false;
    }

    while (parser_has_more(parser)) {
        char c = parser->text[parser->offset++];
        if (c == '"') {
            buffer[length] = '\0';
            *output = buffer;
            return true;
        }

        if (c == '\\') {
            if (!parser_has_more(parser)) {
                fprintf(stderr,
                        "gds_compile: unterminated escape on line %zu\n",
                        parser->line + 1U);
                free(buffer);
                return false;
            }
            c = parser->text[parser->offset++];
            switch (c) {
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case '\\':
            case '"':
                break;
            default:
                fprintf(stderr,
                        "gds_compile: unsupported escape \\%c on line %zu\n", c,
                        parser->line + 1U);
                free(buffer);
                return false;
            }
        }

        if (length + 1U >= capacity) {
            size_t new_capacity = capacity * 2U;
            char *new_buffer = (char *)realloc(buffer, new_capacity);
            if (new_buffer == NULL) {
                fprintf(stderr, "gds_compile: out of memory\n");
                free(buffer);
                return false;
            }
            buffer = new_buffer;
            capacity = new_capacity;
        }
        buffer[length++] = c;
    }

    fprintf(stderr, "gds_compile: unterminated string on line %zu\n",
            parser->line + 1U);
    free(buffer);
    return false;
}

static bool parser_read_number(parser_t *parser, int32_t *value) {
    char buffer[64];
    size_t length = 0U;
    long long parsed = 0;
    char *end_ptr = NULL;

    parser_skip_ws(parser);
    if (!parser_has_more(parser)) {
        return false;
    }

    if (parser->text[parser->offset] == '-' ||
        parser->text[parser->offset] == '+') {
        if (length + 1U >= sizeof(buffer)) {
            return false;
        }
        buffer[length++] = parser->text[parser->offset++];
    }

    while (parser_has_more(parser) &&
           isdigit((unsigned char)parser->text[parser->offset])) {
        if (length + 1U >= sizeof(buffer)) {
            return false;
        }
        buffer[length++] = parser->text[parser->offset++];
    }

    if (length == 0U) {
        return false;
    }

    buffer[length] = '\0';
    errno = 0;
    parsed = strtoll(buffer, &end_ptr, 10);
    if (errno != 0 || end_ptr == buffer || parsed < INT32_MIN ||
        parsed > INT32_MAX) {
        fprintf(stderr, "gds_compile: integer out of range on line %zu\n",
                parser->line + 1U);
        return false;
    }
    *value = (int32_t)parsed;
    return true;
}

static bool lookup_opcode(const char *name, gds_opcode_t *opcode) {
    *opcode = gds_opcode_from_string(name);
    return *opcode != SCRIPT_CMD_Invalid;
}

static bool parse_statement(parser_t *parser, bytebuf_t *output);
static bool parse_expression(parser_t *parser, bytebuf_t *output);

static bool parse_primary(parser_t *parser, bytebuf_t *output) {
    char identifier[128];
    int32_t int_value;
    char *string_value = NULL;
    bool result = false;
    gds_opcode_t opcode;

    parser_skip_ws(parser);
    if (parser_match_char(parser, '(')) {
        if (!parse_expression(parser, output)) {
            return false;
        }
        if (!parser_match_char(parser, ')')) {
            fprintf(stderr, "gds_compile: expected ')' on line %zu\n",
                    parser->line + 1U);
            return false;
        }
        return true;
    }

    if (parser_read_number(parser, &int_value)) {
        emit_int32(output, int_value);
        return true;
    }

    if (parser_read_string(parser, &string_value)) {
        emit_string(output, string_value);
        free(string_value);
        return true;
    }

    if (!parser_read_identifier(parser, identifier, sizeof(identifier))) {
        fprintf(stderr, "gds_compile: expected expression on line %zu\n",
                parser->line + 1U);
        return false;
    }

    if (ci_strcmp(identifier, "TRUE")) {
        emit_command(output, SCRIPT_CMD_TRUE);
        return true;
    }
    if (ci_strcmp(identifier, "FALSE")) {
        emit_command(output, SCRIPT_CMD_FALSE);
        return true;
    }

    if (!lookup_opcode(identifier, &opcode)) {
        fprintf(stderr,
                "gds_compile: unknown opcode or expression '%s' on line %zu\n",
                identifier, parser->line + 1U);
        return false;
    }

    emit_command(output, opcode);

    parser_skip_ws(parser);
    if (parser_match_char(parser, '(')) {
        bool first = true;
        while (true) {
            parser_skip_ws(parser);
            if (parser_match_char(parser, ')')) {
                break;
            }
            if (!first) {
                if (!parser_match_char(parser, ',')) {
                    fprintf(stderr,
                            "gds_compile: expected ',' in call on line %zu\n",
                            parser->line + 1U);
                    return false;
                }
            }
            if (!parse_expression(parser, output)) {
                return false;
            }
            first = false;
        }
        return true;
    }

    if (opcode == SCRIPT_CMD_TRUE || opcode == SCRIPT_CMD_FALSE) {
        result = true;
    }
    return result;
}

static bool parse_unary(parser_t *parser, bytebuf_t *output) {
    parser_skip_ws(parser);
    if (parser_match_char(parser, '!')) {
        emit_operator(output, GDS_RECORD_NOT);
        return parse_unary(parser, output);
    }
    return parse_primary(parser, output);
}

static bool parse_and(parser_t *parser, bytebuf_t *output) {
    if (!parse_unary(parser, output)) {
        return false;
    }
    while (true) {
        parser_skip_ws(parser);
        if (parser->offset + 1U < parser->length &&
            parser->text[parser->offset] == '&' &&
            parser->text[parser->offset + 1U] == '&') {
            parser->offset += 2U;
            emit_operator(output, GDS_RECORD_AND);
            if (!parse_unary(parser, output)) {
                return false;
            }
            continue;
        }
        break;
    }
    return true;
}

static bool parse_or(parser_t *parser, bytebuf_t *output) {
    if (!parse_and(parser, output)) {
        return false;
    }
    while (true) {
        parser_skip_ws(parser);
        if (parser->offset + 1U < parser->length &&
            parser->text[parser->offset] == '|' &&
            parser->text[parser->offset + 1U] == '|') {
            parser->offset += 2U;
            emit_operator(output, GDS_RECORD_OR);
            if (!parse_and(parser, output)) {
                return false;
            }
            continue;
        }
        break;
    }
    return true;
}

static bool parse_expression(parser_t *parser, bytebuf_t *output) {
    return parse_or(parser, output);
}

static bool parse_block(parser_t *parser, bytebuf_t *output) {
    size_t block_start_offset;

    parser_skip_ws(parser);
    if (!parser_match_char(parser, '{')) {
        fprintf(stderr, "gds_compile: expected '{' on line %zu\n",
                parser->line + 1U);
        return false;
    }

    block_start_offset = output->size;
    emit_block(output, GDS_RECORD_BLOCK_START);
    while (true) {
        parser_skip_ws(parser);
        if (!parser_has_more(parser)) {
            fprintf(stderr, "gds_compile: unterminated block on line %zu\n",
                    parser->line + 1U);
            return false;
        }
        if (parser_match_char(parser, '}')) {
            size_t block_end_offset = output->size;
            emit_block(output, GDS_RECORD_BLOCK_END);
            patch_u32_at(output, block_start_offset + 2U,
                         (uint32_t)block_end_offset);
            patch_u32_at(output, block_end_offset + 2U,
                         (uint32_t)block_end_offset);
            return true;
        }
        if (!parse_statement(parser, output)) {
            return false;
        }
    }
}

static bool parse_statement(parser_t *parser, bytebuf_t *output) {
    char name[128];
    gds_opcode_t opcode;

    parser_skip_ws(parser);
    if (!parser_has_more(parser)) {
        return true;
    }
    if (parser->text[parser->offset] == '}') {
        return true;
    }

    if (!parser_read_identifier(parser, name, sizeof(name))) {
        fprintf(stderr, "gds_compile: expected statement on line %zu\n",
                parser->line + 1U);
        return false;
    }

    if (ci_strcmp(name, "IF") || ci_strcmp(name, "WHILE") ||
        ci_strcmp(name, "ELSEIF")) {
        if (!lookup_opcode(name, &opcode)) {
            fprintf(stderr,
                    "gds_compile: unknown block opcode '%s' on line %zu\n",
                    name, parser->line + 1U);
            return false;
        }
        emit_command(output, opcode);
        parser_skip_ws(parser);
        if (!parser_match_char(parser, '(')) {
            fprintf(stderr, "gds_compile: expected '(' after %s on line %zu\n",
                    name, parser->line + 1U);
            return false;
        }
        if (!parse_expression(parser, output)) {
            return false;
        }
        if (!parser_match_char(parser, ')')) {
            fprintf(stderr,
                    "gds_compile: expected ')' after condition on line %zu\n",
                    parser->line + 1U);
            return false;
        }
        if (!parse_block(parser, output)) {
            return false;
        }
        return true;
    }

    if (ci_strcmp(name, "ELSE")) {
        if (!lookup_opcode(name, &opcode)) {
            fprintf(stderr, "gds_compile: unknown opcode '%s' on line %zu\n",
                    name, parser->line + 1U);
            return false;
        }
        emit_command(output, opcode);
        return parse_block(parser, output);
    }

    if (ci_strcmp(name, "Loop")) {
        if (!lookup_opcode(name, &opcode)) {
            fprintf(stderr, "gds_compile: unknown opcode '%s' on line %zu\n",
                    name, parser->line + 1U);
            return false;
        }
        emit_command(output, opcode);
        return parse_block(parser, output);
    }

    if (!lookup_opcode(name, &opcode)) {
        fprintf(stderr, "gds_compile: unknown opcode '%s' on line %zu\n", name,
                parser->line + 1U);
        return false;
    }

    emit_command(output, opcode);

    parser_skip_ws(parser);
    if (parser_match_char(parser, '(')) {
        bool first = true;
        while (true) {
            parser_skip_ws(parser);
            if (parser_match_char(parser, ')')) {
                break;
            }
            if (!first) {
                if (!parser_match_char(parser, ',')) {
                    fprintf(
                        stderr,
                        "gds_compile: expected ',' in arguments on line %zu\n",
                        parser->line + 1U);
                    return false;
                }
            }
            if (!parse_expression(parser, output)) {
                return false;
            }
            first = false;
        }
    }

    parser_skip_ws(parser);
    if (parser_match_char(parser, ';')) {
        return true;
    }

    return true;
}

static bool compile_script(const char *source_text, bytebuf_t *output) {
    parser_t parser;

    parser.text = source_text;
    parser.length = strlen(source_text);
    parser.offset = 0U;
    parser.line = 0U;

    while (parser_has_more(&parser)) {
        parser_skip_ws(&parser);
        if (!parser_has_more(&parser)) {
            break;
        }
        if (!parse_statement(&parser, output)) {
            return false;
        }
        parser_skip_ws(&parser);
        if (parser_match_char(&parser, ';')) {
            continue;
        }
    }

    return true;
}

static unsigned char *read_entire_file(const char *path, size_t *length_out) {
    FILE *file = fopen(path, "rb");
    long length;
    unsigned char *buffer;
    size_t read_count;

    if (file == NULL) {
        fprintf(stderr, "gds_compile: could not open '%s': %s\n", path,
                strerror(errno));
        return NULL;
    }

    if (fseek(file, 0L, SEEK_END) != 0) {
        fprintf(stderr, "gds_compile: failed to seek '%s'\n", path);
        fclose(file);
        return NULL;
    }

    length = ftell(file);
    if (length < 0L) {
        fprintf(stderr, "gds_compile: failed to measure '%s'\n", path);
        fclose(file);
        return NULL;
    }

    if (fseek(file, 0L, SEEK_SET) != 0) {
        fprintf(stderr, "gds_compile: failed to rewind '%s'\n", path);
        fclose(file);
        return NULL;
    }

    buffer = (unsigned char *)malloc((size_t)length + 1U);
    if (buffer == NULL) {
        fprintf(stderr, "gds_compile: out of memory\n");
        fclose(file);
        return NULL;
    }

    read_count = fread(buffer, 1U, (size_t)length, file);
    fclose(file);

    if (read_count != (size_t)length) {
        fprintf(stderr, "gds_compile: failed to read '%s'\n", path);
        free(buffer);
        return NULL;
    }

    buffer[length] = '\0';
    *length_out = (size_t)length;
    return buffer;
}

static bool write_output_file(const char *path, const bytebuf_t *payload) {
    FILE *file = fopen(path, "wb");
    uint8_t header[4];

    if (file == NULL) {
        fprintf(stderr, "gds_compile: could not open '%s' for writing: %s\n",
                path, strerror(errno));
        return false;
    }

    header[0] = (uint8_t)(payload->size & 0xFFU);
    header[1] = (uint8_t)((payload->size >> 8U) & 0xFFU);
    header[2] = (uint8_t)((payload->size >> 16U) & 0xFFU);
    header[3] = (uint8_t)((payload->size >> 24U) & 0xFFU);

    if (fwrite(header, 1U, sizeof(header), file) != sizeof(header)) {
        fprintf(stderr, "gds_compile: failed to write header to '%s'\n", path);
        fclose(file);
        return false;
    }

    if (payload->size > 0U &&
        fwrite(payload->data, 1U, payload->size, file) != payload->size) {
        fprintf(stderr, "gds_compile: failed to write payload to '%s'\n", path);
        fclose(file);
        return false;
    }

    if (fclose(file) != 0) {
        fprintf(stderr, "gds_compile: failed to close '%s'\n", path);
        return false;
    }

    return true;
}

static void print_usage(const char *program_name) {
    fprintf(stderr, "Usage: %s <input.gsc> <output.gds>\n", program_name);
}

int main(int argc, char **argv) {
    const char *input_path = NULL;
    const char *output_path = NULL;
    unsigned char *source = NULL;
    size_t source_size = 0U;
    bytebuf_t output = {0};
    bool ok;

    if (argc != 3) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    input_path = argv[1];
    output_path = argv[2];

    source = read_entire_file(input_path, &source_size);
    if (source == NULL) {
        return EXIT_FAILURE;
    }

    ok = compile_script((const char *)source, &output);
    free(source);
    if (!ok) {
        bytebuf_free(&output);
        return EXIT_FAILURE;
    }

    if (!write_output_file(output_path, &output)) {
        bytebuf_free(&output);
        return EXIT_FAILURE;
    }

    bytebuf_free(&output);
    return EXIT_SUCCESS;
}
