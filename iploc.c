/*
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "iploc.h"

typedef unsigned char byte;
typedef uint32_t uint;

// ------------------------------------------------------------------
// Decodes a 4 bytes big endian number from a byte array.

static inline uint decode_uint32_be(byte *b)
{
    return (b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3];
}

// ------------------------------------------------------------------
// Decode a 3 bytes little endian number from a byte array.

static inline uint decode_uint24_le(byte *b)
{
    return (b[2] << 16) | (b[1] << 8) | b[0];
}

// ------------------------------------------------------------------
// Decode a 4 bytes little endian number from a byte array.

static inline uint decode_uint32_le(byte *b)
{
    return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | b[0];
}

// ------------------------------------------------------------------
// _ip_db_t holds meta-data of a 17MON DB data

struct _ip_db_t
{
    uint hint[256]; // counter for each IP segment
    uint num_index; // total number of indexed IP in the DB
    byte *raw;      // raw db data copied from file
    byte *index;    // pointer to the first index
    byte *text;     // pointer to ip description section
};

// ------------------------------------------------------------------
// Create an ip_db_t object with zero value.

static ip_db_t*
ip_db_new()
{
    ip_db_t *db = (ip_db_t*)malloc(sizeof(ip_db_t));
    memset(db, 0, sizeof(ip_db_t));
    return db;
}

// ------------------------------------------------------------------
// Destroy an ip_db_t object and reclaim allocated memory as needed.

void
ip_db_destroy(ip_db_t **db)
{
    if (*db) {
        ip_db_t *p  = *db;
        if (p->raw) {
            free(p->raw);
        }
        free(p);
        *db = NULL;
    }
}

// ------------------------------------------------------------------
// Create and then initialize an ip_db_t object using a 17MON DB file.

ip_db_t*
ip_db_init(const char *path)
{
    FILE *fp = fopen(path, "rb");

    if (!fp) {
        printf("Cannot open %s\n", path);
        return NULL;
    }

    if (fseek(fp, 0, SEEK_END) != 0) {
        printf("Cannot seek to the end of %s\n", path);
        fclose(fp);
        return NULL;
    }

    long flen = ftell(fp);

    if (fseek(fp, 0, SEEK_SET) != 0) {
        printf("Cannot seek to the start of %s\n", path);
        fclose(fp);
        return NULL;
    }

    ip_db_t *db = ip_db_new();
    db->raw = (byte*)malloc(flen);

    if (fread(db->raw, flen, 1, fp) != 1) {
        printf("Failed to read %s\n", path);
        fclose(fp);
        ip_db_destroy(&db);
        return NULL;
    }

    fclose(fp);
    byte *pos = db->raw + 4;
    int i = 0;

    for (; i < 256; i++) {
        db->hint[i] = decode_uint32_le(pos);
        pos += 4;
    }

    uint text_offset = decode_uint32_be(db->raw);
    db->num_index = (text_offset - 4 - 2048) / 8;
    db->index = db->raw + 4 + 256 * 4;
    db->text = db->raw + text_offset;

    return db;
}

// ------------------------------------------------------------------
// Get the host representation of an ipv4 address. Return 0 on 
// malformed input.

static uint
get_ip_val(const char *ipv4)
{
    struct in_addr addr;
    return inet_aton(ipv4, &addr) == 0 ? 0 : ntohl(addr.s_addr);
}

// ------------------------------------------------------------------
// Search an ipv4 address in DB and return related description text.
// Return 0 on success, and -1 if any input is invalid.

int
ip_locate(ip_db_t *db, const char *ipv4, char *result)
{
    return ip_locate_v(db, get_ip_val(ipv4), result);
}

// ------------------------------------------------------------------
// IP search implementation. Binary Search is under the hood.
// Return 0 on success, and -1 if any input is invalid.

int
ip_locate_v(ip_db_t *db, uint32_t ip_val, char *result)
{
    if (db == NULL || ip_val == 0 || result == NULL) {
        return -1;
    }

    byte part1 = (ip_val >> 24);
    uint low = db->hint[part1];
    uint high = db->num_index - 1;

    if (part1 < 255) {
        high = db->hint[part1 + 1];
    }

    while (low < high) {
        uint mid = low + (high - low)/2;
        uint ip_indexed = decode_uint32_be(db->index + mid*8);

        if (ip_val > ip_indexed) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    byte *pos = db->index + high*8;
    uint offset = decode_uint24_le(pos + 4);
    uint len = pos[7];
    strncpy(result, db->text + offset - 1024, len);
    result[len] = 0;
    return 0;
}

