/*
 * eap_tls.c
 *
 * Version:     $Id: eap_tls.c,v 1.13 2003/04/04 20:06:12 aland Exp $
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

/*
 *
 *  TLS Packet Format in EAP 
 *  --- ------ ------ -- ---
 * 0                   1                   2                   3
 * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Code      |   Identifier  |            Length             |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     Type      |     Flags     |      TLS Message Length
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * |     TLS Message Length        |       TLS Data...
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 */

#include "eap_tls.h"

/*
 *      Allocate a new TLS_PACKET
 */
EAPTLS_PACKET *eaptls_alloc(void)
{
	EAPTLS_PACKET   *rp;

	if ((rp = malloc(sizeof(EAPTLS_PACKET))) == NULL) {
		radlog(L_ERR, "rlm_eap_tls: out of memory");
		return NULL;
	}
	memset(rp, 0, sizeof(EAPTLS_PACKET));
	return rp;
}

/*
 *      Free EAPTLS_PACKET
 */
void eaptls_free(EAPTLS_PACKET **eaptls_packet_ptr)
{
	EAPTLS_PACKET *eaptls_packet;

	if (!eaptls_packet_ptr) return;
	eaptls_packet = *eaptls_packet_ptr;
	if (eaptls_packet == NULL) return;

	if (eaptls_packet->data) {
		free(eaptls_packet->data);
		eaptls_packet->data = NULL;
	}

	free(eaptls_packet);
	*eaptls_packet_ptr = NULL;
}

/*
   The S flag is set only within the EAP-TLS start message
   sent from the EAP server to the peer.
*/
int eaptls_start(EAP_DS *eap_ds)
{
	EAPTLS_PACKET 	reply;

	reply.code = EAPTLS_START;
	reply.length = TLS_HEADER_LEN + 1/*flags*/;

	reply.flags = 0x00;
	reply.flags = SET_START(reply.flags);

	reply.data = NULL;
	reply.dlen = 0;

	eaptls_compose(eap_ds, &reply);

	return 1;
}

int eaptls_success(EAP_DS *eap_ds)
{
	EAPTLS_PACKET	reply;

	reply.code = EAPTLS_SUCCESS;
	reply.length = TLS_HEADER_LEN;
	reply.flags = 0x00;
	reply.data = NULL;
	reply.dlen = 0;

	eaptls_compose(eap_ds, &reply);

	return 1;
}

int eaptls_fail(EAP_DS *eap_ds)
{
	EAPTLS_PACKET	reply;

	reply.code = EAPTLS_FAIL;
	reply.length = TLS_HEADER_LEN;
	reply.flags = 0x00;
	reply.data = NULL;
	reply.dlen = 0;

	eaptls_compose(eap_ds, &reply);

	return 1;
}

/*
   A single TLS record may be up to 16384 octets in length, but a TLS
   message may span multiple TLS records, and a TLS certificate message
   may in principle be as long as 16MB. 
*/
/*
 * Frame the Dirty data that needs to be send to the client in an EAP-Request.
 * We always embed the TLS-length in all EAP-TLS packets 
 * that we send, for easy reference purpose.
 * Handles fragmentation and sending the next fragment etc.,
 */
int eaptls_request(EAP_DS *eap_ds, tls_session_t *ssn)
{
	EAPTLS_PACKET	reply;
	unsigned int	size;
	unsigned int 	nlen;
	unsigned int 	lbit = 0;

	/* This value determines whether we set (L)ength flag for 
		EVERY packet we send and add corresponding 
		"TLS Message Length" field.

	length_flag = TRUE;
		This means we include L flag and "TLS Msg Len" in EVERY
		packet we send out.
       
	length_flag = FALSE;
		This means we include L flag and "TLS Msg Len" **ONLY**
		in First packet of a fragment series. We do not use 
		it anywhere else.

		Having L flag in every packet is prefered.

	*/
	if (ssn->length_flag) {
		lbit = 4;
	}
	if (ssn->fragment == 0) {
		ssn->tls_msg_len = ssn->dirty_out.used;
	}

	reply.code = EAPTLS_REQUEST;
	reply.flags = 0x00;

	/* Send data, NOT more than the FRAGMENT size */
	if (ssn->dirty_out.used > ssn->offset) {
		size = ssn->offset;
		reply.flags = SET_MORE_FRAGMENTS(reply.flags);
		/* Length MUST be included if it is the First Fragment */
		if (ssn->fragment == 0) {
			lbit = 4;
		}
		ssn->fragment = 1;
	} else {
		size = ssn->dirty_out.used;
		ssn->fragment = 0;
	}

	reply.dlen = lbit + size;
	reply.length = TLS_HEADER_LEN + 1/*flags*/ + reply.dlen;

	reply.data = malloc(reply.dlen);
	if (lbit) {
		nlen = htonl(ssn->tls_msg_len);
		memcpy(reply.data, &nlen, lbit);
		reply.flags = SET_LENGTH_INCLUDED(reply.flags);
	}
	record_minus(&ssn->dirty_out, reply.data+lbit, size);

	eaptls_compose(eap_ds, &reply);
	free(reply.data);
	reply.data = NULL;

	return 1;
}

