/*
 * ASCEND: @(#)filters.c	1.3 (95/07/25 00:55:30)
 *
 *      Copyright (c) 1994 Ascend Communications, Inc.
 *      All rights reserved.
 *
 *	Permission to copy all or part of this material for any purpose is
 *	granted provided that the above copyright notice and this paragraph
 *	are duplicated in all copies.  THIS SOFTWARE IS PROVIDED ``AS IS''
 *	AND WITHOUT ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, WITHOUT
 *	LIMITATION, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *	FOR A PARTICULAR PURPOSE.
 */

/* $Id: filters.c,v 1.25.2.1 2003/09/02 16:41:31 phampson Exp $ */

/*
 *  Alan DeKok's comments:  This code SUCKS.
 *
 *  This code should be re-written to get rid of Ascend's copyright,
 *  and to fix all of the horrible crap of the current implementation.
 */

#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "libradius.h"

#if HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

/*
 * Two types of filters are supported, GENERIC and IP.  The identifiers
 * are:
 */

#define RAD_FILTER_GENERIC	0
#define RAD_FILTER_IP		1
#define RAD_FILTER_IPX		2

/*
 * Generic filters mask and match up to RAD_MAX_FILTER_LEN bytes
 * starting at some offset.  The length is:
 */
#define RAD_MAX_FILTER_LEN	6

/*
 * ASCEND extensions for ABINARY filters
 */

#define IPX_NODE_ADDR_LEN		6

typedef uint32_t		IpxNet;
typedef uint8_t			IpxNode[ IPX_NODE_ADDR_LEN ];
typedef uint16_t		IpxSocket;

#if ! defined( FALSE )
# define FALSE		0
# define TRUE		(! FALSE)
#endif

/*
 * RadFilterComparison:
 *
 * An enumerated values for the IP filter port comparisons.
 */
typedef enum {
	RAD_NO_COMPARE = 0,
	RAD_COMPARE_LESS,
	RAD_COMPARE_EQUAL,
	RAD_COMPARE_GREATER,
	RAD_COMPARE_NOT_EQUAL
} RadFilterComparison;

/*
 * RadIpFilter:
 *
 * The binary format of an IP filter.  ALL fields are stored in
 * network byte order.
 *
 *	srcip:		The source IP address.
 *
 *	dstip:		The destination IP address.
 *
 *	srcmask:	The number of leading one bits in the source address
 *			mask.  Specifies the bits of interest.
 *
 *	dstmask:	The number of leading one bits in the destination
 *			address mask. Specifies the bits of interest.
 *
 *	proto:		The IP protocol number
 *
 *	establised:	A boolean value.  TRUE when we care about the
 *			established state of a TCP connection.  FALSE when
 *			we dont care.
 *
 *	srcport:	TCP or UDP source port number.
 *
 *	dstport:	TCP or UDP destination port number.
 *
 *	srcPortCmp:	One of the values of the RadFilterComparison enumeration
 *			specifying how to compare the srcport value.
 *
 *	dstPortCmp:	One of the values of the RadFilterComparison enumeration
 *			specifying how to compare the dstport value.
 *
 *	fill:		Round things out to a dword boundary.
 */
typedef struct radip {
	uint32_t	srcip;
	uint32_t	dstip;
	uint8_t 	srcmask;
	uint8_t 	dstmask;
	uint8_t		proto;
	uint8_t		established;
	uint16_t	srcport;
	uint16_t	dstport;
	uint8_t		srcPortComp;
	uint8_t		dstPortComp;
	unsigned char   fill[4];        /* used to be fill[2] */
} RadIpFilter;

/*
 * RadIpxFilter:
 * The binary format of an IPX filter.  ALL fields are stored in
 * network byte order.
 *
 *  srcIpxNet:      Source IPX Net address
 *
 *  srcIpxNode:     Source IPX Node address
 *
 *  srcIpxSoc:      Source IPX socket address
 *
 *  dstIpxNet:      Destination IPX Net address
 *
 *  dstIpxNode:     Destination IPX Node address
 *
 *  dstIpxSoc:      Destination IPX socket address
 *
 *  srcSocComp:     Source socket compare value
 *
 *  dstSocComp:     Destination socket compare value
 *
 */
typedef struct radipx {                         
	IpxNet		srcIpxNet; /* LongWord */
	IpxNode		srcIpxNode; /* Byte[6] */
	IpxSocket	srcIpxSoc; /* Word */
	IpxNet		dstIpxNet; /* LongWord */
	IpxNode		dstIpxNode;                     /* Byte[6] */
	IpxSocket	dstIpxSoc;                      /* Word */
	uint8_t		srcSocComp;
	uint8_t		dstSocComp;
} RadIpxFilter;

    /*
     * RadGenericFilter:
     *
     * The binary format of a GENERIC filter.  ALL fields are stored in
     * network byte order.
     *
     *	offset:		Number of bytes into packet to start comparison.
     *
     *	len:		Number of bytes to mask and compare.  May not
     *			exceed RAD_MAX_FILTER_LEN.
     *
     *	more:		Boolean.  If non-zero the next filter entry is
     *			also to be applied to a packet.
     *
     *	mask:		A bit mask specifying the bits to compare.
     *
     *	value:		A value to compare against the masked bits at
     *			offset in a users packet.
     *			
     *	compNeq:	Defines type of comarison (Equal or Notequal)
     *			default is Equal.
     *
     *	fill:		Round things out to a dword boundary
     */
