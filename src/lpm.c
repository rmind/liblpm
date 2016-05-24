/*
 * Copyright (c) 2016 Mindaugas Rasiukevicius <rmind at noxt eu>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <assert.h>

#include "lpm.h"

#define	MAXPREF_TO_ALEN(x)	((x) >> 3)
#define	ALEN_TO_NWORDS(x)	((x) >> 2)

#define	LPM_MAX_PREFIX		(128)
#define	LPM_MAX_WORDS		(LPM_MAX_PREFIX >> 5)
#define	LPM_HASH_STEP		(8)

#ifdef DEBUG
#define	ASSERT	assert
#else
#define	ASSERT
#endif

typedef struct lpm_ent {
	struct lpm_ent *next;
	void *		val;
	uint8_t		key[];
} lpm_ent_t;

typedef struct {
	unsigned	hashsize;
	unsigned	nitems;
	lpm_ent_t **	bucket;
} lpm_hmap_t;

struct lpm {
	uint32_t	bitmask[LPM_MAX_WORDS];
	unsigned	maxpref;
	void *		defval;
	lpm_hmap_t	prefix[];
};

lpm_t *
lpm_create(lpm_af_t af)
{
	unsigned maxpref;
	lpm_t *lpm;

	switch (af) {
	case LPM_INET4:
		maxpref = 32;
		break;
	case LPM_INET6:
		maxpref = 128;
		break;
	default:
		return NULL;
	}
	lpm = calloc(1, offsetof(struct lpm, prefix[maxpref]));
	if (!lpm) {
		return NULL;
	}
	lpm->maxpref = maxpref;
	return lpm;
}

void
lpm_destroy(lpm_t *lpm)
{
	for (unsigned n = 0; n < lpm->maxpref; n++) {
		lpm_hmap_t *hmap = &lpm->prefix[n];

		if (!hmap->hashsize) {
			ASSERT(!hmap->bucket);
			continue;
		}
		for (unsigned i = 0; i < hmap->hashsize; i++) {
			lpm_ent_t *entry = hmap->bucket[i];

			while (entry) {
				lpm_ent_t *next = entry->next;

				free(entry);
				entry = next;
			}
		}
		free(hmap->bucket);
	}
	free(lpm);
}

/*
 * fnv1a_hash: Fowler-Noll-Vo hash function (FNV-1a variant).
 */
static uint32_t
fnv1a_hash(const void *buf, size_t len)
{
	uint32_t hash = 2166136261UL;
	const uint8_t *p = buf;

	while (len--) {
		hash ^= *p++;
		hash *= 16777619U;
	}
	return hash;
}

static bool
hashmap_rehash(lpm_hmap_t *hmap, unsigned size, size_t len)
{
	lpm_ent_t **bucket;
	unsigned hashsize;

	for (hashsize = 1; hashsize < size; hashsize <<= 1) {
		continue;
	}
	if ((bucket = calloc(1, hashsize * sizeof(lpm_ent_t *))) == NULL) {
		return false;
	}
	for (unsigned n = 0; n < hmap->hashsize; n++) {
		lpm_ent_t *list = hmap->bucket[n];

		while (list) {
			lpm_ent_t *entry = list;
			const uint32_t hash = fnv1a_hash(entry->key, len);
			const unsigned i = hash & (hashsize - 1);

			list = entry->next;
			entry->next = hmap->bucket[i];
			bucket[i] = entry;
		}
	}
	hmap->hashsize = hashsize;
	hmap->bucket = bucket;
	return true;
}

static lpm_ent_t *
hashmap_insert(lpm_hmap_t *hmap, const void *key, size_t len)
{
	const unsigned target = hmap->nitems + LPM_HASH_STEP;
	const size_t entlen = offsetof(lpm_ent_t, key[len]);
	lpm_ent_t *entry;

	if (hmap->hashsize < target && !hashmap_rehash(hmap, target, len)) {
		return NULL;
	}
	if ((entry = malloc(entlen)) != NULL) {
		const uint32_t hash = fnv1a_hash(key, len);
		const unsigned i = hash & (hmap->hashsize - 1);

		memcpy(entry->key, key, len);
		entry->next = hmap->bucket[i];
		hmap->bucket[i] = entry;
		hmap->nitems++;
	}
	return entry;
}

