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
 * File: xsup_monitor.c
 *
 * Authors: Chris.Hessing@utah.edu
 *
 * $Id: xsup_monitor.c,v 1.2 2004/03/20 03:57:50 chessing Exp $
 * $Date: 2004/03/20 03:57:50 $
 * $Log: xsup_monitor.c,v $
 * Revision 1.2  2004/03/20 03:57:50  chessing
 *
 * A couple of small updates to xsup_monitor, and gui_interface.
 *
 * Revision 1.1  2004/03/19 23:43:55  chessing
 *
 * Lots of changes.  Changed the password prompting code to no longer require the EAP methods to maintain their own stale frame buffer.  (Frame buffer pointers should be moved out of generic_eap_data before a final release.)  Instead, EAP methods should set need_password in generic_eap_data to 1, along with the variables that identify the eap type being used, and the challenge data (if any -- only interesting to OTP/GTC at this point).  Also fixed up xsup_set_pwd.c, and got it back in CVS.  (For some reason, it was in limbo.)  Added xsup_monitor under gui_tools/cli.  xsup_monitor will eventually be a cli program that will monitor XSupplicant (running as a daemon) and display status information, and request passwords when they are not in the config.
 *
 *
 *******************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>


#include "../../common/gui_interface.h"

struct ipc_interface ipcint;
int connected;
char activeint[16];

/*******************************************************************
 *
 * Make sure that XSupplicant is running.  If it isn't, then block and
 * check every so often.
 *
 *******************************************************************/
void block_while_not_xsup()
{
  int disp=0;

  while (is_xsup_running() == FALSE)
    {
      if (disp == 0)
	{
	  printf("XSupplicant isn't running.  We will block, and check again every second!\n");
	  disp = 1;
	}

      if (connected == 1)
	{
	  gui_interface_disconnect(&ipcint);
	  connected = 0;
	}

      sleep(1);
    }
}

void do_password(char *eaptype, char *challenge)
{
  char *passwd, buffer[1520], result[256];
  int bufptr, dlen, done = FALSE, i;

  bzero((char *)&buffer, 1520);
  
  if (gui_interface_build_header(&ipcint, (char *)&buffer, &bufptr, (char *)&activeint, IPC_SET)
      != ERR_NONE)
    {
      printf("Error! Couldn't build header!\n");
      return;
    }

  passwd = getpass("Please enter your password : ");
 
  if (passwd == NULL)
    {
      printf("No password set!\n");
      return;
    }

  gui_interface_set_password(&ipcint, (char *)&buffer, &bufptr, passwd);

  switch (gui_interface_send_packet(&ipcint, (char *)&buffer, bufptr))
    {
    case ERR_PKT_BAD:
      printf("Error! Packet appears to be bad!\n");
      return;
      break;
    case ERR_SOCK:
      printf("Error! There was a socket error!\n");
      return;
      break;
    }

  // Flush extra request packets.
  bzero((char *)&buffer, 1520);
  while (!done)
    {
      bufptr = 1520;
      if (gui_interface_get_packet(&ipcint, (char *)&buffer, &bufptr) == ERR_SOCK)
	{
	  done = TRUE;
	  continue;
	}

      bufptr = sizeof(struct ipc_interface)-1;

      i = gui_interface_parse_packet(&ipcint, (char *)&buffer, &bufptr, (char *)&result, &dlen, (char *)&activeint);

      if (i != PASSWORD)
	{
	  done = TRUE;
	}
    }

  if (result[0] == ACK) 
    {
      printf("Your password has been successfully submitted to the XSupplicant daemon.\n");
    } else {
      printf("Error!  Password not set!\n");
    }
}

void handle_password(char *packet)
{
  int bufptr=0;
  char eaptype[128], challenge[256];

  strcpy((char *)&eaptype[0], (char *)&packet[bufptr]);
  printf("EAP Type : %s\n",eaptype);
  bufptr += strlen(eaptype)+1;

  if (challenge[0] != 0x00)
    {
      strcpy((char *)&challenge[0], (char *)&packet[bufptr]);
      printf("Challenge : %s\n", challenge);
    }

  do_password(eaptype, challenge);
}

int main(int argc, char *argv[])
{
  char buffer[1520], result[256];
  int bufptr, dlen, i;


  while (1)
    {
      // Make sure that we have something to connect to.
      block_while_not_xsup();

      // If we have not connected before, or if we need to reconnect.
      if (connected != 1)
	{
	  if (gui_interface_connect(&ipcint, FALSE) != ERR_NONE)
	    {
	      printf("Error!  Couldn't get a handle to the daemon!\n");
	      exit(0);
	    }
	  connected = 1;

	  // Build our registration
	  bzero((char *)&buffer, 1520);

	  bufptr = 1520;
	  gui_interface_build_header(&ipcint, (char *)&buffer, &bufptr,
				     NULL, IPC_SET);
	  gui_interface_reg_client(&ipcint, (char *)&buffer, (int *)&bufptr);

	  switch (gui_interface_send_packet(&ipcint, (char *)&buffer, bufptr))
	    {
	    case ERR_PKT_BAD:
	      printf("Packet bad!\n");
	      break;
	    case ERR_SOCK:
	      printf("Socket error!\n");
	      break;
	    }
	}

      bzero(&buffer, 1520);
      bufptr = 1520;

      gui_interface_get_packet(&ipcint, (char *)&buffer, (int *)&bufptr);

      if (bufptr > 0)
	{
	  // Process the message we have been given.
	  bufptr = 0;
	  i=gui_interface_parse_packet(&ipcint, (char *)&buffer, &bufptr, 
				       (char *)&result, &dlen, (char *)&activeint);

	  switch (i)
	    {
	    case ERR_PKT_BAD:
	      // We have a bad packet!
	      printf("Got a bad packet from the Supplicant!\n");
	      break;
	    case PASSWORD:
	      handle_password((char *)&result[0]);
	    }
	} 

      sleep(1);
    }
}