typedef struct radgeneric {
	uint16_t	offset;
	uint16_t	len;
	uint16_t	more;
	uint8_t		mask[ RAD_MAX_FILTER_LEN ];
	uint8_t		value[ RAD_MAX_FILTER_LEN ];
	uint8_t		compNeq;
	uint8_t       fill[3];        /* used to be fill[1] */
} RadGenericFilter;

    /*
     * RadFilter:
     *
     * A binary filter element.  Contains either a RadIpFilter or a
     * RadGenericFilter.  All fields are stored in network byte order.
     *
     *	type:		Either RAD_FILTER_GENERIC or RAD_FILTER_IP.
     *
     *	forward:	TRUE if we should forward packets that match this
     *			filter, FALSE if we should drop packets that match
     *			this filter.
     *
     *	indirection:	TRUE if this is an input filter, FALSE if this is
     *			an output filter.
     *
     *	fill:		Round things out to a dword boundary.
     *
     *	u:		A union of
     *			ip:		An ip filter entry
     *			generic:	A generic filter entry
     */
typedef struct filter {
	uint8_t 	type;
	uint8_t		forward;
	uint8_t		indirection;
	uint8_t		fill;
	union {
		RadIpFilter   	 ip;
		RadIpxFilter   	 ipx;
		RadGenericFilter generic;
	} u;
} RadFilter;
#define SIZEOF_RADFILTER 32

/*
 * FilterPortType:
 *
 * Ascii names of some well known tcp/udp services.
 * Used for filtering on a port type.
 *
 * ??? What the heck is wrong with getservbyname?
 */
static const LRAD_NAME_NUMBER filterPortType[] = {
	{ "ftp-data",   20 },
	{ "ftp",	21 },
	{ "telnet",	23 },
	{ "smtp",	25 },
	{ "nameserver", 42 },
	{ "domain",	53 },
	{ "tftp",	69 },
	{ "gopher",	70 },
	{ "finger",	79 },
	{ "www",	80 },
	{ "kerberos",	88 },
	{ "hostname",	101 },
	{ "nntp",	119 },
	{ "ntp",	123 },
	{ "exec",	512 },
	{ "login",	513 },
	{ "cmd",	514 },
	{ "talk",	517 },
	{  NULL ,	0},
};

static const LRAD_NAME_NUMBER filterType[] = {
	{ "generic",RAD_FILTER_GENERIC},
	{ "ip", 	RAD_FILTER_IP},
	{ "ipx", 	RAD_FILTER_IPX},
	{ NULL, 	0},
};

typedef enum {
    FILTER_GENERIC_TYPE,
    FILTER_IP_TYPE,
    FILTER_IN,
    FILTER_OUT,
    FILTER_FORWARD,
    FILTER_DROP,
    FILTER_GENERIC_OFFSET,
    FILTER_GENERIC_MASK,
    FILTER_GENERIC_VALUE,
    FILTER_GENERIC_COMPNEQ,
    FILTER_GENERIC_COMPEQ,
    FILTER_MORE,
    FILTER_IP_DST,
    FILTER_IP_SRC,
    FILTER_IP_PROTO,
    FILTER_IP_DST_PORT,
    FILTER_IP_SRC_PORT,
    FILTER_EST,
    FILTER_IPX_TYPE,
    FILTER_IPX_DST_IPXNET,
    FILTER_IPX_DST_IPXNODE,
    FILTER_IPX_DST_IPXSOCK,
    FILTER_IPX_SRC_IPXNET,
    FILTER_IPX_SRC_IPXNODE,
    FILTER_IPX_SRC_IPXSOCK
} FilterTokens;


static const LRAD_NAME_NUMBER filterKeywords[] = {
    { "ip", 	FILTER_IP_TYPE },
    { "generic",FILTER_GENERIC_TYPE },
    { "in", 	FILTER_IN },
    { "out",	FILTER_OUT },
    { "forward",FILTER_FORWARD },
    { "drop",	FILTER_DROP },
    { "dstip",  FILTER_IP_DST },
    { "srcip",  FILTER_IP_SRC },
    { "dstport",FILTER_IP_DST_PORT },
    { "srcport",FILTER_IP_SRC_PORT },
    { "est",	FILTER_EST },
    { "more",	FILTER_MORE },
    { "!=",	FILTER_GENERIC_COMPNEQ },
    { "==",	FILTER_GENERIC_COMPEQ  },
    { "ipx",	FILTER_IPX_TYPE  },
    { "dstipxnet",	FILTER_IPX_DST_IPXNET  },
    { "dstipxnode",	FILTER_IPX_DST_IPXNODE  },
    { "dstipxsock",	FILTER_IPX_DST_IPXSOCK  },
    { "srcipxnet",	FILTER_IPX_SRC_IPXNET  },
    { "srcipxnode",	FILTER_IPX_SRC_IPXNODE  },
    { "srcipxsock",	FILTER_IPX_SRC_IPXSOCK  },
    {  NULL , 		0},
};

#define FILTER_DIRECTION 	0
#define FILTER_DISPOSITION	1
#define IP_FILTER_COMPLETE  	0x3	/* bits shifted by FILTER_DIRECTION */
					/* FILTER_DISPOSITION */

