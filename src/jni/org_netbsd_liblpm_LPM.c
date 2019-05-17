/*
 * Copyright (c) 2016 Henry Rodrick <henry at holodisc org uk>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#include <inttypes.h>
#include <assert.h>

#include "org_netbsd_liblpm_LPM.h"
#include "lpm.h"

static void
lpm_jni_dtor(void *arg, const void *key, size_t len, void *val)
{
	JNIEnv *env = (JNIEnv*)arg;
	(*env)->DeleteGlobalRef(env, val);
}

JNIEXPORT jlong JNICALL
Java_org_netbsd_liblpm_LPM_init(JNIEnv *env, jobject obj)
{
	return (jlong)lpm_create();
}

JNIEXPORT void JNICALL
Java_org_netbsd_liblpm_LPM_destroy(JNIEnv *env, jobject obj, jlong lpm_ref)
{
	lpm_t *lpm = (lpm_t *)lpm_ref;
	lpm_clear(lpm, lpm_jni_dtor, env);
	lpm_destroy(lpm);
}

JNIEXPORT void JNICALL
Java_org_netbsd_liblpm_LPM_clear(JNIEnv *env, jobject obj, jlong lpm_ref)
{
	lpm_clear((lpm_t *)lpm_ref, lpm_jni_dtor, env);
}

JNIEXPORT jint JNICALL
Java_org_netbsd_liblpm_LPM_insert__JLjava_lang_String_2Ljava_lang_Object_2
    (JNIEnv *env, jobject obj, jlong lpm_ref, jstring cidr, jobject value)
{
	lpm_t *lpm = (lpm_t *)lpm_ref;
	jobject val_ref, old_val_ref;
	const char *cidr_s;
	uint32_t addr[4];
	size_t len;
	unsigned pref;
	int ret;

	cidr_s = (*env)->GetStringUTFChars(env, cidr, NULL);
	if (cidr_s == NULL) {
		return -1;
	}
	ret = lpm_strtobin(cidr_s, addr, &len, &pref);
	(*env)->ReleaseStringUTFChars(env, cidr, cidr_s);
	if (ret != 0) {
		return ret;
	}

	old_val_ref = lpm_lookup_prefix(lpm, addr, len, pref);

	val_ref = (*env)->NewGlobalRef(env, value);
	if (val_ref == NULL) {
		return -1;
	}
	ret = lpm_insert(lpm, addr, len, pref, (void *)val_ref);
	if (ret != 0) {
		(*env)->DeleteGlobalRef(env, val_ref);
	} else if (old_val_ref != NULL) {
		(*env)->DeleteGlobalRef(env, old_val_ref);
	}
	return ret;
}

JNIEXPORT jint JNICALL
Java_org_netbsd_liblpm_LPM_insert__J_3BILjava_lang_Object_2
    (JNIEnv *env, jobject obj, jlong lpm_ref, jbyteArray addr_ref,
    jint pref, jobject value)
{
	lpm_t *lpm = (lpm_t *)lpm_ref;
	jobject val_ref, old_val_ref;
	jbyte *addr;
	size_t len;
	int ret;

	len = (*env)->GetArrayLength(env, addr_ref);
	assert(len == 16 || len == 4);

	addr = (*env)->GetByteArrayElements(env, addr_ref, NULL);
	if (addr == NULL) {
		return -1;
	}

	old_val_ref = lpm_lookup_prefix(lpm, addr, len, pref);

	val_ref = (*env)->NewGlobalRef(env, value);
	if (val_ref == NULL) {
		(*env)->ReleaseByteArrayElements(env, addr_ref, addr, JNI_ABORT);
		return -1;
	}
	ret = lpm_insert(lpm, addr, len, pref, (void *)val_ref);
	if (ret != 0) {
		(*env)->DeleteGlobalRef(env, val_ref);
	} else if (old_val_ref != NULL) {
		(*env)->DeleteGlobalRef(env, old_val_ref);
	}
	(*env)->ReleaseByteArrayElements(env, addr_ref, addr, JNI_ABORT);

	return ret;
}

JNIEXPORT jobject JNICALL JNICALL
Java_org_netbsd_liblpm_LPM_lookup__JLjava_lang_String_2(JNIEnv *env,
    jobject obj, jlong lpm_ref, jstring addr)
{
	lpm_t *lpm = (lpm_t *)lpm_ref;
	const char *addr_s;
	uint32_t addr_buf[4];
	size_t len;
	unsigned pref;

	addr_s = (*env)->GetStringUTFChars(env, addr, NULL);
	if (addr_s == NULL) {
		/* XXX would be better to throw an exception in this case */
		return NULL;
	}

	if (lpm_strtobin(addr_s, addr_buf, &len, &pref) != 0) {
		(*env)->ReleaseStringUTFChars(env, addr, addr_s);
		return NULL;
	}
	(*env)->ReleaseStringUTFChars(env, addr, addr_s);

	return lpm_lookup(lpm, addr_buf, len);
}

