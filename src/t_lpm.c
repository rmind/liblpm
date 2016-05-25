/*
 * This file is in the Public Domain.
 */

#include <stdio.h>
#include <inttypes.h>
#include <assert.h>

#include "lpm.h"

static void
ipv4_basic_test(void)
{
	lpm_t *lpm;
	uint32_t addr;
	size_t len;
	unsigned pref;
	void *val;
	int ret;

	lpm = lpm_create();
	assert(lpm != NULL);

	/*
	 * Single /24 network.
	 */

	lpm_strtobin("10.1.1.0/24", &addr, &len, &pref);
	ret = lpm_insert(lpm, &addr, len, pref, (void *)0x100);
	assert(ret == 0);

	lpm_strtobin("10.1.1.64", &addr, &len, &pref);
	val = lpm_lookup(lpm, &addr, len);
	assert(val == (void *)0x100);

	/*
	 * Two adjacent /25 networks.
	 */

	lpm_strtobin("10.2.2.0/25", &addr, &len, &pref);
	ret = lpm_insert(lpm, &addr, len, pref, (void *)0x101);
	assert(ret == 0);

	lpm_strtobin("10.2.2.128/25", &addr, &len, &pref);
	ret = lpm_insert(lpm, &addr, len, pref, (void *)0x102);
	assert(ret == 0);

	lpm_strtobin("10.2.2.127", &addr, &len, &pref);
	val = lpm_lookup(lpm, &addr, len);
	assert(val == (void *)0x101);

	lpm_strtobin("10.2.2.255", &addr, &len, &pref);
	val = lpm_lookup(lpm, &addr, len);
	assert(val == (void *)0x102);

	/*
	 * Longer /31 prefix in /29.
	 */

	lpm_strtobin("10.3.3.60/31", &addr, &len, &pref);
	ret = lpm_insert(lpm, &addr, len, pref, (void *)0x103);
	assert(ret == 0);

	lpm_strtobin("10.3.3.56/29", &addr, &len, &pref);
	ret = lpm_insert(lpm, &addr, len, pref, (void *)0x104);
	assert(ret == 0);

	lpm_strtobin("10.3.3.61", &addr, &len, &pref);
	val = lpm_lookup(lpm, &addr, len);
	assert(val == (void *)0x103);

	lpm_strtobin("10.3.3.62", &addr, &len, &pref);
	val = lpm_lookup(lpm, &addr, len);
	assert(val == (void *)0x104);

	/*
	 * No match (out of range by one).
	 */

	lpm_strtobin("10.3.3.55", &addr, &len, &pref);
	val = lpm_lookup(lpm, &addr, len);
	assert(val == NULL);

	lpm_strtobin("10.3.3.64", &addr, &len, &pref);
	val = lpm_lookup(lpm, &addr, len);
	assert(val == NULL);

	/*
	 * Default.
	 */
	lpm_strtobin("0.0.0.0/0", &addr, &len, &pref);
	ret = lpm_insert(lpm, &addr, len, pref, (void *)0x105);
	assert(ret == 0);

	lpm_strtobin("10.3.3.64", &addr, &len, &pref);
	val = lpm_lookup(lpm, &addr, len);
	assert(val == (void *)0x105);

	lpm_destroy(lpm);
}

int
main(void)
{
	ipv4_basic_test();
	puts("ok");
	return 0;
}
