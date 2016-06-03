/*
 * Copyright (c) 2016 Henry Rodrick <henry at holodisc org uk>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include <inttypes.h>

#include "org_netbsd_liblpm_LPM.h"
#include "lpm.h"

void dtor(void *, const void *, size_t, void *);

void dtor(void *arg, const void *key, size_t len, void *val)
{
	JNIEnv *env = (JNIEnv*)arg;
	(*env)->DeleteWeakGlobalRef(env, val);
}

JNIEXPORT jlong JNICALL Java_org_netbsd_liblpm_LPM_init
  (JNIEnv *env, jobject obj)
{
	return (jlong) lpm_create();
}

JNIEXPORT void JNICALL Java_org_netbsd_liblpm_LPM_destroy
  (JNIEnv *env, jobject obj, jlong lpm_ref)
{
	lpm_t* lpm = (lpm_t*)lpm_ref;
	lpm_clear(lpm, dtor, env);
	lpm_destroy(lpm);
}

JNIEXPORT void JNICALL Java_org_netbsd_liblpm_LPM_clear
  (JNIEnv *env, jobject obj, jlong lpm_ref)
{
	lpm_clear((lpm_t*)lpm_ref, dtor, env);
}


JNIEXPORT jint JNICALL Java_org_netbsd_liblpm_LPM_insert
  (JNIEnv *env, jobject obj, jlong lpm_ref, jstring cidr, jobject value)
{
	lpm_t *lpm;
	jobject val_ref;
	const char *cidr_s;
	uint32_t addr[4];
	size_t len;
	unsigned pref;
	int ret;

	lpm = (lpm_t*)lpm_ref;

	cidr_s = (*env)->GetStringUTFChars(env, cidr, NULL);
	if (cidr_s == NULL) {
		return -1;
	}

	ret = lpm_strtobin(cidr_s, addr, &len, &pref);
	if (ret != 0) {
		(*env)->ReleaseStringUTFChars(env, cidr, cidr_s);
		return ret;

	}

	val_ref = (*env)->NewWeakGlobalRef(env, value);
	if (val_ref == NULL) {
		(*env)->ReleaseStringUTFChars(env, cidr, cidr_s);
		return -1;
	}
	ret = lpm_insert(lpm, addr, len, pref, (void *)val_ref);
	if (ret != 0) {
		(*env)->DeleteWeakGlobalRef(env, val_ref);
		return ret;
	}

	(*env)->ReleaseStringUTFChars(env, cidr, cidr_s);

	return ret;
}

JNIEXPORT jobject JNICALL Java_org_netbsd_liblpm_LPM_lookup
  (JNIEnv *env, jobject obj, jlong lpm_ref, jstring addr)
{
	lpm_t *lpm;
	const char *addr_s;
	uint32_t addr_buf[4];
	size_t len;
	unsigned pref;
	void *ret;

	lpm = (lpm_t*)lpm_ref;

	addr_s = (*env)->GetStringUTFChars(env, addr, NULL);
	if (addr_s == NULL) {
		/* XXX would be better to throw an exception in this case */
		return NULL;
	}

	if (lpm_strtobin(addr_s, addr_buf, &len, &pref) != 0) {
		(*env)->ReleaseStringUTFChars(env, addr, addr_s);
		return NULL;
	}
	ret = lpm_lookup(lpm, addr_buf, len);

	(*env)->ReleaseStringUTFChars(env, addr, addr_s);

	return ret;
}

JNIEXPORT jint JNICALL Java_org_netbsd_liblpm_LPM_remove
  (JNIEnv *env, jobject obj, jlong lpm_ref, jstring addr)
{
	lpm_t *lpm;
	const char *addr_s;
	uint32_t addr_buf[4];
	size_t len;
	unsigned pref;
	int ret;
	void *val;

	lpm = (lpm_t*)lpm_ref;

	addr_s = (*env)->GetStringUTFChars(env, addr, NULL);
	if (addr_s == NULL) {
		/* XXX would be better to throw an exception in this case */
		return -1;
	}

	ret = lpm_strtobin(addr_s, addr_buf, &len, &pref);
	if (ret != 0) {
		(*env)->ReleaseStringUTFChars(env, addr, addr_s);
		return ret;
	}

	val = lpm_lookup(lpm, addr_buf, len);
	if (val == NULL) {
		(*env)->ReleaseStringUTFChars(env, addr, addr_s);
		return -1;
	}

	ret = lpm_remove(lpm, addr_buf, len, pref);

	(*env)->ReleaseStringUTFChars(env, addr, addr_s);
	(*env)->DeleteWeakGlobalRef(env, val);

	return ret;
}

