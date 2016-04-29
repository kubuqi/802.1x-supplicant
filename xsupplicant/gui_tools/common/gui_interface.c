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
 * $Id: gui_interface.c,v 1.8 2004/03/29 21:57:13 chessing Exp $
 * $Date: 2004/03/29 21:57:13 $
 * $Log: gui_interface.c,v $
 * Revision 1.8  2004/03/29 21:57:13  chessing
 *
 * Changed the socket number we use for communication with the daemon from 10240 (which seems like a bad choice) to 26798 (which seems a little more random ;).  Also changed our debug code so that it doesn't output to the console when we are running in daemon mode.  The only way to get debug info while in daemon mode is to set a log file!!!
 *
 * Revision 1.7  2004/03/22 00:40:59  chessing
 *
 * Added logfile option to the global config options in the config file.  The logfile is where output will go when we are running in daemon mode.  If no logfile is defined, output will go to the console that started xsupplicant.   Added forking to the code, so that when started, the process can daemonize, and run in the background.  If there is a desire to force running in the foreground (such as for debugging), the -f option was added.
 *
 * Revision 1.6  2004/03/20 03:57:51  chessing
 *
 * A couple of small updates to xsup_monitor, and gui_interface.
 *
 * Revision 1.5  2004/03/19 23:43:56  chessing
 *
 * Lots of changes.  Changed the password prompting code to no longer require the EAP methods to maintain their own stale frame buffer.  (Frame buffer pointers should be moved out of generic_eap_data before a final release.)  Instead, EAP methods should set need_password in generic_eap_data to 1, along with the variables that identify the eap type being used, and the challenge data (if any -- only interesting to OTP/GTC at this point).  Also fixed up xsup_set_pwd.c, and got it back in CVS.  (For some reason, it was in limbo.)  Added xsup_monitor under gui_tools/cli.  xsup_monitor will eventually be a cli program that will monitor XSupplicant (running as a daemon) and display status information, and request passwords when they are not in the config.
 *
 * Revision 1.4  2004/01/20 00:07:06  chessing
 *
 * EAP-SIM fixes.
 *
 * Revision 1.3  2004/01/18 06:31:19  chessing
 *
 * A few fixes here and there.  Added support in EAP-TLS to wait for a password to be entered from a "GUI" interface.  Added a small CLI utility to pass the password in to the daemon. (In gui_tools/cli)  Made needed IPC updates/changes to support passing in of a generic password to be used.
 *
 * Revision 1.2  2004/01/15 23:45:10  chessing
 *
 * Fixed a segfault when looking for wireless interfaces when all we had was a wired interface.  Fixed external command execution so that junk doesn't end up in the processed string anymore.  Changed the state machine to call txRspAuth even if there isn't a frame to process.  This will enable EAP methods to request information from a GUI interface (such as passwords, or supply challenge information that might be needed to generate passwords).  EAP methods now must decide what to do when they are handed NULL for the pointer to the in frame.  If they don't need any more data, they should quietly exit.
 *
 * Revision 1.1  2003/12/23 04:57:10  chessing
 *
 * IPC additions, GUI client routines.
 *
 *
 *******************************************************************/
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "gui_interface.h"

char pidfileName[] = "/var/run/xsupplicant";

/*******************************************************************
 *
 * Establish a handler to talk to the supplicant.
 *
 *******************************************************************/
int gui_interface_connect(struct ipc_interface *handler, int block)
{
  int sockOpts, sockErr;

  handler->sock = socket(AF_INET, SOCK_DGRAM, 0);
  
  bzero(&handler->dest, sizeof(struct sockaddr_in));
  handler->dest.sin_family = AF_INET;
  handler->dest.sin_port = htons(26798);
  inet_aton("127.0.0.1", (struct in_addr *)&handler->dest.sin_addr.s_addr);

  /* See if we need to use non-blocking mode. */
  if (block == FALSE)
    {
      sockOpts = fcntl(handler->sock, F_GETFL, 0);
      sockErr = fcntl(handler->sock, F_SETFL, sockOpts | O_NONBLOCK);
      if (sockErr == -1) return ERR_SOCK;
    }

  return ERR_NONE;
}

/******************************************************************
 *
 * Disconnect from the daemon.
 *
 ******************************************************************/
int gui_interface_disconnect(struct ipc_interface *handler)
{
  close(handler->sock);
  return 0;
}

/******************************************************************
 *
 * See if we have any data.  If we aren't blocking, we will wait here
 * forever until we get something back!
 *
 ******************************************************************/
int gui_interface_get_packet(struct ipc_interface *handler, char *buffer,
			     int *bufsize)
{
  int readStat = -1;
  struct ipc_header *header;

  readStat = recvfrom(handler->sock, buffer, *bufsize, 0, NULL, 0);
 
  if (readStat < 0) 
    {
      if (errno != EWOULDBLOCK)
	{
	  printf("Socket error : %s\n",strerror(errno));
	  return ERR_SOCK;
	}
     
      *bufsize = 0;
      return ERR_NONE;
    }

  /* Make sure we have valid packet.  If we don't, then act like we didn't
     get a packet.  */
  header = (struct ipc_header *)buffer;

  if (header->version != 1)
    {
      *bufsize = 0;
      return ERR_PKT_BAD;
    }

  *bufsize = readStat;

  return ERR_NONE;
}

