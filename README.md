# iploc (IP Locator)

The [17MON IP database](https://www.ipip.net) is widely adopted by Chinese Internet corporations. 
It's being actively built and improved from massive probing data, so accuracy is advocated. 
iploc provides a minimalistic set of C API that efficiently parses 17monipdb.dat and searches for an 
IP's location infomation.

## Initiative

Why do I bother with creating a new C API for the 17monipdb given the fact that
there've already been [official](https://www.ipip.net/download.html#ip_code) and
[3rd-party](https://www.ipip.net/download.html#ip_code2) ones?

* `The official C API is buggy and inefficient to some extent`
* `Existing 3rd-party C/C++ APIs are badly tasted`

## Example

```c
//
// Use ip_db_init_x instead if you have an extended version
// of 17MON IP DB file. Then the remaining part is the same.
//
// ip_db_t *ipdb = ip_db_init_x("17monipdb.datx");
//

ip_db_t *ipdb = ip_db_init("17monipdb.dat");

char buf[256];
if (ip_locate(ipdb, "8.8.8.8", buf) == 0) {
    printf("8.8.8.8 -> %s\n", buf);
}

ip_db_destroy(&ipdb);
```

## Benchmark
* CPU: 2.6 GHz Intel Core i5
* OS: Ubuntu 14.04 LTS
```
$ make test
8.8.8.8 -> GOOGLE       GOOGLE
random_ip_bench:        5000000 ops     684.42 msec     136 nsec/op
```

## Note
The 17monipdb.dat in this repo is for test purpose only, and it's probably outdated.
For any real-world usage, you should download the latest one from [official site](https://www.ipip.net).