#define IPX_FILTER_COMPLETE      0x3     /* bits shifted by FILTER_DIRECTION */
                                        /* FILTER_DISPOSITION */

#define GENERIC_FILTER_COMPLETE 0x1c3	/* bits shifted for FILTER_DIRECTION */
					/* FILTER_DISPOSITION, FILTER_GENERIC_OFFSET*/
					/* FILTER_GENERIC_MASK, FILTER_GENERIC_VALUE*/

/*
 * FilterProtoName:
 *
 * Ascii name of protocols used for filtering.
 *
 *  ??? What the heck is wrong with getprotobyname?
 */
static const LRAD_NAME_NUMBER filterProtoName[] = {
	{ "tcp",  6 },
	{ "udp",  17 },
	{ "ospf", 89 },
	{ "icmp", 1 },
	{ "0",	  0 },
	{  NULL , 0 },
};

static const LRAD_NAME_NUMBER filterCompare[] = {
	{ "<",	RAD_COMPARE_LESS },
	{ "=",	RAD_COMPARE_EQUAL },
	{ ">",	RAD_COMPARE_GREATER },
	{ "!=", RAD_COMPARE_NOT_EQUAL },
	{ NULL, 0 },
};

/*
 * isAllDigit:
 *
 * Routine checks a string to make sure all values are digits.
 *
 *	token:			Pointer to sting to check.
 *
 * 	returns:		TRUE if all digits, or FALSE.
 *
 */
static int
isAllDigit(const char *token)
{
    int i;

    i = strlen( token );
    while( i-- ) {
	if( isdigit( (int) *token ) ) {
	    token++;
	} else {
	    break;
	}
    }
    if( i > 0 ) {
	return( FALSE );
    } 

    return( TRUE );
}

/*
 * a2octet:
 *
 * Converts the ascii mask and value for generic filters into octets.
 * It also does a sanity check to see if the string is greater than
 * MAX_FILTER_LEN. It assumes the sting is hex with NO leading "0x"
 *
 *	tok:			Pointer to the string.
 *
 *  retBuf:			Pointer to place the octets.
 *
 *	returns:		Number of octects or -1 for error.
 * 
 */
static short
a2octet(const char *tok, char *retBuf)
{
    short	rc, len, val, retLen, i;
    char	buf[ RAD_MAX_FILTER_LEN *2 ];
    char	*octet = buf;

    rc = -1;
    retLen = 0;

    if( ( len = strlen( tok ) ) <= ( RAD_MAX_FILTER_LEN*2 ) ) {
	retLen = len/2;
	if( len % 2 ) {
	    retLen++;
	}
	memset( buf, '\0', RAD_MAX_FILTER_LEN * 2 );
	for( ; len; len-- ) {
	    if( *tok <= '9' && *tok >= '0' ) {
		val = '0';
	        *octet++ = *tok++ - val;
	    } else if( isxdigit( (int) *tok ) ) {
		if( *tok > 'Z' ) {
		    val = 'a';
		} else {
		    val = 'A';
		}
	        *octet++ = ( *tok++ - val ) + 10;
	    } else {
		break;	
	    }
	}
	if( !len ) {
	    /* merge the values */
	    for( i = 0; i < RAD_MAX_FILTER_LEN*2; i+=2 ) {
		*retBuf++ = (buf[i] << 4) | buf[i+1];
	    }
	}
    }

    if( len ) {
	rc = -1;
    } else {
	rc = retLen;
    }
    return( rc );
}



/*
 * defaultNetmask:
 *
 *	Given an ip address this routine calculate a default netmask.
 *
 *	address:		Ip address.
 *
 *	returns:		Number of bits for the netmask
 *
 */
static char
defaultNetmask(uint32_t address)
{
    char netmask;

    if ( ! address ) {
	netmask = 0;
    } else if (( address & htonl( 0x80000000 ) ) == 0 ) {
	netmask = 8;
    } else if (( address & htonl( 0xc0000000 ) ) == htonl( 0x80000000 ) ) {
	netmask = 16;
    } else if (( address & htonl( 0xe0000000 ) ) == htonl( 0xc0000000 ) ) {
	netmask = 24;
    } else {
	netmask = 32;
    }
    return netmask;
}


/*
 * This functions attempts to convert an IP address in ASCII dot
 * with an optional netmask part to a pair of IpAddress.  Note:
 * An IpAddress is always stored in network byte order.
 *
 * Parameters:
 *
 *  string:		Pointer to a NULL terminated IP address in dot 
 *			notation followed by an optional /nn to indicate
 *			the number leading of bits in the netmask.
 * 
 *  ipAddress:	Pointer to an IpAddress where the converted
 *			address will be stored.
 *
 *	netmask:	Pointer to an IpAddress where the netmask
 *			will be stored.  If no netmask is passed as
 *			as part of the address the default netmask will
 *			be stored here.
 *
 * Returns:
 *	<>		TRUE if valid conversion, FALSE otherwise.
 *
 *	*ipAddress:	If function returns TRUE, the IP address in NBO.
 *	*netmask:	If function returns TRUE, the netmask in NBO.
 */
