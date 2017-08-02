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
#include <sys/time.h>
#include <time.h>
#include <string.h>

#include "iploc.h"

ip_db_t *ipdb;

void test_basic(int argc, const char *argv[])
{
    char buf[65536];
    if (ip_locate(ipdb, "8.8.8.8", buf) == 0) {
        printf("8.8.8.8 -> %s\n", buf);
    }

    if (argc > 1 && ip_locate(ipdb, argv[1], buf) == 0) {
        printf("%s -> %s\n", argv[1], buf);
    }
}

#define PANIC(reason) do { \
    printf("%s\n", reason); \
    exit(-1); \
} while(0)

void get_time(struct timespec *tp)
{
    clockid_t cid = CLOCK_REALTIME;
    if (clock_gettime(cid, tp) != 0) {
        PANIC("failed to get nano time");
    }
}

long time_diff(struct timespec *x, struct timespec *y)
{
    return (x->tv_sec - y->tv_sec) * 1e9 + (x->tv_nsec - y->tv_nsec);
}

typedef void (*Action)(void *arg);

void benchmark(const char *name, int N, Action action, void *arg)
{
    struct timespec start, stop;
    get_time(&start);

    int i;
    for (i = 0; i < N; ++i) {
        action(arg);
    }

    get_time(&stop);
    double nanosec = time_diff(&stop, &start);
    printf("%s\t%d ops\t%.2f msec\t%ld nsec/op\n",
            name, N, nanosec/1e6, (long)nanosec/N);
}

void random_ip_location(void *arg)
{
    char result[256];
    uint32_t ip = (uint32_t)rand() + 1;

    if (ip_locate_v(ipdb, ip, result) != 0) {
        PANIC("failed to locate ip");
    }
}

int main(int argc, const char *argv[])
{
    srand(time(0));

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
    }

    if (!ipdb) {
        fprintf(stderr, "Failed to init ip db");
        return -1;
    }

    test_basic(argc, argv);

    int n = 5000000;
    benchmark("random_ip_bench:", n, random_ip_location, NULL);

    ip_db_destroy(&ipdb);
    return 0;
}

