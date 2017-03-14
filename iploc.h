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

#pragma once

#include <stdint.h>

typedef struct _ip_db_t ip_db_t;

//
// ip_db_init creates and then initializes an ip_db_t object using the
// given 17MON DB file. The returned object must be destroied via
// ip_db_destroy when no longer needed.
//
ip_db_t* ip_db_init(const char *path);

//
// ip_db_destroy destroies an ip_db_t object and reclaim all memory 
// allocated underneath.
//
void ip_db_destroy(ip_db_t **db);

//
// ip_locate searches the specified IP in DB and copies its location 
// description into the given result buffer if found.
//
int ip_locate(ip_db_t *db, const char *ipv4, char *result);

//
// ip_locate_v searches the specified IP (value in host representation)
// in DB. If found, 0 is returned with its location description copied
// into the given result buffer, -1 otherwise.
//
int ip_locate_v(ip_db_t *db, uint32_t ip_val, char *result);