static int
ipAddressStringToValue(char *string, uint32_t *ipAddress,
		       char *netmask)
{
    u_char*	dst;
    char*	cp;
    int		numDots;
    int		i;
    long	value;

    if ( ! string ) {
    	return(FALSE);
    }

    /* Allow an IP address to be blanked instead of forcing entry of
       0.0.0.0 -- the user will like it. */

    if ( *string == 0 ) {
	*ipAddress = 0;
	*netmask = 0;
	return TRUE;
    }

    /* First just count the number of dots in the address.  If there
       are more or less than three the address is invalid. */

    cp = string;
    numDots = 0;
    while( *cp ) {
	if( !strchr("1234567890./", *cp) ) {
	    return( FALSE );
	}
	if ( *cp == '.') {
	    ++numDots;
	}
	++cp;
    }
    if ( numDots != 3 ) {
	return( FALSE );
    }

    dst = (u_char *) ipAddress;
    cp = string;

    for ( i = 0; i < (int)sizeof( *ipAddress ); i++ ) {
	value = strtol( cp, &cp, 10 );
	if (( value < 0 ) || ( value > 255 )) {
	    return( FALSE );
	}
	*dst++ = (u_char) value;
	if ( *cp == '.' ) {
	    cp += 1;
	}
    }

    /* If there is a netmask part, parse it, otherwise figure out the
       default netmask for this class of address. */

    if ( *cp == '/' ) {
	value = strtol( cp + 1, &cp, 10 );
	if (( *cp != 0 ) || ( value < 0 ) || ( value > 32 )) {
	    return FALSE;
	}
	*netmask = (char) value;
    } else {
	*netmask = defaultNetmask( *ipAddress );
    }
    return TRUE;
}

/*
 * Convert a 12 digit string representation of a hex data field to a
 * value.
 */
static int
stringToNode(unsigned char *dest, unsigned char *src)
{
    int         srcIx = 0;
    int         ix;
    int         nibble1;
    int         nibble2;
    int		temp;
    unsigned char *src1;

    src1 = (unsigned char *) strchr((char *)src, 'x');

    if (src1 == NULL)
	src1 = (unsigned char *) strchr((char *)src,'X');

    if (src1 == NULL)
	src1 = src;
    else
	src1++;

    /* skip any leading 0x or 0X 's */
    temp = strlen( (char*) src1 );
    if( strlen( (char*) src1 ) != ( IPX_NODE_ADDR_LEN * 2 ) ) {
        return( FALSE );
    }

    for ( ix = 0; ix < IPX_NODE_ADDR_LEN; ++ix ) {
        if ( src1[ srcIx ] <= '9' ) {
            nibble1 = src1[ srcIx ] & 0x0f;
        } else {
            nibble1 = (src1[ srcIx ] & 0x0f) + 9;
        }
        srcIx += 1;
        if ( src1[ srcIx ] <= '9' ) {
            nibble2 = src1[ srcIx ] & 0x0f;
        } else {
            nibble2 = (src1[ srcIx ] & 0x0f) + 9;
        }
        srcIx += 1;
        ((unsigned char *) dest)[ ix ] = (unsigned char) (nibble1 << 4) + nibble2;
    }

    return( TRUE );
}


/*
 * parseIpxFilter:
 *
 * This routine parses an IPX filter string from a RADIUS
 * reply. The format of the string is:
 *
 *	ipx dir action [ srcipxnet nnnn srcipxnode mmmmm [srcipxsoc cmd value ]]
 * 	               [ dstipxnet nnnn dstipxnode mmmmm [dstipxsoc cmd value ]]
 *
 * Fields in [...] are optional.
 *	where:
 *
 *  ipx:		Keyword to designate an IPX filter. Actually this
 *			has been determined by parseFilter.
 *
 *	dir:		Filter direction. "IN" or "OUT"
 *
 *	action:		Filter action. "FORWARD" or "DROP"
 *
 *  srcipxnet:      Keyword for source IPX address.
 *                  nnnn = IPX Node address.
 *
 *  srcipxnode:     Keyword for source IPX Node address.
 *                  mmmmm = IPX Node Address, could be FFFFFF.
 *                  A vlid ipx node number should accompany ipx net number.
 *
 *  srcipxsoc:      Keyword for source IPX socket address.
 *
 *  cmd:            One of ">" or "<" or "=" or "!=".
 *
 *  value:          Socket value to be compared against, in hex. 
 *			
 *	dstipxnet:	Keyword for destination IPX address.
 *			nnnn = IPX Node address. 
 *			
 *	dstipxnode:	Keyword for destination IPX Node address.
 *  		mmmmm = IPX Node Address, could be FFFFFF.
 *			A vlid ipx node number should accompany ipx net number.
 *			
 *	dstipxsoc:	Keyword for destination IPX socket address.
 *			
 *	cmd:		One of ">" or "<" or "=" or "!=".
 *			
 *	value:		Socket value to be compared against, in hex.
 *			
 *			
 * expects:
 *
 *	curEntry:	Pointer to place the filter structure
 *
 *	returns:	-1 for error or 0 for OK
 *	
 */