/*
 * Acknowledge received is for one of the following messages sent earlier
 * 1. Handshake completed Message, so now send, EAP-Success
 * 2. Alert Message, now send, EAP-Failure
 * 3. Fragment Message, now send, next Fragment
 *
 * After Success/Failure, We are done with the Session, free all the resources
 */
eaptls_status_t eaptls_ack_handler(EAP_HANDLER *handler)
{
	tls_session_t *tls_session;

	tls_session = (tls_session_t *)handler->opaque;
	if ((tls_session == NULL) || (tls_session->info.origin == 0)) {

		radlog(L_ERR, "rlm_eap_tls: Unexpected ACK received");
		return EAPTLS_NOOP;
	}

	switch (tls_session->info.content_type) {

	case alert:
		eaptls_fail(handler->eap_ds);
		session_free(&handler->opaque);
		return EAPTLS_FAIL;

	case handshake:
		if (tls_session->info.handshake_type == finished) {
			eaptls_success(handler->eap_ds);
			eaptls_gen_mppe_keys(handler->reply_vps, 
					     tls_session->ssl);
			session_free(&handler->opaque);
			return EAPTLS_SUCCESS;
		} else if (tls_session->fragment > 0) {
			/* Fragmentation handler, send next fragment */
			eaptls_request(handler->eap_ds, tls_session);
			return EAPTLS_REQUEST;
		}
		/*
		 * For the rest of the conditions,
		 * switch over to the default section below.
		 */

	default:
		radlog(L_ERR, "rlm_eap_tls: Invalid ACK received");
		session_free(&handler->opaque);
		return EAPTLS_NOOP;
	}
}

/*
   Similarly, when the EAP server receives an EAP-Response with the M
   bit set, it MUST respond with an EAP-Request with EAP-Type=EAP-TLS
   and no data. This serves as a fragment ACK. 

   In order to prevent errors in the processing of fragments, the EAP
   server MUST use increment the Identifier value for each fragment ACK
   contained within an EAP-Request, and the peer MUST include this
   Identifier value in the subsequent fragment contained within an EAP-
   Reponse.

 * EAP server sends an ACK when it determines there are More fragments 
 * to receive to make the complete TLS-record/TLS-Message
 */
int eaptls_send_ack(EAP_DS *eap_ds)
{
	EAPTLS_PACKET 	reply;

	reply.code = EAPTLS_ACK;
	reply.length = TLS_HEADER_LEN + 1/*flags*/;
	reply.flags = 0x00;
	reply.data = NULL;
	reply.dlen = 0;

	eaptls_compose(eap_ds, &reply);

	return 1;
}

/*
   The S flag is set only within the EAP-TLS start message
   sent from the EAP server to the peer.

   Similarly, when the EAP server receives an EAP-Response with the M
   bit set, it MUST respond with an EAP-Request with EAP-Type=EAP-TLS
   and no data. This serves as a fragment ACK. The EAP peer MUST wait
 */
