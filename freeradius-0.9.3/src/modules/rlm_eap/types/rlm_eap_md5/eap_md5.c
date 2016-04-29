/*
 * eap_md5.c  EAP MD5 functionality.
 *
 * Version:     $Id: eap_md5.c,v 1.8 2003/05/14 14:33:57 aland Exp $
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
 * Copyright 2000,2001  The FreeRADIUS server project
 * Copyright 2001  hereUare Communications, Inc. <raghud@hereuare.com>
 */

/*
 *
 *  MD5 Packet Format in EAP Type-Data
 *  --- ------ ------ -- --- --------- 
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Value-Size   |  Value ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |  Name ...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "eap.h"

#include "eap_md5.h"

/*
 *      Allocate a new MD5_PACKET
 */
MD5_PACKET *eapmd5_alloc(void)
{
	MD5_PACKET   *rp;

	if ((rp = malloc(sizeof(MD5_PACKET))) == NULL) {
		radlog(L_ERR, "rlm_eap_md5: out of memory");
		return NULL;
	}
	memset(rp, 0, sizeof(MD5_PACKET));
	return rp;
}

/*
 *      Free MD5_PACKET
 */
void eapmd5_free(MD5_PACKET **md5_packet_ptr)
{
	MD5_PACKET *md5_packet;

	if (!md5_packet_ptr) return;
	md5_packet = *md5_packet_ptr;
	if (md5_packet == NULL) return;

	if (md5_packet->value) free(md5_packet->value);
	if (md5_packet->name) free(md5_packet->name);

	free(md5_packet);

	*md5_packet_ptr = NULL;
}

/* 
 * We expect only RESPONSE for which CHALLENGE, SUCCESS or FAILURE is sent back
 */ 
MD5_PACKET *eapmd5_extract(EAP_DS *eap_ds)
{
	md5_packet_t	*data;
	MD5_PACKET	*packet;
	unsigned short	name_len;

	if (!eap_ds 					|| 
	    !eap_ds->response 				|| 
	    (eap_ds->response->code != PW_MD5_RESPONSE)	||
	    eap_ds->response->type.type != PW_EAP_MD5	||
	    !eap_ds->response->type.data 		||
	    (eap_ds->response->length < MD5_HEADER_LEN)	||
	    (eap_ds->response->type.data[0] <= 0)	) {
	  
		radlog(L_ERR, "rlm_eap_md5: corrupted data");
		return NULL;
	}

	packet = eapmd5_alloc();
	if (!packet) return NULL;

	/*
	 * Code, id & length for MD5 & EAP are same
	 * but md5_length = eap_length - 1(Type = 1 octet)
	 */
	packet->code = eap_ds->response->code;
	packet->id = eap_ds->response->id;
	packet->length = eap_ds->response->length - 1;
	packet->value_size = 0;
	packet->value = NULL;
	packet->name = NULL;

	data = (md5_packet_t *)eap_ds->response->type.data;

	packet->value_size = data->value_size;
	if (packet->value_size < 1) {
		radlog(L_ERR, "rlm_eap_md5: Value size is too small");
		eapmd5_free(&packet);
		return NULL;
	}

	packet->value = malloc(packet->value_size);
	if (packet->value == NULL) {
		radlog(L_ERR, "rlm_eap_md5: out of memory");
		eapmd5_free(&packet);
		return NULL;
	}
	memcpy(packet->value, data->value_name, packet->value_size);

	/*
	 * Name is optional and is present after Value, but we need to check for it
	 */
	name_len =  packet->length - (packet->value_size + 5);
	if (name_len) {
		packet->name = malloc(name_len+1);
		if (!packet->name) {
			radlog(L_ERR, "rlm_eap_md5: out of memory");
			eapmd5_free(&packet);
			return NULL;
		}
		memset(packet->name, 0, name_len+1);
		memcpy(packet->name, data->value_name+packet->value_size, name_len);
	}

	return packet;
}

/*
 * Generate a random value
 * challenge = MD5(random)
 */
int eapmd5_challenge(unsigned char *value, int len)
{
	int i;

	/*
	 *	Get real pseudo-random numbers.
	 */
	for (i = 0; i < len; i++) {
		value[i] = lrad_rand();
	}
	radlog(L_INFO, "rlm_eap_md5: Issuing Challenge");

	return 1;
}

/* 
 * verify = MD5(id+password+challenge_sent)
 */
int eapmd5_verify(MD5_PACKET *packet, VALUE_PAIR* password, 
		md5_packet_t *challenge)
{
	char	*ptr;
	char	string[MAX_STRING_LEN*2];
	unsigned char output[MAX_STRING_LEN];
	unsigned short len;

	if ((password == NULL) || (challenge == NULL)) {
		return 0;
	}

	/*
	 *	Sanity check it.
	 */
	if (packet->value_size != 16) {
		radlog(L_ERR, "rlm_eap_md5: Expected 16 bytes of response to challenge, got %d", packet->value_size);
		return 0;
	}

	len = 0;
	ptr = string;

	*ptr++ = packet->id;
	len++;
	memcpy(ptr, password->strvalue, password->length);
	ptr += password->length;
	len += password->length;

	memcpy(ptr, challenge->value_name, challenge->value_size);
	len += challenge->value_size;

	librad_md5_calc((u_char *)output, (u_char *)string, len);

	/*
	 *	The length of the response is always 16 for MD5.
	 */
	if (memcmp(output, packet->value, 16) != 0) {
		return 0;
	}
	return 1;
}