static int 
parseIpxFilter(const char *curString, RadFilter *curEntry)
{
    unsigned long	elements = 0l;
    int			tok; 
    char*		token;
    RadIpxFilter*	ipx;

    token = strtok( NULL, " " ); 

    memset( curEntry, '\0', sizeof( RadFilter ) );
    curEntry->type = RAD_FILTER_IPX; 
    ipx = &curEntry->u.ipx;
 
    while( token ) {
  	tok = lrad_str2int(filterKeywords, token, -1);
	switch( tok ) {
	    case FILTER_IN:
	    case FILTER_OUT:
		curEntry->indirection = tok == FILTER_IN ? TRUE: FALSE;
	        elements |= (1 << FILTER_DIRECTION );
		break;

	    case FILTER_FORWARD:
	    case FILTER_DROP:
	        elements |= (1 << FILTER_DISPOSITION );
		if( tok == FILTER_FORWARD ) {
		    curEntry->forward = TRUE;
		} else {
		    curEntry->forward = FALSE;
		}
		break;

	    case FILTER_IPX_DST_IPXNET:
	    case FILTER_IPX_SRC_IPXNET:
		token = strtok( NULL, " " );

		if ( token ) {
		    if( tok == FILTER_IPX_DST_IPXNET ) {
			ipx->dstIpxNet = ntohl( strtol( token, 0, 16 ));
		    } else {
			ipx->srcIpxNet = ntohl( strtol( token, 0, 16 ));
		    }
		    break;
		} 
		goto doneErr; 

            case FILTER_IPX_DST_IPXNODE:
            case FILTER_IPX_SRC_IPXNODE:
		token = strtok( NULL, " " );

		if ( token ) {
		    if ( tok == FILTER_IPX_DST_IPXNODE) {
			stringToNode( (unsigned char *)ipx->dstIpxNode, (unsigned char*)token );
		    } else {
			stringToNode( (unsigned char *)ipx->srcIpxNode, (unsigned char*)token );
		    }
		    break;
		}
                goto doneErr;

            case FILTER_IPX_DST_IPXSOCK:
            case FILTER_IPX_SRC_IPXSOCK:
	    {
		RadFilterComparison cmp;

                token = strtok( NULL, " " );

		if ( token ) {
		    cmp = lrad_str2int(filterCompare, token, RAD_NO_COMPARE);
		    if (cmp > RAD_NO_COMPARE) {
		      token = strtok( NULL, " " );
			if ( token ) {
			    if ( tok == FILTER_IPX_DST_IPXSOCK ) {
				ipx->dstSocComp = cmp;
				ipx->dstIpxSoc = 
			    ntohs( (IpxSocket) strtol( token, NULL, 16 ));
			    } else {
				ipx->srcSocComp = cmp;
				ipx->srcIpxSoc 
				    = ntohs( (IpxSocket) strtol( token, NULL, 16 ));
			    }
			    break;
			}
		    }
		}
		goto doneErr;
	     }

	    default:
		/* no keyword match */
		goto doneErr;
	}
        token = strtok( NULL, " " ); 
    } 

    if( elements == IPX_FILTER_COMPLETE ) {
	return( 0 );
    }

doneErr:
    librad_log("ipx filter error: do not recognize \"%s\" in \"%s\"\n",
	      token, curString );
    return( -1 );
}


/*
 * parseIpFilter:
 *
 * This routine parses an IP filter string from a RADIUS
 * reply. The format of the string is:
 *
 *	ip dir action [ dstip n.n.n.n/nn ] [ srcip n.n.n.n/nn ]
 *	    [ proto [ dstport cmp value ] [ srcport cmd value ] [ est ] ] 
 *
 * Fields in [...] are optional.
 *	where:
 *
 *  ip:		Keyword to designate an IP filter. Actually this
 *			has been determined by parseFilter.
 *
 *	dir:		Filter direction. "IN" or "OUT"
 *
 *	action:		Filter action. "FORWARD" or "DROP"
 *
 *	dstip:		Keyword for destination IP address.
 *			n.n.n.n = IP address. /nn - netmask. 
 *			
 *	srcip:		Keyword for source IP address.
 *			n.n.n.n = IP address. /nn - netmask. 
 *			
 *	proto:		Optional protocol field. Either a name or
 *			number. Known names are in FilterProtoName[].
 *			
 *	dstpost:	Keyword for destination port. Only valid with tcp
 *			or udp. 'cmp' are in FilterPortType[]. 'value' can be
 *			a name or number.
 *
 *	srcpost:	Keyword for source port. Only valid with tcp
 *			or udp. 'cmp' are in FilterPortType[]. 'value' can be
 *			a name or number.
 *			
 *	est:		Keyword for TCP established. Valid only for tcp.
 *			
 * expects:
 *
 *	curEntry:	Pointer to place the filter structure
 *
 *	returns:	-1 for error or 0 for OK
 *	
 */