eaptls_status_t eaptls_verify(EAP_DS *eap_ds, EAP_DS *prev_eap_ds)
{
	eaptls_packet_t	*eaptls_packet, *eaptls_prev = NULL;

	if ((eap_ds == NULL) 					|| 
		(eap_ds->response == NULL)			|| 
		(eap_ds->response->code != PW_EAP_RESPONSE)	||
		(eap_ds->response->length <= EAP_HEADER_LEN + 1/*EAP-Type*/)	||
		(eap_ds->response->type.type != PW_EAP_TLS)) {

		radlog(L_ERR, "rlm_eap_tls: corrupted data");
		return EAPTLS_INVALID;
	}

	eaptls_packet = (eaptls_packet_t *)eap_ds->response->type.data;
	if (prev_eap_ds && prev_eap_ds->response)
		eaptls_prev = (eaptls_packet_t *)prev_eap_ds->response->type.data;

	/*
	 * check for ACK
	 * 1. Find if this is a reply to the previous request sent
	 * 2. If typedata[0] == NULL && length == EAP_HEADER_LEN && ALLTLSFLAGS == 0
	 */
	if ((eap_ds->response->length == EAP_HEADER_LEN + 2/*EAPtype+flags*/) && 
		((eaptls_packet != NULL) && (eaptls_packet->flags == 0x00))) {

		if (prev_eap_ds->request->id == eap_ds->response->id) {
			radlog(L_INFO, "rlm_eap_tls: Received EAP-TLS ACK message");
			return EAPTLS_ACK;
		} else {
			radlog(L_ERR, "rlm_eap_tls: Received Invalid EAP-TLS ACK message");
			return EAPTLS_INVALID;
		}
	}

	/* We donot receive a TLS_START but we send it.  */
	if (TLS_START(eaptls_packet->flags) == 1) {
		radlog(L_ERR, "rlm_eap_tls:  Received EAP-TLS Start message");
		//return EAPTLS_START;
		return EAPTLS_INVALID;
	}

	/*
      The L bit (length included) is set to indicate the presence of the
      four octet TLS Message Length field, and MUST be set for the first
      fragment of a fragmented TLS message or set of messages. The M bit
      (more fragments) is set on all but the last fragment. The S bit
      (EAP-TLS start) is set in an EAP-TLS Start message. This
      differentiates the EAP-TLS Start message from a fragment
      acknowledgement.
	*/
	if (TLS_LENGTH_INCLUDED(eaptls_packet->flags)) {
		if (TLS_MORE_FRAGMENTS(eaptls_packet->flags)) {
			/*
			 * FIRST_FRAGMENT is identified
			 * 1. If there is no previous EAP-response received.
			 * 2. If EAP-response received, then its M bit not set.
			 * 	(It is because Last fragment will not have M bit set)
			 */
			if ((prev_eap_ds->response == NULL) ||
					(eaptls_prev == NULL) ||
					(TLS_MORE_FRAGMENTS(eaptls_prev->flags) == 0)) {

				radlog(L_INFO, "rlm_eap_tls:  Received EAP-TLS First Fragment of the message");
				return EAPTLS_FIRST_FRAGMENT;
			} else {

				radlog(L_INFO, "rlm_eap_tls:  More Fragments with length included");
				return EAPTLS_MORE_FRAGMENTS_WITH_LENGTH;
			}
		} else {

			radlog(L_INFO, "rlm_eap_tls:  Length Included");
			return EAPTLS_LENGTH_INCLUDED;
		}
	}

	if (TLS_MORE_FRAGMENTS(eaptls_packet->flags)) {
		radlog(L_INFO, "rlm_eap_tls:  More fragments to follow");
		return EAPTLS_MORE_FRAGMENTS;
	}

	/* None of the flags are set & But is valid EAPTLS packet */
	return EAPTLS_OK;
}

/*
 * EAPTLS_PACKET
 * code   =  EAP-code
 * id     =  EAP-id
 * length = code + id + length + flags + tlsdata
 *        =  1   +  1 +   2    +  1    +  X
 * length = EAP-length - 1(EAP-Type = 1 octet)
 * flags  = EAP-typedata[0] (1 octet)
 * dlen   = EAP-typedata[1-4] (4 octets), if L flag set
 *        = length - 5(code+id+length+flags), otherwise
 * data   = EAP-typedata[5-n], if L flag set
 *        = EAP-typedata[1-n], otherwise
 * packet = EAP-typedata (complete typedata)
 *
 * Points to consider during EAP-TLS data extraction
 * 1. In the received packet, No data will be present incase of ACK-NAK
 * 2. Incase if more fragments need to be received then ACK after retreiving this fragment.
 *
 *  RFC 2716 Section 4.2.  PPP EAP TLS Request Packet
 *
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Code      |   Identifier  |            Length             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     Type      |     Flags     |      TLS Message Length
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |     TLS Message Length        |       TLS Data...
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 *  The Length field is two octets and indicates the length of the EAP
 *  packet including the Code, Identifir, Length, Type, and TLS data
 *  fields.
 */
