/*
 * Copyright (c) 2016 Mindaugas Rasiukevicius <rmind at noxt eu>
 * All rights reserved.
 *
 * Use is subject to license terms, as specified in the LICENSE file.
 */

#ifndef _LPM_H_
#define _LPM_H_

__BEGIN_DECLS

typedef struct lpm lpm_t;
typedef enum { LPM_INET4, LPM_INET6 } lpm_af_t;

lpm_t *		lpm_create(lpm_af_t);
void		lpm_destroy(lpm_t *);
int		lpm_add(lpm_t *, const uint32_t *, unsigned, void *);
void *		lpm_lookup(lpm_t *, const uint32_t *);
int		lpm_strtobin(const char *, lpm_af_t *, uint32_t *, unsigned *);

__END_DECLS

#endif