static int 
parseIpFilter(const char *curString, RadFilter *curEntry)
{
 
    unsigned long	elements = 0l;
    int			tok; 
    char*		token;
    RadIpFilter*	ip;

    token = strtok( NULL, " " ); 

    memset( curEntry, '\0', sizeof( RadFilter ) );
    curEntry->type = RAD_FILTER_IP; 
    ip = &curEntry->u.ip;
    ip->established = FALSE;
 
    while( token ) {
  	tok = lrad_str2int(filterKeywords, token, -1);
	switch( tok ) {
	    case FILTER_IN:
	    case FILTER_OUT:
		curEntry->indirection = tok == FILTER_IN ? TRUE: FALSE;
	        elements |= (1 << FILTER_DIRECTION );
		break;
	    case FILTER_FORWARD:
	    case FILTER_DROP:
	        elements |= (1 << FILTER_DISPOSITION );
		if( tok == FILTER_FORWARD ) {
		    curEntry->forward = TRUE;
		} else {
		    curEntry->forward = FALSE;
		}
		break;
	    case FILTER_IP_DST:
	    case FILTER_IP_SRC:
		token = strtok( NULL, " " );
		if ( token ) {
		    if( tok == FILTER_IP_DST ) {
			
		        if( ipAddressStringToValue( token, 
				 &ip->dstip, (char *)&ip->dstmask ) ) {
			    break;
			}
		    } else {
		        if( ipAddressStringToValue( token, 
				&ip->srcip, (char *)&ip->srcmask ) ) {
			    break;
			}
		    }
		} 

		librad_log("ip filter error: do not recognize \"%s\" in \"%s\"\n",
			  token, curString );
		goto doneErr ;

	    case FILTER_IP_DST_PORT:
	    case FILTER_IP_SRC_PORT:
	    {
		RadFilterComparison cmp;
		short		 port;

		token = strtok( NULL, " " );
		if ( token ) {
  		    cmp = lrad_str2int(filterCompare, token, RAD_NO_COMPARE);
		    if (cmp > RAD_NO_COMPARE) {
			token = strtok( NULL, " " );
			if ( token ) {
			    if( isAllDigit( token ) ) {
				port = atoi( (char *) token );
			    } else {
  		    	        port = lrad_str2int(filterPortType, token, -1);
			    }
			    if (port >= 0) {
				if( tok == FILTER_IP_DST_PORT ) {
				    ip->dstPortComp = cmp;
				    ip->dstport = htons( port );
				} else {
				    ip->srcPortComp = cmp;
				    ip->srcport = htons( port );
				}
				break;
			    }
			}
		    }
		}
		librad_log( "ip filter error: do not recognize \"%s\" in \"%s\"n",
			  token, curString );
		goto doneErr;
		break;
	    }
	    case FILTER_EST:
		ip->established = TRUE;
		break;
	    default:
		/* no keyword match but may match a protocol list */
		if( isAllDigit( token ) ) {
		    tok = atoi( (char *) token );
		} else {
		    tok = lrad_str2int(filterProtoName, token, -1);
		    if (tok == -1) {
			librad_log("ip filter error: do not recognize \"%s\" in \"%s\"\n",
			     token, curString );
			goto doneErr;
		    }
		}
		ip->proto = tok;
	}
        token = strtok( NULL, " " ); 
    } 

    if( elements == IP_FILTER_COMPLETE ) {
	return( 0 );
    }

doneErr:
    return( -1 );
}

/*
 * parseGenericFilter:
 *
 * This routine parses a Generic filter string from a RADIUS
 * reply. The format of the string is:
 *
 *	GENERIC dir action offset mask value [== or != ] [more]
 *
 * Fields in [...] are optional.
 *	where:
 *
 * 	generic:	Keyword to indicate a generic filter. This
 *			has been determined by parseFilter.
 *
 *	dir:		Filter direction. "IN" or "OUT"
 *
 *	action:		Filter action. "FORWARD" or "DROP"
 *
 *	offset:		A Number. Specifies an offset into a frame 
 *			to start comparing.
 *			
 *	mask:		A hexadecimal mask of bits to compare.
 *			
 *	value:		A value to compare with the masked data.
 *
 *	compNeq:	Defines type of comparison. ( "==" or "!=")
 *			Default is "==".
 *			
 *	more:		Optional keyword MORE, to represent the attachment
 *			to the next entry.
 *
 * expects:
 *
 *	curEntry:	Pointer to place the filter structure
 *
 *	returns:	-1 for error or 0 for OK
 *	
 */
