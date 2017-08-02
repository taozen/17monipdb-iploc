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

#include "iploc.h"

int main(int argc, const char *argv[])
{
    ip_db_t *ipdb = NULL;

    if (argc == 1) {
        ipdb = ip_db_init("17monipdb.dat");
    } else if (argc == 2) {
        if (strcmp(argv[1], "-x") == 0) {
            ipdb = ip_db_init_x("17monipdb.datx");
        } else {
            ipdb = ip_db_init(argv[1]);
        }
    } else if (argc == 3 && strcmp(argv[1], "-x") == 0) {
        ipdb = ip_db_init_x(argv[2]);
    } else {
        fprintf(stderr, "Usage: %s [-x] [IP DB file]\n", argv[0]);
        return 1;
    }

    if (!ipdb) {
        fprintf(stderr, "Failed to initialize ipdb\n");
        return -1;
    }

    char ip[32];
    char info[65536];
    printf("Enter an IP address like 8.8.8.8 to query its location.\n");
    printf("Enter q or quit to exit.\n>>> ");

    while (fgets(ip, sizeof(ip), stdin)) {
        int n = strlen(ip);
        if (ip[n-1] == '\n') {
            ip[n-1] = 0;
            n--;
        }

        if (n == 0) {
            printf(">>> ");
            continue;
        }

        if (strcmp(ip, "q") == 0 || strcmp(ip, "quit") == 0) {
            break;
        }

        printf("%s -> ", ip);
        if (ip_locate(ipdb, ip, info) == 0) {
            printf("%s\n>>> ", info);
        } else {
            printf("no location info\n>>> ");
        }
    }

    ip_db_destroy(&ipdb);
    printf("\nExit loop...\n");
    return 0;
}

