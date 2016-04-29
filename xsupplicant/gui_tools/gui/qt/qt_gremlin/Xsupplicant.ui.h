/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <gui_interface.h>
#include <unistd.h>
#include <stdlib.h>

struct ipc_interface ipcint;
int connected = 0;
char activeint[16];

void Xsupplicant::init()
{
    password->clear();
    challengeLabel->hide();
    challengeMessage->hide();
}

void Xsupplicant::destroy()
{
    ipc_cleanup();
}

void Xsupplicant::ipc_cleanup()
{
    printf("Cleaning up IPC!\n");
          if (connected == 1)
	{
	  ipc_disconnect();
	  connected = 0;
	}
}

void Xsupplicant::ipc_flush_requests(int reqtype)
{
    
}

void Xsupplicant::ipc_handle_packet(int reqtype, char *packet)
{
    // Do something with the packet...
    
    if(reqtype == ACK)
    {
	printf("Got ACK.\n");
    }
    //printf("Would do handle packet\n");
}

void Xsupplicant::ipc_disconnect()
{	
    printf("Disconnecting IPC.\n");
    gui_interface_disconnect(&ipcint);	
}


int Xsupplicant::sendPassword()
{
    setPassword();
}

void Xsupplicant::setPassword()
{
    char *ipc_passwd;
    char *buffer[1520];
    char result[256];
    char myint[17];
    int bufptr, dlen, op;
    int done = FALSE, i;
    
    QString thePassword;
    int pwd_length;
    
    bzero((char *)&buffer, 1520);
    
     thePassword = Xsupplicant::password->text();
    
    pwd_length = thePassword.length();
    
    ipc_passwd = (char *)malloc(pwd_length + 1);
    
    bzero((char *)ipc_passwd, pwd_length + 1);
    
    strcpy(ipc_passwd, thePassword.ascii());

    if(connected != 1)
    {
	printf("ACK! We're not connected...\n");
    }
    else
    {
	    
	if(gui_interface_build_header(&ipcint, (char*)&buffer, &bufptr, (char *)&activeint, IPC_SET) == ERR_NONE)
	{
		if(ipc_passwd != NULL)
		{
		    gui_interface_set_password(&ipcint, (char *)&buffer, &bufptr, ipc_passwd);
		    
		    free(ipc_passwd);
		    
		    switch (gui_interface_send_packet(&ipcint, (char *)&buffer, bufptr))
		    {
		    case ERR_PKT_BAD:
			ipc_disconnect();
			break;
		    case ERR_SOCK:
			ipc_disconnect();
			break;
		    } 		   
		    
		    // Flush extra request packets.
		    bzero((char *)&buffer, 1520);
		    while (!done)
		    {
			
			bufptr = 1520;
			
			if(gui_interface_get_packet(&ipcint, (char *)&buffer, &bufptr) == ERR_SOCK)
			{
			    done = TRUE;
			    continue;
			}
		
			bufptr = sizeof(struct ipc_interface)-1;

			i = gui_interface_parse_packet(&ipcint, (char *)&buffer, &bufptr, (char *)&result, &dlen, (char *)&activeint);

			if (i != PASSWORD)
			{
			    //Hmmm this is going to be difficult to handle...
			    ipc_handle_packet(i, (char *)&result[0]);
			    done = TRUE;
			}
		    }
		    
		}
		else // passwd == NULL
		{
		    printf("NULL Password!\n");
		    //Not sure what we should do here...
		}
	    }
	    else // gui_interface_build_header(&ipcint, (char*)&buffer, &bufptr, NULL, IPC_SET) != ERR_NONE
	    {
		//Pop up a dialog box explaining the problem
		printf("Couldn't build IPC Header!\n");
	    }
	}	   
}

void Xsupplicant::block_while_not_xsup()
{
  while (is_xsup_running() == FALSE)
    {
      if (connected == 1)
	{
	  gui_interface_disconnect(&ipcint);
	  connected = 0;
	}

      sleep(1);
    }
}

int Xsupplicant::check_for_message()  
{
    char buffer[1520], result[256];
    int bufptr, dlen, i;
    
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
	      return 0;
	      break;
	    case ERR_SOCK:
	      printf("Socket error!\n");
	      return 0;
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
	  
	  i = gui_interface_parse_packet(&ipcint, (char *)&buffer, &bufptr, 
				       (char *)&result, &dlen, (char *)&activeint);
	  
	  if(i == PASSWORD)
	  {
	      char *packet = (char *)&result[0];
	      
	      int bufptr=0;
	      char eaptype[128], challenge[256];
	      QString *eapText, *challengeText;

	      strcpy((char *)&eaptype[0], (char *)&packet[bufptr]);
	      
	      eapText = new QString(eaptype);
	      
	      eapType->setText(*eapText);
	    
	      bufptr += strlen(eaptype)+1;

	      if (challenge[0] != 0x00)
	      {
		  strcpy((char *)&challenge[0], (char *)&packet[bufptr]);
		  
		  challengeText = new QString(challenge);
		  
		  challengeMessage->setText(*challengeText);
		  
		  challengeLabel->show();
		  challengeMessage->show();
	      }
	  }
	  
	  return i;
	  
	} 
      
      return 0;
}

void Xsupplicant::main()
{
    int message;	
    
    //Set up defaults for every run
    init();
	
    while(1)
    {	
	sleep(1);
	
	message = check_for_message();
	
	if(message > 0)
	{
	   switch (message)
	    {
	    case ERR_PKT_BAD:
	      // We have a bad packet!
	      printf("Got a bad packet from the Supplicant!\n");
	      break;
	    case PASSWORD:
		{
		    show();
		    exec();
		    //a.connect( &a, SIGNAL( lastWindowClosed() ), &a, SLOT( Xsupplicant::hideSelf() ) );

		}break;
	    case EAP_NOTIFICATION:
		{
		    printf("Got an EAP Notification!");
		}break;
	    };
	   
       }
	
    }
    
}


void Xsupplicant::hideSelf()
{
    hide();
}
