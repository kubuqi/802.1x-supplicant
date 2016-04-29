/*
 * state.c  To generate and verify State attribute
 *
 * $Id: state.c,v 1.7 2003/03/03 19:52:25 aland Exp $
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 */

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include "rlm_eap.h"

static const char rcsid[] = "$Id: state.c,v 1.7 2003/03/03 19:52:25 aland Exp $";

/*
 *	Global key to generate & verify State
 *
 *	FIXME: this means that we can't have multiple instances
 *	of the EAP module.
 */
static unsigned char state_key[AUTH_VECTOR_LEN];

/*
 * Generate & Verify the State attribute
 *
 * In the simplest implementation, we would just use the challenge as state.
 * Unfortunately, the RADIUS secret protects only the User-Password
 * attribute; an attacker that can remove packets from the wire and insert
 * new ones can simply insert a replayed state without having to know
 * the secret.  If not for an attacker that can remove packets from the
 * network, I believe trivial state to be secure.
 *
 * So, we have to make up for that deficiency by signing our state with
 * data unique to this specific request.  A NAS would use the Request
 * Authenticator, we don't know what that will be when the State is
 * returned to us, so we'll use the time.  So our replay prevention
 * is limited to a time interval (inst->maxdelay).  We could keep
 * track of all challenges issued over that time interval for
 * better protection.
 *
 * Our state, then, is (challenge + time + hmac(challenge + time, key)),
 * where '+' denotes concatentation, 'challenge' is the ASCII octets of
 * the challenge, 'time' is the 32-bit time (LSB if time_t is 64 bits)
 * in network byte order, and 'key' is a random key, generated once in
 * eap_init().  This means that only the server which generates
 * a challenge can verify it; this should be OK if your NAS's load balance
 * across RADIUS servers by a "first available" algorithm.  If your
 * NAS's round-robin (ugh), you could use the RADIUS secret instead, but
 * read RFC 2104 first, and make very sure you really want to do this.
 *
 */

void generate_key(void)
{
	int i;

	/*
	 *	Use a cryptographically strong method to generate
	 *	pseudo-random numbers.
	 */
	for (i = 0; i < sizeof(state_key); i++) {
		state_key[i] = lrad_rand();
	}
}

/*
 * Our state, is (challenge + time + hmac(challenge + time, key))
 */
VALUE_PAIR *generate_state(void)
{
	int i;
	unsigned char challenge[AUTH_VECTOR_LEN];
	unsigned char hmac[AUTH_VECTOR_LEN];
	unsigned char value[AUTH_VECTOR_LEN+sizeof(time_t)];
	VALUE_PAIR    *state;
	time_t now;

	/* Generate challenge (a random value).  */
	for (i = 0; i < sizeof(challenge); i++) {
		challenge[i] = lrad_rand();
	}

	now = time(NULL);
	memcpy(value, challenge, AUTH_VECTOR_LEN);
	memcpy(value + AUTH_VECTOR_LEN, &now, sizeof(time_t));

	/* Generate hmac.  */
	lrad_hmac_md5(value, AUTH_VECTOR_LEN + sizeof(time_t),
              state_key, AUTH_VECTOR_LEN, hmac);


	/* Create state attribute.  */
	state = paircreate(PW_STATE, PW_TYPE_OCTETS);
	if (state == NULL) {
		radlog(L_ERR, "rlm_eap: out of memory");
		return NULL;
	}
	memcpy(state->strvalue, value, AUTH_VECTOR_LEN+sizeof(time_t));
	memcpy(state->strvalue+AUTH_VECTOR_LEN+sizeof(time_t), hmac, AUTH_VECTOR_LEN);
	state->length = AUTH_VECTOR_LEN + sizeof(time_t) + AUTH_VECTOR_LEN;

	return state;
}

/*
 * Returns 0 on success, non-zero otherwise. 
 */
int verify_state(VALUE_PAIR *state)
{
	unsigned char prev_hmac[AUTH_VECTOR_LEN];
	unsigned char hmac[AUTH_VECTOR_LEN];
	unsigned char value[AUTH_VECTOR_LEN+sizeof(time_t)];
	
	/* Get the challenge value & hmac from the State */
	memcpy(value, state->strvalue, AUTH_VECTOR_LEN+sizeof(time_t));
	memcpy(prev_hmac, state->strvalue+AUTH_VECTOR_LEN+sizeof(time_t), AUTH_VECTOR_LEN);
	
	/* Generate hmac.  */
	lrad_hmac_md5(value, AUTH_VECTOR_LEN + sizeof(time_t),
              state_key, AUTH_VECTOR_LEN, hmac);

	/* verify both the hmacs */
	return memcmp(hmac, prev_hmac, AUTH_VECTOR_LEN);
}