EAPTLS_PACKET *eaptls_extract(EAP_DS *eap_ds, eaptls_status_t status)
{
	EAPTLS_PACKET	*tlspacket;
	uint32_t	data_len = 0;
	uint32_t	len = 0;
	uint8_t		*data = NULL;

	if (status  == EAPTLS_INVALID)
		return NULL;

	/*
	 *	We have a 'type' after the EAP header, but no more data.
	 *
	 *	For TLS, we always have 'flags'
	 */
	if (eap_ds->response->length <= 2) {
		radlog(L_ERR, "rlm_eap_tls: Invalid EAP-TLS packet received.  (No data.)");
		return NULL;
	}

	tlspacket = eaptls_alloc();
	if (tlspacket == NULL) return NULL;

	/*
	 * Code & id for EAPTLS & EAP are same
	 * but eaptls_length = eap_length - 1(EAP-Type = 1 octet)
	 *
	 * length = code + id + length + type + tlsdata
	 *        =  1   +  1 +   2    +  1    +  X
	 */
	tlspacket->code = eap_ds->response->code;
	tlspacket->id = eap_ds->response->id;
	tlspacket->length = eap_ds->response->length - 1; /* EAP type */
	tlspacket->flags = eap_ds->response->type.data[0];

	/*
	 *	A quick sanity check of the flags.  If we've been told
	 *	that there's a length, and there isn't one, then stop.
	 */
	if ((tlspacket->flags & 0x80) &&
	    (tlspacket->length < 5)) { /* flags + TLS message length */
		radlog(L_ERR, "rlm_eap_tls: Invalid EAP-TLS packet received.  (Length bit is set, but no length was found.)");
		eaptls_free(&tlspacket);
		return NULL;
	}

	switch (status) {
	/*
	   The TLS Message Length field is
	   four octets, and provides the total length of the TLS message or set
	   of messages that is being fragmented; this simplifies buffer
	   allocation.
	 * For now we ignore TLS-Length field and hence the Total Length.
	 */
	/*
	 * Start storing this fragment 
	 * ???: How to interpret the Length ?
	 *	1. Is it is the total length of all the TLS records ?
	 *	2. Is it is the total length of one TLS record/current EAP data ?

	 * Dynamic allocation of buffers as & when we know the length
	 * should solve the problem.
	 */
	case EAPTLS_FIRST_FRAGMENT:
	case EAPTLS_LENGTH_INCLUDED:
	case EAPTLS_MORE_FRAGMENTS_WITH_LENGTH:
		if (tlspacket->length < 5) { /* flags + TLS message length */
			radlog(L_ERR, "rlm_eap_tls: Invalid EAP-TLS packet received.  (Excepted length, got none.)");
			eaptls_free(&tlspacket);
			return NULL;
		}

		/*
		 * Extract all the TLS fragments from the previous eap_ds
		 * Start appending this fragment to the above ds
		 */
		memcpy(&data_len, &eap_ds->response->type.data[1], sizeof(uint32_t));
		data_len = ntohl(data_len);
		data = (eap_ds->response->type.data + 5/*flags+TLS-Length*/);
		len = eap_ds->response->type.length - 5/*flags+TLS-Length*/;

		/*
		 *	Hmm... this should be an error, too.
		 */
		if (data_len > len) {
			radlog(L_INFO, "Total Length Included");
			data_len = len;
		}
		break;

		/*
		 *	Data length is implicit, from the EAP header.
		 */
	case EAPTLS_MORE_FRAGMENTS:
	case EAPTLS_OK:
		data_len = eap_ds->response->type.length - 1/*flags*/;
		data = eap_ds->response->type.data + 1/*flags*/;
		break;

	default:
		radlog(L_ERR, "rlm_eap_tls: Invalid EAP-TLS packet received");
		eaptls_free(&tlspacket);
		return NULL;
	}

	tlspacket->dlen = data_len;
	if (data_len) {
		tlspacket->data = (unsigned char *)malloc(data_len);
		if (tlspacket->data == NULL) {
			radlog(L_ERR, "rlm_eap_tls: out of memory");
			eaptls_free(&tlspacket);
			return NULL;
		}
		memcpy(tlspacket->data, data, data_len);
	}

	return tlspacket;
}

