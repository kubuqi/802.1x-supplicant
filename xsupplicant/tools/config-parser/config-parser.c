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
 * The driver function for a test configuration parser 
 *
 * File: config-parser.c
 *
 * Authors: npetroni@cs.umd.edu
 *
 * $Id: config-parser.c,v 1.3 2003/12/19 23:19:11 npetroni Exp $
 * $Date: 2003/12/19 23:19:11 $
 * $Log: config-parser.c,v $
 * Revision 1.3  2003/12/19 23:19:11  npetroni
 * updated config code and test example. Fixed a couple things
 *   1. added new variables to globals:
 *      startup_command
 *      first_auth_command
 *      reauth_command
 *      auth_period
 *      held_period
 *      max_starts
 *      allow_interfaces
 *      deny_ineterfaces
 *
 *   2. added new variables to network:
 *      dest_mac
 *
 *   3. added new variables to ttls:
 *      phase2_type
 *
 *   4. added new variables to peap:
 *      allow_types
 *
 *   5. layed the groundwork for "preferred types" to be sent in Nak
 *
 * Revision 1.2  2003/11/29 01:11:31  npetroni
 * Added first round of configuration code.
 * Structural Changes:
 *    added examle config file and finished config-parser to test configuration
 *    files and optionally dump the output
 *
 * Current Status:
 *   Have not added parameters for any other method than TLS so we can discuss
 *   the changes before doing so.
 *
 *   Did not update config_build() so chris can keep testing as before.
 *
 * Revision 1.1  2003/11/26 21:11:55  npetroni
 * Added a config parser directory in which to start writing a test parser.
 * no functionality yet.
 *
 *
 *******************************************************************/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>

#include "xsup_debug.h"
#include "xsup_err.h"
#include "profile.h"
#include "config.h"

#define CONFIG_PARSE_VERBOSE   0x00000001
#define CONFIG_PARSE_HAVE_FILE 0x00000002

void usage(char *prog)
{
  debug_printf(DEBUG_NORMAL, "Usage: %s [-v] [-h] [-f file]"
	       "\n\t-f  file to parse (required)"
	       "\n\t-h  print this message"
	       "\n\t-v  verbose"
	       "\n", prog);
}

extern struct config_data *config_info;
/***************************************
 *
 * The main body of the program.  We should keep this simple!  Process any
 * command line options that were passed in, set any needed variables, and
 * enter the loop to handle sending an receiving frames.
 *
 ***************************************/
int main(int argc, char *argv[])
{
  int op;
  char *theOpts = "f:vh";
  char *config_fname = NULL;
  int flags = 0x00000000;

  // We should have at least one argument passed in!
  if (argc<2)
    {
      usage(argv[0]);
      exit(0);
    }

  // Process any arguments we were passed in.
  while ((op = getopt(argc, argv, theOpts)) != EOF) 
    {
      switch (op)
	{
	case 'f':
	  config_fname = optarg;
	  flags |= CONFIG_PARSE_HAVE_FILE;
	  break;
	case 'v':
	  flags |= CONFIG_PARSE_VERBOSE;
	  break;
	case 'h':
	  usage(argv[0]);
	  exit(0);
	  break;
	default:
	  usage(argv[0]);
	  exit(0);
	  break;
	}
    }
  if (config_fname == NULL) {
    debug_printf(DEBUG_NORMAL, "No filename given!\n");
    usage(argv[0]);
    exit(0);
  }
  
  if (config_setup(config_fname) == XENONE) {
    if (flags & CONFIG_PARSE_VERBOSE)
      dump_config_data(config_info);
    debug_printf(DEBUG_NORMAL, "Parsed successfully. Exiting.\n");
    config_destroy();
  }
  else {
    debug_printf(DEBUG_NORMAL, "Failed to Parse \"%s\". Exiting.\n", 
		 config_fname);
  }
  
  
  return XENONE;
}