static int
parseGenericFilter(const char *curString, RadFilter *curEntry)
{
    unsigned long	elements = 0l; 
    int			tok; 
    int			gstate = FILTER_GENERIC_OFFSET;
    char*		token;
    short		valLen, maskLen;
    RadGenericFilter*	gen;

    token = strtok( NULL, " " ); 

    maskLen = 0;
    memset( (char *)curEntry, '\0', sizeof( RadFilter ) );
    curEntry->type = RAD_FILTER_GENERIC;
    gen = &curEntry->u.generic;
    gen->more = FALSE; 
    gen->compNeq = FALSE;	

    while( token ) {
  	tok = lrad_str2int(filterKeywords, token, -1);
	switch( tok ) {
	    case FILTER_IN:
	    case FILTER_OUT:
		curEntry->indirection = tok == FILTER_IN ? TRUE: FALSE;
	        elements |= (1 << FILTER_DIRECTION );
		break;
	    case FILTER_FORWARD:
	    case FILTER_DROP:
	        elements |= (1 << FILTER_DISPOSITION );
		if( tok == FILTER_FORWARD ) {
		    curEntry->forward = TRUE;
		} else {
		    curEntry->forward = FALSE;
		}
		break;
	    case FILTER_GENERIC_COMPNEQ:
		gen->compNeq = TRUE;
		break;
	    case FILTER_GENERIC_COMPEQ:
		gen->compNeq = FALSE;
		break;
	    case FILTER_MORE:
		gen->more = htons( TRUE );
		break;
	    default:
	        elements |= ( 1 << gstate );
		switch( gstate ) {
		    case FILTER_GENERIC_OFFSET:
			gstate = FILTER_GENERIC_MASK;
			gen->offset = htons( atoi( (char *) token ) );
			break;
		    case FILTER_GENERIC_MASK:
			gstate = FILTER_GENERIC_VALUE;
			maskLen = a2octet( token, (char *)gen->mask );
			if( maskLen == (short) -1 ) {
			    librad_log("filter mask error: %s \n", curString );
			    goto doneErr;
			}
			break;
		    case FILTER_GENERIC_VALUE:
			gstate ++;
			valLen = a2octet( token, (char *)gen->value );
			if( valLen != maskLen ) {
			    librad_log("filter value size is not the same size as the filter mask: %s \n", 
				     curString );
			    goto doneErr;
			}
			gen->len = htons( valLen );
			break;
		    default:
			librad_log("filter: do not know %s in %s \n",
				 token, curString );
			goto doneErr;    
		}
	}
        token = strtok( NULL, " " ); 
    }

    if( elements == GENERIC_FILTER_COMPLETE ) {
	return( 0 );
    }

doneErr:
    return( -1 );
}
		       

/*
 * filterBinary:
 *
 * This routine will call routines to parse entries from an ASCII format
 * to a binary format recognized by the Ascend boxes.
 *
 *	pair:			Pointer to value_pair to place return.
 *
 *	valstr:			The string to parse	
 *
 *	return:			-1 for error or 0.
 */
int 
filterBinary(VALUE_PAIR *pair, const char *valstr)
{

    char*		token;
    unsigned long	tok;
    int			rc;
    RadFilter		radFil, *filt;
    RadGenericFilter*	gen;
    static VALUE_PAIR	*prevRadPair = NULL;
    char		*copied_valstr;

    rc = -1;

    /*
     *  Copy the valstr, so we don't smash it in place via strtok!
     */
    copied_valstr = strdup(valstr);
    if (!copied_valstr) return -1;

    token = strtok(copied_valstr, " " );
    tok = lrad_str2int(filterType, token, -1);
    pair->length = SIZEOF_RADFILTER;
    switch( tok ) {
      case RAD_FILTER_GENERIC:
	rc = parseGenericFilter( valstr, &radFil );
	break;
      case RAD_FILTER_IP:
	rc = parseIpFilter( valstr, &radFil );
	break;
      case RAD_FILTER_IPX:
	rc = parseIpxFilter( valstr, &radFil );
        break;
      default:
	librad_log("filterBinary: unknown filter type \"%s\"", token);
	free(copied_valstr);
	return -1;
	break;
    }
    free(copied_valstr);
    copied_valstr = NULL;

    /*
     * if 'more' is set then this new entry must exist, be a 
     * FILTER_GENERIC_TYPE, direction and disposition must match for 
     * the previous 'more' to be valid. If any should fail then TURN OFF 
     * previous 'more'
     */
    if( prevRadPair ) {
	filt = ( RadFilter * )prevRadPair->strvalue;
	if(( tok != FILTER_GENERIC_TYPE ) || (rc == -1 ) ||
	   ( prevRadPair->attribute != pair->attribute ) || 
	   ( filt->indirection != radFil.indirection ) || 
	   ( filt->forward != radFil.forward ) ) {
	    gen = &filt->u.generic;
	    gen->more = FALSE;
	    librad_log("filterBinary:  'more' for previous entry doesn't match: %s.\n",
		     valstr);
	}
    }
    prevRadPair = NULL;
    if( rc != -1 && tok == FILTER_GENERIC_TYPE ) {
	if( radFil.u.generic.more ) {
	    prevRadPair = pair;
	} 
    }

    if( rc != -1 ) {
	memcpy( pair->strvalue, (char *) &radFil, pair->length );
    }
    return(rc);
}

/********************************************************************/

/*
 *  The following code was written specifically for the FreeRADIUS
 *  server by Alan DeKok <aland@ox.org>, and as such, falls under
 *  the GPL, and not under the previous Ascend license.
 */

/*
 *	Print an Ascend binary filter attribute to a string,
 *	Grrr... Ascend makes the server do this work, instead
 *	of doing it on the NAS.
 *
 *	Note we don't bother checking 'len' after the snprintf's.
 *	This function should ONLY be called with a large (~1k) buffer.
 */