/*
 * Identify whether the response that you got is either the
 * response to the challenge that we sent or a new one.
 * If it is a response to the request then issue success/failure
 * else issue a challenge
 */
MD5_PACKET *eapmd5_process(MD5_PACKET *packet, int id,
		VALUE_PAIR *username, VALUE_PAIR* password, md5_packet_t *request)
{
	unsigned char output[MAX_STRING_LEN];
	MD5_PACKET *reply;

	if (!username || !password || !packet)
		return NULL;

	reply = eapmd5_alloc();
	if (!reply) return NULL;
	memset(output, 0, MAX_STRING_LEN);
	reply->id = id;
	
	if (request) {
		/* verify and issue Success/failure */
		if (eapmd5_verify(packet, password, request) == 0) {
			radlog(L_INFO, "rlm_eap_md5: Challenge failed");
			reply->code = PW_MD5_FAILURE;
		}
		else {
			reply->code = PW_MD5_SUCCESS;
		}

	} else {
		/*
		 * Previous request not found.
		 * Probably it is timed out.
		 * So send another challenge.
		 * TODO: Later Send these challenges for the configurable
		 * 		number of times for each user & stop.
		 */

		/*
		 *	Ensure that the challenge is always of the correct
		 *	length.  i.e. Don't take value size from data
		 *	supplied by the client.
		 */
		if (reply->value_size != MD5_LEN) {
			free(reply->value);
			reply->value_size = MD5_LEN;
			reply->value = malloc(reply->value_size);
		}

		eapmd5_challenge(reply->value, reply->value_size);
		reply->code = PW_MD5_CHALLENGE;
		radlog(L_INFO, "rlm_eap_md5: Previous request not found");
		radlog(L_INFO, "rlm_eap_md5: Issuing Challenge to the user - %s",
			(char *)username->strvalue);
	}

	/* fill reply packet */
	if (reply->code == PW_MD5_CHALLENGE) {
		reply->value_size = packet->value_size;
		reply->value = malloc(reply->value_size);
		if (reply->value == NULL) {
			radlog(L_ERR, "rlm_eap_md5: out of memory");
			eapmd5_free(&reply);
			return NULL;
		}
		memcpy(reply->value, output, reply->value_size);
		reply->length = packet->length;
	} else {
		reply->length = MD5_HEADER_LEN;
	}
	
	return reply;
}

/*
 * If an EAP MD5 request needs to be initiated then
 * create such a packet.
 */
MD5_PACKET *eapmd5_initiate(EAP_DS *eap_ds)
{
	MD5_PACKET 	*reply;

	reply = eapmd5_alloc();
	if (reply == NULL)  {
		radlog(L_ERR, "rlm_eap_md5: out of memory");
		return NULL;
	}

	reply->code = PW_MD5_CHALLENGE;
	reply->length = MD5_HEADER_LEN + 1/*value_size*/ + MD5_LEN;
	reply->value_size = MD5_LEN;

	reply->value = malloc(reply->value_size);
	if (reply->value == NULL) {
		radlog(L_ERR, "rlm_eap_md5: out of memory");
		eapmd5_free(&reply);
		return NULL;
	}

	eapmd5_challenge(reply->value, reply->value_size);

	return reply;
}

/* 
 * compose the MD5 reply packet in the EAP reply typedata
 */
int eapmd5_compose(EAP_DS *eap_ds, MD5_PACKET *reply)
{
	uint8_t *ptr;
	unsigned short name_len;

	if (reply->code < 3) {

		eap_ds->request->type.type = PW_EAP_MD5;

		eap_ds->request->type.data = malloc(reply->length - MD5_HEADER_LEN);
		if (eap_ds->request->type.data == NULL) {
			radlog(L_ERR, "rlm_eap_md5: out of memory");
			return 0;
		}
		ptr = eap_ds->request->type.data;
		*ptr++ = (uint8_t)(reply->value_size & 0xFF);
		memcpy(ptr, reply->value, reply->value_size);

		/* Just the Challenge length */
		eap_ds->request->type.length = reply->value_size + 1;

		name_len = reply->length - (reply->value_size + 1 + MD5_HEADER_LEN);
		if (name_len && reply->name) {
			ptr += reply->value_size;
			memcpy(ptr, reply->name, name_len);
			/* Challenge length + Name length */
			eap_ds->request->type.length += name_len;
		}
	} else {
		eap_ds->request->type.length = 0;
		/* TODO: In future we might add message here wrt rfc1994 */
	}
	eap_ds->request->code = reply->code;

	return 1;
}