JNIEXPORT jobject JNICALL
Java_org_netbsd_liblpm_LPM_lookup__J_3B(JNIEnv *env, jobject obj,
    jlong lpm_ref, jbyteArray addr_ref)
{
	lpm_t *lpm = (lpm_t *)lpm_ref;
	jbyte *addr;
	size_t len;
	void *ret;

	len = (*env)->GetArrayLength(env, addr_ref);
	assert(len == 16 || len == 4);

	addr = (*env)->GetByteArrayElements(env, addr_ref, NULL);
	if (addr == NULL) {
		return NULL;
	}
	ret = lpm_lookup(lpm, addr, len);
	(*env)->ReleaseByteArrayElements(env, addr_ref, addr, JNI_ABORT);
	return ret;
}

JNIEXPORT jint JNICALL
Java_org_netbsd_liblpm_LPM_remove__JLjava_lang_String_2(JNIEnv *env,
    jobject obj, jlong lpm_ref, jstring addr)
{
	lpm_t *lpm = (lpm_t *)lpm_ref;
	const char *addr_s;
	uint32_t addr_buf[4];
	size_t len;
	unsigned pref;
	int ret;
	void *val;

	addr_s = (*env)->GetStringUTFChars(env, addr, NULL);
	if (addr_s == NULL) {
		/* XXX would be better to throw an exception in this case */
		return -1;
	}
	ret = lpm_strtobin(addr_s, addr_buf, &len, &pref);
	(*env)->ReleaseStringUTFChars(env, addr, addr_s);
	if (ret != 0) {
		return ret;
	}

	val = lpm_lookup_prefix(lpm, addr_buf, len, pref);
	if (val == NULL) {
		return -1;
	}
	ret = lpm_remove(lpm, addr_buf, len, pref);
	(*env)->DeleteGlobalRef(env, val);
	return ret;
}

JNIEXPORT jint JNICALL
Java_org_netbsd_liblpm_LPM_remove__J_3BI(JNIEnv *env, jobject obj,
    jlong lpm_ref, jbyteArray addr_ref, jint pref)
{
	lpm_t *lpm = (lpm_t *)lpm_ref;
	jbyte *addr;
	void *val;
	size_t len;
	int ret;

	len = (*env)->GetArrayLength(env, addr_ref);
	assert(len == 16 || len == 4);

	addr = (*env)->GetByteArrayElements(env, addr_ref, NULL);
	if (addr == NULL) {
		return -1;
	}

	val = lpm_lookup_prefix(lpm, addr, len, pref);
	if (val == NULL) {
		(*env)->ReleaseByteArrayElements(env, addr_ref, addr, JNI_ABORT);
		return -1;
	}

	ret = lpm_remove(lpm, addr, len, pref);

	(*env)->DeleteGlobalRef(env, val);
	(*env)->ReleaseByteArrayElements(env, addr_ref, addr, JNI_ABORT);

	return ret;
}