void print_abinary(VALUE_PAIR *vp, u_char *buffer, int len)
{
  int i;
  char *p;
  RadFilter	filter;
  
  static const char *action[] = {"drop", "forward"};
  static const char *direction[] = {"output", "input"};
  
  p = (char *)buffer;

  /*
   *  Just for paranoia: wrong size filters get printed as octets
   */
  if (vp->length > SIZEOF_RADFILTER) {
    strcpy(p, "0x");
    p += 2;
    for (i = 0; i < vp->length; i++) {
      sprintf(p, " %02x", vp->strvalue[i]);
      p += 3;
    }
    return;
  }

  memcpy(&filter, vp->strvalue, SIZEOF_RADFILTER); /* alignment issues */
  *(p++) = '"';
  len -= 3;			/* account for leading & trailing quotes */

  i = snprintf(p, len, "%s %s %s",
	       lrad_int2str(filterType, filter.type, "??"),
	       direction[filter.indirection & 0x01],
	       action[filter.forward & 0x01]);

  p += i;
  len -= i;

  /*
   *	Handle IP filters
   */
  if (filter.type == RAD_FILTER_IP) {

    i =  snprintf(p, len, " %s", 
		  lrad_int2str(filterProtoName, filter.u.ip.proto, "??"));
    p += i;
    len -= i;
    
    if (filter.u.ip.dstip) {
      i = snprintf(p, len, " dstip %d.%d.%d.%d/%d",
		   ((u_char *) &filter.u.ip.dstip)[0],
		   ((u_char *) &filter.u.ip.dstip)[1],
		   ((u_char *) &filter.u.ip.dstip)[2],
		   ((u_char *) &filter.u.ip.dstip)[3],
		   filter.u.ip.dstmask);
      p += i;
      len -= i;
    }
    
    if (filter.u.ip.srcip) {
      i = snprintf(p, len, " srcip %d.%d.%d.%d/%d",
		   ((u_char *) &filter.u.ip.srcip)[0],
		   ((u_char *) &filter.u.ip.srcip)[1],
		   ((u_char *) &filter.u.ip.srcip)[2],
		   ((u_char *) &filter.u.ip.srcip)[3],
		   filter.u.ip.srcmask);
      p += i;
      len -= i;
    }

    if (filter.u.ip.dstPortComp > RAD_NO_COMPARE) {
      i = snprintf(p, len, " dstport %s %d",
		   lrad_int2str(filterCompare, filter.u.ip.dstPortComp, "??"),
		   ntohs(filter.u.ip.dstport));
      p += i;
      len -= i;
    }
    
    if (filter.u.ip.srcPortComp > RAD_NO_COMPARE) {
      i = snprintf(p, len, " srcport %s %d",
		   lrad_int2str(filterCompare, filter.u.ip.srcPortComp, "??"),
		   ntohs(filter.u.ip.srcport));
      p += i;
      len -= i;
    }

    if (filter.u.ip.established) {
      i = snprintf(p, len, " est");
      p += i;
      len -= i;
    }

    /*
     *	Handle IPX filters
     */
  } else if (filter.type == RAD_FILTER_IPX) {
    /* print for source */
    if (filter.u.ipx.srcIpxNet) {
      i = snprintf(p, len, " srcipxnet 0x%04x srcipxnode 0x%02x%02x%02x%02x%02x%02x",
		  (unsigned int)ntohl(filter.u.ipx.srcIpxNet),
		  filter.u.ipx.srcIpxNode[0], filter.u.ipx.srcIpxNode[1], 
		  filter.u.ipx.srcIpxNode[2], filter.u.ipx.srcIpxNode[3], 
		  filter.u.ipx.srcIpxNode[4], filter.u.ipx.srcIpxNode[5]);
      p += i;
      len -= i;

      if (filter.u.ipx.srcSocComp > RAD_NO_COMPARE) {
	i = snprintf(p, len, " srcipxsock %s 0x%04x",
		     lrad_int2str(filterCompare, filter.u.ipx.srcSocComp, "??"),
		     ntohs(filter.u.ipx.srcIpxSoc));
	p += i;
	len -= i;
      }
    }

    /* same for destination */
    if (filter.u.ipx.dstIpxNet) {
      i = snprintf(p, len, " dstipxnet 0x%04x dstipxnode 0x%02x%02x%02x%02x%02x%02x",
		  (unsigned int)ntohl(filter.u.ipx.dstIpxNet),
		  filter.u.ipx.dstIpxNode[0], filter.u.ipx.dstIpxNode[1], 
		  filter.u.ipx.dstIpxNode[2], filter.u.ipx.dstIpxNode[3], 
		  filter.u.ipx.dstIpxNode[4], filter.u.ipx.dstIpxNode[5]);
      p += i;
      len -= i;

      if (filter.u.ipx.dstSocComp > RAD_NO_COMPARE) {
	i = snprintf(p, len, " dstipxsock %s 0x%04x",
		     lrad_int2str(filterCompare, filter.u.ipx.dstSocComp, "??"),
		     ntohs(filter.u.ipx.dstIpxSoc));
	p += i;
	len -= i;
      }
    }


  } else if (filter.type == RAD_FILTER_GENERIC) {
    int count;

    i = snprintf(p, len, " %d ", filter.u.generic.offset);
    p += i;
    i -= len;

    /* show the mask */
    for (count = 0; count < ntohs(filter.u.generic.len); count++) {
      i = snprintf(p, len, "%02x", filter.u.generic.mask[count]);
      p += i;
      len -= i;
    }

    strcpy(p, " ");
    p++;
    len--;

    /* show the value */
    for (count = 0; count < ntohs(filter.u.generic.len); count++) {
      i = snprintf(p, len, "%02x", filter.u.generic.value[count]);
      p += i;
      len -= i;
    }

    i = snprintf(p, len, " %s", (filter.u.generic.compNeq) ? "!=" : "==");
    p += i;
    len -= i;
  }

  *(p++) = '"';
  *p = '\0';
}
