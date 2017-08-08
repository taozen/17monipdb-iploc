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
// Decodes a 2 bytes big endian number from a byte array.

static inline uint decode_uint16_be(byte *b)
{
    return (b[0] << 8) | b[1];
}

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
// _ip_db_t holds meta-data of a 17MON DB

struct _ip_db_t
{
    byte extended;      // datx or dat?
    uint hindex_size;   // size of the index of a hint
    uint hint_size;     // size of bytes total hint area occupies
    uint index_num;     // total number of indexed IP in the DB
    uint index_size;    // size of an index chunk
    uint *hint;         // hint for the number of indexed IP in each IP segment
    byte *raw;          // raw data copied from 17MON DB file
    byte *index;        // pointer to the first index
    byte *text;         // pointer to ip description section
};

// ------------------------------------------------------------------
// ip_db_hint_get_low returns the lower bound of the sequence of an
// IP's index

static inline uint
ip_db_hint_get_low(ip_db_t *db, uint ip_val)
{
    // Index of the desired hint is equal to the first |hindex_size|
    // octet(s) of the ip value.
    uint hid = ip_val >> (8*(4-db->hindex_size));
    return db->hint[hid];
}

// ------------------------------------------------------------------
// ip_db_hint_get_high returns the upper bound of the sequence of an
// IP's index

static inline uint
ip_db_hint_get_high(ip_db_t *db, uint ip_val)
{
    uint hid = ip_val >> (8*(4-db->hindex_size));
    uint limit = (1 << (8*db->hindex_size)) - 1;
    return (hid == limit ? db->index_num-1 : db->hint[hid+1]);
}

// ------------------------------------------------------------------
// ip_db_index_get_ip returns the value of the nth-indexed IP

static inline uint
ip_db_index_get_ip(ip_db_t *db, uint n)
{
    byte *pos = db->index + n*db->index_size;
    return decode_uint32_be(pos);
}

// ------------------------------------------------------------------
// ip_db_index_get_offset returns the offset of the nth-indexed IP
// against the text section

static inline uint
ip_db_index_get_offset(ip_db_t *db, uint n)
{
    byte *pos = db->index + n*db->index_size + 4;
    return decode_uint24_le(pos);
}

// ------------------------------------------------------------------
// ip_db_index_get_text_len returns the text length of the
// nth-indexed IP

static inline uint
ip_db_index_get_text_len(ip_db_t *db, uint n)
{
    byte *pos = db->index + n*db->index_size + 7;
    return db->extended ? decode_uint16_be(pos) : pos[0];
}

// ------------------------------------------------------------------
// ip_db_get_text gets the pointer to the description text according
// to the offset obtained from the index section.

static inline const char*
ip_db_get_text(ip_db_t *db, uint offset)
{
    return (const char*)(db->text + offset - db->hint_size);
}

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

        if (p->hint) {
            free(p->hint);
        }

        free(p);
        *db = NULL;
    }
}

// ------------------------------------------------------------------
// Create and then initialize an ip_db_t object (implementation).

ip_db_t*
ip_db_init_impl(const char *path, byte extended)
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

    db->extended = extended;
    uint hindex_size = extended ? 2 : 1;
    db->hindex_size = hindex_size;
    db->index_size = 4 + 3 + hindex_size;

    uint hint_num = 1 << (hindex_size*8);
    uint hint_size = sizeof(uint) * hint_num;
    db->hint = malloc(hint_size);
    db->hint_size = hint_size;

    int i = 0;
    byte *pos = db->raw + 4;
    for (; i < hint_num; i++) {
        db->hint[i] = decode_uint32_le(pos);
        pos += 4;
    }

    uint text_offset = decode_uint32_be(db->raw);
    db->text = db->raw + text_offset;
    db->index = db->raw + 4 + hint_size;

    // There's a reserved area in the end of the index area. Its size
    // is equal to |hint_size|.
    db->index_num = ((db->text - db->index) - hint_size) / db->index_size;

    return db;
}

// ------------------------------------------------------------------
// Create and then initialize an ip_db_t object using a 17MON DB file.

ip_db_t*
ip_db_init(const char *path)
{
    return ip_db_init_impl(path, 0);
}

// ------------------------------------------------------------------
// Create and then initialize an ip_db_t object using an extended
// 17MON DB file.

ip_db_t*
ip_db_init_x(const char *path)
{
    return ip_db_init_impl(path, 1);
}

// ------------------------------------------------------------------
// Get the host representation of an ipv4 address. Return 0 on 
// malformed input.

static inline uint
get_ip_val(const char *ipv4)
{
    struct in_addr addr;
    return inet_pton(AF_INET, ipv4, &addr) == 1 ? ntohl(addr.s_addr) : 0;
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

    uint low = ip_db_hint_get_low(db, ip_val);
    uint high = ip_db_hint_get_high(db, ip_val);

    while (low < high) {
        uint mid = low + (high - low)/2;
        uint ip_indexed = ip_db_index_get_ip(db, mid);

        if (ip_val > ip_indexed) {
            low = mid + 1;
        } else {
            high = mid;
        }
    }

    uint offset = ip_db_index_get_offset(db, high);
    uint len = ip_db_index_get_text_len(db, high);
    const char *text = ip_db_get_text(db, offset);

    strncpy(result, text, len);
    result[len] = 0;
    return 0;
}

// ------------------------------------------------------------------
// Dump the whole DB to stdout.
//

void ip_db_dump(ip_db_t *db)
{
    fprintf(stderr, "extended=%u, index_num=%u\n", db->extended, db->index_num);

    char buf[65536];
    uint i = 0;

    for (; i < db->index_num; ++i) {
        uint ip_val = ip_db_index_get_ip(db, i);
        struct in_addr in;
        in.s_addr = htonl(ip_val);
        char *ip = inet_ntoa(in);

        uint offset = ip_db_index_get_offset(db, i);
        uint len = ip_db_index_get_text_len(db, i);
        const char *text = ip_db_get_text(db, offset);

#ifdef DEBUG
        printf("idx=%u,offset=%u,len=%u\n", i, offset, len);
#endif
        strncpy(buf, text, len);
        buf[len] = 0;

        printf("%s\t\t%s\n", ip, buf);
    }
}