/*
 * compose the TLS reply packet in the EAP reply typedata
 */
int eaptls_compose(EAP_DS *eap_ds, EAPTLS_PACKET *reply)
{
	uint8_t *ptr;

	eap_ds->request->type.type = PW_EAP_TLS;

/*
   Similarly, when the EAP server receives an EAP-Response with the M
   bit set, it MUST respond with an EAP-Request with EAP-Type=EAP-TLS
   and no data. This serves as a fragment ACK. The EAP peer MUST wait
   until it receives the EAP-Request before sending another fragment.

   In order to prevent errors in the processing of fragments, the EAP
   server MUST use increment the Identifier value for each fragment ACK
   contained within an EAP-Request, and the peer MUST include this
   Identifier value in the subsequent fragment contained within an EAP-
   Reponse.
*/
	eap_ds->request->type.data = malloc(reply->length - TLS_HEADER_LEN + 1);
	if (eap_ds->request->type.data == NULL) {
		radlog(L_ERR, "rlm_eap_tls: out of memory");
		return 0;
	}

	/* EAPTLS Header length is excluded while computing EAP typelen */
	eap_ds->request->type.length = reply->length - TLS_HEADER_LEN;

	ptr = eap_ds->request->type.data;
	*ptr++ = (uint8_t)(reply->flags & 0xFF);

	if (reply->dlen) memcpy(ptr, reply->data, reply->dlen);

	switch (reply->code) {
	case EAPTLS_ACK:
	case EAPTLS_START:
	case EAPTLS_REQUEST:
		eap_ds->request->code = PW_EAP_REQUEST;
		break;
	case EAPTLS_SUCCESS:
		eap_ds->request->code = PW_EAP_SUCCESS;
		break;
	case EAPTLS_FAIL:
		eap_ds->request->code = PW_EAP_FAILURE;
		break;
	default:
		/* Should never enter here */
		eap_ds->request->code = PW_EAP_FAILURE;
		break;
	}

	return 1;
}

/*
 * To process the TLS,
 *  INCOMING DATA:
 * 	1. EAP-TLS should get the compelete TLS data from the peer.
 * 	2. Store that data in a data structure with any other required info
 *	3. Handle that data structure to the TLS module.
 *	4. TLS module will perform its operations on the data and handle back to EAP-TLS
 *  OUTGOING DATA:
 * 	1. EAP-TLS if necessary will fragment it and send it to the destination.
 *
 * During EAP-TLS initialization, TLS Context object will be initialized and stored.
 * For every new authentication requests, TLS will open a new session object and that
 * session object should be maintained even after the session is completed for
 * session resumption. (Probably later as a feature as we donot know who maintains these
 * session objects ie, SSL_CTX (internally) or TLS module(explicitly). If TLS module, then
 * how to let SSL API know about these sessions.)
 */
void eaptls_operation(EAPTLS_PACKET *eaptls_packet, eaptls_status_t status, EAP_HANDLER *handler)
{
	tls_session_t *tls_session;

	tls_session = (tls_session_t *)handler->opaque;
	/*
	 * Get the session struct from the handler 
	 * update the dirty_in buffer
	 * NOTE: This buffer will contain partial data when M bit is set.
	 * 	CAUTION while reinitializing this buffer.
	 * 	It should be reinitialized only when this M bit is NOT set.
	 */
	if (eaptls_packet->dlen != 
			record_plus(&tls_session->dirty_in, eaptls_packet->data, eaptls_packet->dlen)) {
			radlog(L_ERR, "rlm_eap_tls: Exceeded maximum record size");
			return;
	}

	if ((status == EAPTLS_MORE_FRAGMENTS) || 
		(status == EAPTLS_MORE_FRAGMENTS_WITH_LENGTH) || 
		(status == EAPTLS_FIRST_FRAGMENT)) {
		/*
		 * Send the ACK.
		 */
		eaptls_send_ack(handler->eap_ds);
	} else {
		/*
		 * We have the complete TLS-data or TLS-message
		 * Clean the dirty message
		 * Authenticate the user and send Success/Failure
		 * If more info is required then send another request.
		 */
		if (tls_handshake_recv(tls_session)) {
			eaptls_request(handler->eap_ds, tls_session);
		} else {
			eaptls_fail(handler->eap_ds);
		}
		//record_init(&tls_session->dirty_in);
	}
	return;
}