/******************************************************************
 *
 * Validate, and send a packet.
 *
 ******************************************************************/
int gui_interface_send_packet(struct ipc_interface *handler, char *buffer,
			      int bufptr)
{
  struct ipc_header *header;
  int i;

  header = (struct ipc_header *)buffer;

  if (header->version != 1) return ERR_PKT_BAD;

  i=sendto(handler->sock, buffer, bufptr, 0, 
	     (struct sockaddr *)&handler->dest, 
	   sizeof(struct sockaddr_in));
  if (i < 0)
    {
      printf("Socket error! (%d : %d)\n",i, errno);
      return ERR_SOCK;
    }

  return ERR_NONE;
}

/******************************************************************
 *
 * bufptr should point to the next command record to be processed. We
 * will pull the record out, copy the data for the record in to the
 * *data, and return the value of the record we are using.
 *
 * The caller should check that *bufptr < buffer length.
 *
 ******************************************************************/
int gui_interface_parse_packet(struct ipc_interface *handler, char *buffer,
			       int *bufptr, char *data, int *datalen, 
			       char * workint)
{
  struct ipc_header *header;
  struct ipc_cmd *cmd;

  header = (struct ipc_header *)&buffer[0];

  if (header->version != 1) return ERR_PKT_BAD;

  strcpy(workint, (char *)&header->interface);

  if (*bufptr <= 0) 
    {
      *bufptr = sizeof(struct ipc_header);
    } 

  cmd = (struct ipc_cmd *)&buffer[*bufptr];

  /* zero out the buffer, just to be safe. */
  bzero(data, cmd->len+1);

  *bufptr += 2;  /* Skip the command structure. */

  memcpy(data, (char *)&buffer[*bufptr], cmd->len);

  *datalen = cmd->len;

  return cmd->attribute;
}

/******************************************************************
 *
 * Build a header for our packet.
 *
 ******************************************************************/
int gui_interface_build_header(struct ipc_interface *handler, char *buffer,
			       int *bufptr, char *intName, int getset)
{
  struct ipc_header *header;

  header = (struct ipc_header *)buffer;
  
  header->version = 1;
  bzero(&header->interface, 16);
  if (intName != NULL) strcpy((char *)&header->interface, intName);
  header->getset = getset;
  header->numcmds = 0;
  *bufptr = sizeof(struct ipc_interface)-1;
  
  return ERR_NONE;
}

/*****************************************************************
 *
 * Request a list of all known interfaces.
 *
 *****************************************************************/
int gui_interface_get_interfaces(struct ipc_interface *handler, char *buffer,
				 int *bufptr)
{
  struct ipc_cmd *cmd;
  struct ipc_header *header;

  header = (struct ipc_header *)buffer;
  
  if (header->getset != IPC_GET) return ERR_CANT_GET;

  cmd = (struct ipc_cmd *)&buffer[*bufptr];
  cmd->attribute = INTERFACES;
  cmd->len = 0;
  *bufptr += 2;

  header->numcmds++;

  return ERR_NONE;
}

/*****************************************************************
 *
 * Request the authentication state of an interface.
 *
 *****************************************************************/
int gui_interface_get_state(struct ipc_interface *handler, char *buffer,
			    int *bufptr)
{
  struct ipc_cmd *cmd;
  struct ipc_header *header;

  header = (struct ipc_header *)buffer;

  if (header->getset != IPC_GET) return ERR_CANT_GET;

  cmd = (struct ipc_cmd *)&buffer[*bufptr];
  cmd->attribute = AUTH_STATE;
  cmd->len = 0;
  *bufptr += 2;

  header->numcmds++;

  return ERR_NONE;
}

/*****************************************************************
 *
 * Register ourselves as a client, so that we can be passed information
 * pushed from the daemon.
 *
 *****************************************************************/
int gui_interface_reg_client(struct ipc_interface *handler, char *buffer,
			     int *bufptr)
{
  struct ipc_cmd *cmd;
  struct ipc_header *header;

  header = (struct ipc_header *)buffer;

  if (header->getset != IPC_SET) return ERR_CANT_SET;

  cmd = (struct ipc_cmd *)&buffer[*bufptr];

  cmd->attribute = REGISTER;
  cmd->len = 0;
  *bufptr += 2;
  
  header->numcmds++;

  return ERR_NONE;
}

/****************************************
 *
 * Send in a password set command.
 *
 ****************************************/
int gui_interface_set_password(struct ipc_interface *handler, char *buffer,
			       int *bufptr, char *password)
{
  struct ipc_cmd *cmd;
  struct ipc_header *header;

  header = (struct ipc_header *)buffer;

  if (header->getset != IPC_SET) return ERR_CANT_SET;

  cmd = (struct ipc_cmd *)&buffer[*bufptr];

  cmd->attribute = TEMPPASSWORD;
  cmd->len = strlen(password);
  *bufptr += 2;

  strcpy((char *)&buffer[*bufptr], password);
  *bufptr += strlen(password);
  
  header->numcmds++;

  return ERR_NONE;
}

/****************************************
 *
 * Test for a PID file, and return an error if something seems to be running.
 *
 ****************************************/
int is_xsup_running()
{
  FILE *pidfile;

  pidfile = fopen(pidfileName, "r");
  if (pidfile)
    {
      fclose(pidfile);
      return TRUE;
    }
  return FALSE;
}