static lpm_ent_t *
hashmap_lookup(lpm_hmap_t *hmap, const void *key, size_t len)
{
	const uint32_t hash = fnv1a_hash(key, len);
	const unsigned i = hash & (hmap->hashsize - 1);
	lpm_ent_t *entry = hmap->bucket[i];

	while (entry) {
		if (memcmp(entry->key, key, len) == 0) {
			return entry;
		}
		entry = entry->next;
	}
	return NULL;
}

/*
 * compute_prefix: given the address and prefix length, compute and
 * return the address prefix.
 */
static inline void
compute_prefix(const unsigned nwords, const uint32_t *addr,
    unsigned preflen, uint32_t *prefix)
{
	for (unsigned i = 0; i < nwords; i++) {
		if (preflen == 0) {
			prefix[i] = 0;
			continue;
		}
		if (preflen < 32) {
			uint32_t mask = htonl(0xffffffff << (32 - preflen));
			prefix[i] = addr[i] & mask;
			preflen = 0;
		} else {
			prefix[i] = addr[i];
			preflen -= 32;
		}
	}
}

/*
 * lpm_add: insert the CIDR into the LPM table.
 *
 * => Returns zero on success and -1 on failure.
 */
int
lpm_add(lpm_t *lpm, const uint32_t *addr, unsigned preflen, void *val)
{
	const unsigned alen = MAXPREF_TO_ALEN(lpm->maxpref);
	const unsigned nwords = ALEN_TO_NWORDS(alen);
	uint32_t prefix[nwords];
	lpm_ent_t *entry;

	if (preflen == 0) {
		/* Default is a special case. */
		lpm->defval = val;
		return 0;
	}
	compute_prefix(nwords, addr, preflen, prefix);
	entry = hashmap_insert(&lpm->prefix[preflen], prefix, alen);
	if (entry) {
		const unsigned n = --preflen >> 5;
		lpm->bitmask[n] |= 1U << (preflen & 31);
		entry->val = val;
		return 0;
	}
	return -1;
}

/*
 * lpm_lookup: find the longest matching prefix given the IP address.
 *
 * => Returns the associated value on success or NULL on failure.
 */
void *
lpm_lookup(lpm_t *lpm, const uint32_t *addr)
{
	const unsigned alen = MAXPREF_TO_ALEN(lpm->maxpref);
	const unsigned nwords = ALEN_TO_NWORDS(alen);
	uint32_t prefix[nwords];

	for (unsigned i, n = 0; n < nwords; n++) {
		while ((i = ffs(lpm->bitmask[n])) != 0) {
			const unsigned preflen = (32 * n) + i;
			lpm_hmap_t *hmap = &lpm->prefix[preflen];
			lpm_ent_t *entry;

			compute_prefix(nwords, addr, preflen, prefix);
			entry = hashmap_lookup(hmap, prefix, alen);
			if (entry) {
				return entry->val;
			}
			lpm->bitmask[n] &= ~(1U << --i);
		}
	}
	return lpm->defval;
}

/*
 * lpm_strtobin: convert CIDR string to the binary IP address and mask.
 *
 * => The address will be in the network byte order.
 * => Returns 0 on success or -1 on failure.
 */
int
lpm_strtobin(const char *cidr, lpm_af_t *af, uint32_t *addr, unsigned *preflen)
{
	char *p, buf[INET6_ADDRSTRLEN];

	strncpy(buf, cidr, sizeof(buf));
	buf[sizeof(buf) - 1] = '\0';

	if ((p = strchr(buf, '/')) != NULL) {
		const ptrdiff_t off = p - buf;
		*preflen = atoi(&buf[off + 1]);
		buf[off] = '\0';
	} else {
		*preflen = 128;
	}

	if (inet_pton(AF_INET6, buf, addr) == 1) {
		*af = LPM_INET6;
		return 0;
	}
	if (inet_pton(AF_INET, buf, addr) == 1) {
		if (*preflen == 128) {
			*preflen = 32;
		}
		*af = LPM_INET4;
		return 0;
	}
	return -1;
}
