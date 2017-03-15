# iploc (IP Locator)

The [17MON IP database](https://www.ipip.net) is being actively built from massive probing data, and is in return 
widely used by Chinese Internet corporations. iploc provides a minimalistic set of C API that parses 17monipdb.dat 
and searches for an IP's location infomation.

## Initiative

Why do I bother with creating a new C API for the 17monipdb given the fact that
there've already been [official](https://www.ipip.net/download.html#ip_code) and
[3rd-party](https://www.ipip.net/download.html#ip_code2) ones?

* `The official C API is buggy and inefficient to some extent`
* `The code of a 3rd-party C API investigated is badly tasted`

## Example

```c
ip_db_t *ipdb = ip_db_init("17monipdb.dat");

char buf[256];
if (ip_locate(ipdb, "8.8.8.8", buf) == 0) {
    printf("8.8.8.8 -> %s\n", buf);
}

ip_db_destroy(&ipdb);
```

## Benchmark

```
$ make test
8.8.8.8 -> GOOGLE       GOOGLE
random_ip_bench:        5000000 ops     684.42 msec     136 nsec/op
```

## Note
The 17monipdb.dat in this repo is for test purpose only, and it's probably outdated.
For any real-world usage, you should buy the commercial version or download the free
one from [official site](https://www.ipip.net).

