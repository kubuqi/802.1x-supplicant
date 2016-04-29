/**
 * A client-side 802.1x implementation
 *
 * This code is released under both the GPL version 2 and BSD licenses.
 * Either license may be used.  The respective licenses are found below.
 *
 * Copyright (C) 2002 Bryan D. Payne & Nick L. Petroni Jr.
 * All Rights Reserved
 *
 * --- GPL Version 2 License ---
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * --- BSD License ---
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  - Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  - All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *       This product includes software developed by the University of
 *       Maryland at College Park and its contributors.
 *  - Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*******************************************************************
 *
 * File: gui_interface.c
 *
 * Authors: Chris.Hessing@utah.edu
 *
 *******************************************************************/

#ifndef _GUI_INTERFACE_H_
#define _GUI_INTERFACE_H_

#include <sys/socket.h>
#include <inttypes.h>
#include <netinet/in.h>

/* Username and password values. */
#define USERNAME         1
#define PASSWORD         2

/* Certificate related items. */
#define USER_CERT        3
#define USER_KEY         4
#define ROOT_CERT        5
#define ROOT_DIR         6
#define CRL_DIR          7
#define CHUNK_SIZE       8
#define RANDOM_FILE      9

/* Allowed types */
#define ALLOWED_PHASE1  10
#define ALLOWED_PHASE2  11

/* SIM/AKA Values */
#define AUTO_REALM      12

/* Global values to set.  (These will be processed by non-eap specific
   handlers.)  */
#define CONN_TYPE       13
#define DEST_MAC        14

#define NET_LIST        15
#define STARTUP_CMD     16
#define FIRST_AUTH_CMD  17
#define REAUTH_CMD      18
#define AUTH_PERIOD     19
#define HELD_PERIOD     20
#define MAX_STARTS      21
#define ALLOW_INTS      22
#define DENY_INTS       23

#define EAP_NOTIFICATION 24


struct ipc_set_config
{
  char phase1type;     /* EAP type for phase 1. */
  char phase2type;     /* EAP type for phase 2 (0 if there isn't one, 1-4 */
                       /* for TTLS phase 2 types.) */
  char setting;        /* 0 to 255, tags the value to set. */
  char length;         /* Length of the string to follow. */

  /* The following (length) bytes are a string value to be set.  The values
     will always be strings, so the command inteperator should know which ones
     need to be converted to other values.  */
};


#define AUTH_STATE    1  /* Get authentication state (get only!) */
#define PASSWORD      2  /* Set password (set only!) */
#define REGISTER      3  /* Register client (set only!) */
#define INTERFACES    4  /* Get interface list (get only!) */
#define PROFILE       5  /* Get or Set a profile by name. */
#define NOTIFY        6  /* A notification message to be displayed. */
#define TEMPPASSWORD  7  /* A temporary password to be used by the daemon. */

#define ERROR_MSG   255  /* Return an error message. */

#define ACK           1
#define NACK          0

#define FALSE         0
#define TRUE          1

#define DONT_CLEAR    0
#define CLEAR         1

#define IPC_RESPONSE  0
#define IPC_GET       1
#define IPC_SET       2

#define ERR_NONE       0
#define ERR_CANT_GET  -1
#define ERR_CANT_SET  -2
#define ERR_SOCK      -3
#define ERR_PKT_BAD   -4

/* This matches the order of states listed in IEEE 802.1x-2001, pg. 59,
   section 8.5.10. */
#define LOGOFF           0
#define DISCONNECTED     1
#define CONNECTING       2
#define ACQUIRED         3
#define AUTHENTICATING   4
#define HELD             5
#define AUTHENTICATED    6 

struct ipc_header {
  uint8_t version;    /* Version number spoken between client and daemon */
  char interface[16]; /* Interface name. */
  uint8_t getset;     /* Is this a get, or set request?
                         (0=Response, 1=Get, 2=Set) */
  uint8_t numcmds;    /* How many commands are in this packet? */
};

struct ipc_cmd {
  uint8_t attribute;
  uint8_t len;
  /* Value comes after that. */
};

struct ipc_interface {
  int sock;
  struct sockaddr_in dest;
};

int gui_interface_connect(struct ipc_interface *, int);
int gui_interface_disconnect(struct ipc_interface *);
int gui_interface_get_packet(struct ipc_interface *, char *, int *);
int gui_interface_send_packet(struct ipc_interface *, char *, int);
int gui_interface_build_header(struct ipc_interface *, char *, int *, char *,
			       int);
int gui_interface_parse_packet(struct ipc_interface *, char *, int *, char *,
			       int *, char *);
int gui_interface_get_interfaces(struct ipc_interface *, char *, int *);
int gui_interface_get_state(struct ipc_interface *, char *, int *);
int gui_interface_reg_client(struct ipc_interface *, char *, int *);
int gui_interface_set_password(struct ipc_interface *, char *, int *, char *);
int is_xsup_running();

#endif
