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
 * File: xsup_set_pwd.c
 *
 * Authors: Chris.Hessing@utah.edu
 *
 * $Id: xsup_set_pwd.c,v 1.2 2004/03/23 18:57:02 chessing Exp $
 * $Date: 2004/03/23 18:57:02 $
 * $Log: xsup_set_pwd.c,v $
 * Revision 1.2  2004/03/23 18:57:02  chessing
 *
 * Hopefully everything is fixed up now. ;)
 *
 * Revision 1.1  2004/03/23 18:48:13  chessing
 *
 * Yet another attempt to fix the xsup_set_pwd directory layout issues.
 *
 * Revision 1.7  2004/03/19 23:43:55  chessing
 *
 * Lots of changes.  Changed the password prompting code to no longer require the EAP methods to maintain their own stale frame buffer.  (Frame buffer pointers should be moved out of generic_eap_data before a final release.)  Instead, EAP methods should set need_password in generic_eap_data to 1, along with the variables that identify the eap type being used, and the challenge data (if any -- only interesting to OTP/GTC at this point).  Also fixed up xsup_set_pwd.c, and got it back in CVS.  (For some reason, it was in limbo.)  Added xsup_monitor under gui_tools/cli.  xsup_monitor will eventually be a cli program that will monitor XSupplicant (running as a daemon) and display status information, and request passwords when they are not in the config.
 *
 * Revision 1.5  2004/03/17 21:21:39  chessing
 *
 * Hopefully xsup_set_pwd is in the right place now. ;)  Added the functions needed for xsupplicant to request a password from a GUI client.  (Still needs to be tested.)  Updated TTLS and PEAP to support password prompting.  Fixed up curState change in statemachine.c, so it doesn't print [ALL] in front of the current state.
 *
 * Revision 1.1  2004/03/17 21:16:07  chessing
 *
 * Moved xsup_set_pwd.c to it's new location.
 *
 * Revision 1.3  2004/02/13 05:51:32  chessing
 *
 * Removed pieces from sha1.c that were duplicates for OpenSSL calls.  Hopefully this will resolve the TLS issues that have been under discussion on the list.  Added support for a default path for the config file.  If a config file is not specified on the command line, xsupplicant will attempt to read it from /etc/xsupplicant.conf.  Moved code to request a password from each of the EAP types to interface.c/h.  Currently this change is only implemented in the EAP-SIM module.  The changes to the GUI prompt code now make more sense, and are easier to follow.  It will be updated in other EAP types soon.
 *
 * Revision 1.2  2004/01/20 00:07:06  chessing
 *
 * EAP-SIM fixes.
 *
 * Revision 1.1  2004/01/18 06:31:19  chessing
 *
 * A few fixes here and there.  Added support in EAP-TLS to wait for a password to be entered from a "GUI" interface.  Added a small CLI utility to pass the password in to the daemon. (In gui_tools/cli)  Made needed IPC updates/changes to support passing in of a generic password to be used.
 *
 *
 *******************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <strings.h>
#include <stdlib.h>


#include "../../common/gui_interface.h"

void usage(char *pname)
{
  printf("Usage: %s -i <interface_name>\n\n\n", pname);
  printf("   The interface specified by <interface_name> should be the "
	 "interface that you wish to set a password for.\n");
}

int main(int argc, char *argv[])
{
  char *theOpts = "i:";
  char *passwd, buffer[1520], result[256];
  char *intname;
  char myint[17];
  struct ipc_interface ipcint;
  int bufptr, dlen, op;

  intname = NULL;

  if (is_xsup_running() == FALSE)
    {
      printf("Error!  XSupplicant must be running to use this tool!\n");
      exit(0);
    }

  if (gui_interface_connect(&ipcint, TRUE) != ERR_NONE)
    {
      printf("Error!  Couldn't get a handle to the daemon!\n");
      exit(0);
    }

  if (argc<2)
    {
      usage(argv[0]);
      exit(0);
    }

  while ((op = getopt(argc, argv, theOpts)) != EOF)
    {
      switch (op)
	{
	case 'i':
	  intname = optarg;
	  break;
	}
    }

  if (intname == NULL)
    {
      usage(argv[0]);
      exit(0);
    }
  
  bzero((char *)&buffer, 1520);
  
  if (gui_interface_build_header(&ipcint, (char *)&buffer, &bufptr, intname, IPC_SET)
      != ERR_NONE)
    {
      printf("Error! Couldn't build header!\n");
      gui_interface_disconnect(&ipcint);
      exit(0);
    }

  passwd = getpass("Please enter your password : ");
 
  if (passwd == NULL)
    {
      printf("No password set!\n");
      gui_interface_disconnect(&ipcint);
      exit(0);
    }

  gui_interface_set_password(&ipcint, (char *)&buffer, &bufptr, passwd);

  switch (gui_interface_send_packet(&ipcint, (char *)&buffer, bufptr))
    {
    case ERR_PKT_BAD:
      printf("Error! Packet appears to be bad!\n");
      gui_interface_disconnect(&ipcint);
      exit(0);
      break;
    case ERR_SOCK:
      printf("Error! There was a socket error!\n");
      gui_interface_disconnect(&ipcint);
      exit(0);
      break;
    }

  bzero((char *)&buffer, 1520);
  gui_interface_get_packet(&ipcint, (char *)&buffer, &bufptr);

  bufptr = sizeof(struct ipc_interface)-1;

  if (gui_interface_parse_packet(&ipcint, (char *)&buffer, &bufptr, (char *)&result, &dlen, (char *)&myint) == ERR_PKT_BAD)
    {
      printf("The daemon returned an invalid packet!\n");
    }

  if (result[0] == ACK) 
    {
      printf("Your password has been successfully submitted to the XSupplicant daemon.\n");
    } else {
      printf("Error!  Password not set!\n");
    }

  gui_interface_disconnect(&ipcint);
  return 0;
}
