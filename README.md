# Longest Prefix Match (LPM) library

!!! WORK IN PROGRESS !!!

Longest Prefix Match (LPM) library supporting IPv4 and IPv6.

The implementation is written in C11 and is distributed under the
2-clause BSD license.

## API

* `lpm_t *lpm_create()`
  * Construct a new LPM object.

* `void lpm_destroy(lpm_t *lpm)`
  * Destroy the LPM object and any entries in it.

* `void lpm_flush(lpm_t *lpm)`
  * Remove all entries in the LPM object.

* `int lpm_insert(lpm_t *lpm, const void *addr, size_t len, unsigned preflen, void *val)`
  * Insert the network address of a given length and prefix length into
  the LPM object and associate the entry with specified pointer value.
  Note: the address must be in the network byte order.  Returns 0 on
  success or -1 on failure.

* `int lpm_remove(lpm_t *lpm, const void *addr, size_t len, unsigned preflen)`
  * Remove the network address of a given length and prefix length from
  the LPM object.  Returns 0 on success or -1 on failure.

* `void *lpm_lookup(lpm_t *lpm, const void *addr, size_t len)`
  * Lookup the given address performing the longest prefix match.
  Returns the associated pointer value on success or `NULL` on failure.

* `int lpm_strtobin(const char *cidr, void *addr, size_t *len, unsigned *preflen)`
  * Convert a string in CIDR notation to a binary address, to be stored in
  the `addr` buffer and its length in `len`, as well as the prefix length (if
  not specified, then the maximum length of the address family will be set).
  The address is stored in the network byte order and its buffer provide at
  least 4 or 16 bytes (depending on the address family).  Returns zero on
  success and -1 on failure.
