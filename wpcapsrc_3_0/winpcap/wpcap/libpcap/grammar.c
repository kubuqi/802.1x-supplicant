/* A Bison parser, made from ../libpcap/grammar.y
   by GNU bison 1.35.  */

#define YYBISON 1  /* Identify Bison output.  */

#define yyparse pcap_parse
#define yylex pcap_lex
#define yyerror pcap_error
#define yylval pcap_lval
#define yychar pcap_char
#define yydebug pcap_debug
#define yynerrs pcap_nerrs
# define	DST	257
# define	SRC	258
# define	HOST	259
# define	GATEWAY	260
# define	NET	261
# define	MASK	262
# define	PORT	263
# define	LESS	264
# define	GREATER	265
# define	PROTO	266
# define	PROTOCHAIN	267
# define	CBYTE	268
# define	ARP	269
# define	RARP	270
# define	IP	271
# define	SCTP	272
# define	TCP	273
# define	UDP	274
# define	ICMP	275
# define	IGMP	276
# define	IGRP	277
# define	PIM	278
# define	VRRP	279
# define	ATALK	280
# define	AARP	281
# define	DECNET	282
# define	LAT	283
# define	SCA	284
# define	MOPRC	285
# define	MOPDL	286
# define	TK_BROADCAST	287
# define	TK_MULTICAST	288
# define	NUM	289
# define	INBOUND	290
# define	OUTBOUND	291
# define	LINK	292
# define	GEQ	293
# define	LEQ	294
# define	NEQ	295
# define	ID	296
# define	EID	297
# define	HID	298
# define	HID6	299
# define	AID	300
# define	LSH	301
# define	RSH	302
# define	LEN	303
# define	IPV6	304
# define	ICMPV6	305
# define	AH	306
# define	ESP	307
# define	VLAN	308
# define	ISO	309
# define	ESIS	310
# define	CLNP	311
# define	ISIS	312
# define	L1	313
# define	L2	314
# define	IIH	315
# define	LSP	316
# define	SNP	317
# define	CSNP	318
# define	PSNP	319
# define	STP	320
# define	IPX	321
# define	NETBEUI	322
# define	LANE	323
# define	LLC	324
# define	METAC	325
# define	BCC	326
# define	SC	327
# define	ILMIC	328
# define	OAMF4EC	329
# define	OAMF4SC	330
# define	OAM	331
# define	OAMF4	332
# define	CONNECTMSG	333
# define	METACONNECT	334
# define	VPI	335
# define	VCI	336
# define	OR	337
# define	AND	338
# define	UMINUS	339

#line 1 "../libpcap/grammar.y"

/*
 * Copyright (c) 1988, 1989, 1990, 1991, 1992, 1993, 1994, 1995, 1996
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that: (1) source code distributions
 * retain the above copyright notice and this paragraph in its entirety, (2)
 * distributions including binary code include the above copyright notice and
 * this paragraph in its entirety in the documentation or other materials
 * provided with the distribution, and (3) all advertising materials mentioning
 * features or use of this software display the following acknowledgement:
 * ``This product includes software developed by the University of California,
 * Lawrence Berkeley Laboratory and its contributors.'' Neither the name of
 * the University nor the names of its contributors may be used to endorse
 * or promote products derived from this software without specific prior
 * written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */
#ifndef lint
static const char rcsid[] =
    "@(#) $Header: /tcpdump/master/libpcap/grammar.y,v 1.78 2002/12/06 00:01:34 hannes Exp $ (LBL)";
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WIN32
#include <pcap-stdinc.h>
#else /* WIN32 */
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#endif /* WIN32 */

#include <stdlib.h>

#ifndef WIN32
#if __STDC__
struct mbuf;
struct rtentry;
#endif

#include <netinet/in.h>
#endif /* WIN32 */

#include <stdio.h>

#include "pcap-int.h"

#include "gencode.h"
#include <pcap-namedb.h>

#ifdef HAVE_OS_PROTO_H
#include "os-proto.h"
#endif

#define QSET(q, p, d, a) (q).proto = (p),\
			 (q).dir = (d),\
			 (q).addr = (a)

int n_errors = 0;

static struct qual qerr = { Q_UNDEF, Q_UNDEF, Q_UNDEF, Q_UNDEF };

static void
yyerror(char *msg)
{
	++n_errors;
	bpf_error("%s", msg);
	/* NOTREACHED */
}

#ifndef YYBISON
int yyparse(void);

int
pcap_parse()
{
	return (yyparse());
}
#endif


#line 90 "../libpcap/grammar.y"
#ifndef YYSTYPE
typedef union {
	int i;
	bpf_u_int32 h;
	u_char *e;
	char *s;
	struct stmt *stmt;
	struct arth *a;
	struct {
		struct qual q;
		int atmfieldtype;
		struct block *b;
	} blk;
	struct block *rblk;
} yystype;
# define YYSTYPE yystype
# define YYSTYPE_IS_TRIVIAL 1
#endif
#ifndef YYDEBUG
# define YYDEBUG 0
#endif



#define	YYFINAL		210
#define	YYFLAG		-32768
#define	YYNTBASE	101

/* YYTRANSLATE(YYLEX) -- Bison token number corresponding to YYLEX. */
#define YYTRANSLATE(x) ((unsigned)(x) <= 339 ? yytranslate[x] : 133)

/* YYTRANSLATE[YYLEX] -- Bison token number corresponding to YYLEX. */
static const char yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    85,     2,     2,     2,     2,    87,     2,
      94,    93,    90,    88,     2,    89,     2,    91,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,   100,     2,
      97,    96,    95,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    98,     2,    99,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,    86,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     3,     4,     5,
       6,     7,     8,     9,    10,    11,    12,    13,    14,    15,
      16,    17,    18,    19,    20,    21,    22,    23,    24,    25,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,    61,    62,    63,    64,    65,
      66,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    77,    78,    79,    80,    81,    82,    83,    84,    92
};

#if YYDEBUG
static const short yyprhs[] =
{
       0,     0,     3,     5,     6,     8,    12,    16,    20,    24,
      26,    28,    30,    32,    36,    38,    42,    46,    48,    52,
      54,    56,    58,    61,    63,    65,    67,    71,    75,    77,
      79,    81,    84,    88,    91,    94,    97,   100,   103,   106,
     110,   112,   116,   120,   122,   124,   126,   129,   131,   132,
     134,   136,   140,   144,   148,   152,   154,   156,   158,   160,
     162,   164,   166,   168,   170,   172,   174,   176,   178,   180,
     182,   184,   186,   188,   190,   192,   194,   196,   198,   200,
     202,   204,   206,   208,   210,   212,   214,   216,   218,   220,
     222,   224,   226,   228,   230,   232,   234,   237,   240,   243,
     246,   251,   253,   255,   258,   260,   262,   264,   266,   268,
     270,   272,   274,   276,   281,   288,   292,   296,   300,   304,
     308,   312,   316,   320,   323,   327,   329,   331,   333,   335,
     337,   339,   341,   345,   347,   349,   351,   353,   355,   357,
     359,   361,   363,   365,   367,   369,   371,   373,   375,   378,
     381,   385,   387,   389
};
static const short yyrhs[] =
{
     102,   103,     0,   102,     0,     0,   112,     0,   103,   104,
     112,     0,   103,   104,   106,     0,   103,   105,   112,     0,
     103,   105,   106,     0,    84,     0,    83,     0,   107,     0,
     126,     0,   109,   110,    93,     0,    42,     0,    44,    91,
      35,     0,    44,     8,    44,     0,    44,     0,    45,    91,
      35,     0,    45,     0,    43,     0,    46,     0,   108,   106,
       0,    85,     0,    94,     0,   107,     0,   111,   104,   106,
       0,   111,   105,   106,     0,   126,     0,   110,     0,   114,
       0,   108,   112,     0,   115,   116,   117,     0,   115,   116,
       0,   115,   117,     0,   115,    12,     0,   115,    13,     0,
     115,   118,     0,   113,   106,     0,   109,   103,    93,     0,
     119,     0,   123,   121,   123,     0,   123,   122,   123,     0,
     120,     0,   127,     0,   128,     0,   129,   130,     0,   119,
       0,     0,     4,     0,     3,     0,     4,    83,     3,     0,
       3,    83,     4,     0,     4,    84,     3,     0,     3,    84,
       4,     0,     5,     0,     7,     0,     9,     0,     6,     0,
      38,     0,    17,     0,    15,     0,    16,     0,    18,     0,
      19,     0,    20,     0,    21,     0,    22,     0,    23,     0,
      24,     0,    25,     0,    26,     0,    27,     0,    28,     0,
      29,     0,    30,     0,    32,     0,    31,     0,    50,     0,
      51,     0,    52,     0,    53,     0,    55,     0,    56,     0,
      58,     0,    59,     0,    60,     0,    61,     0,    62,     0,
      63,     0,    65,     0,    64,     0,    57,     0,    66,     0,
      67,     0,    68,     0,   115,    33,     0,   115,    34,     0,
      10,    35,     0,    11,    35,     0,    14,    35,   125,    35,
       0,    36,     0,    37,     0,    54,   126,     0,    54,     0,
      95,     0,    39,     0,    96,     0,    40,     0,    97,     0,
      41,     0,   126,     0,   124,     0,   119,    98,   123,    99,
       0,   119,    98,   123,   100,    35,    99,     0,   123,    88,
     123,     0,   123,    89,   123,     0,   123,    90,   123,     0,
     123,    91,   123,     0,   123,    87,   123,     0,   123,    86,
     123,     0,   123,    47,   123,     0,   123,    48,   123,     0,
      89,   123,     0,   109,   124,    93,     0,    49,     0,    87,
       0,    86,     0,    97,     0,    95,     0,    96,     0,    35,
       0,   109,   126,    93,     0,    69,     0,    70,     0,    71,
       0,    72,     0,    75,     0,    76,     0,    73,     0,    74,
       0,    77,     0,    78,     0,    79,     0,    80,     0,    81,
       0,    82,     0,   131,     0,   121,    35,     0,   122,    35,
       0,   109,   132,    93,     0,    35,     0,   131,     0,   132,
     105,   131,     0
};

#endif

#if YYDEBUG
/* YYRLINE[YYN] -- source line where rule number YYN was defined. */
static const short yyrline[] =
{
       0,   152,   156,   158,   160,   161,   162,   163,   164,   166,
     168,   170,   171,   173,   175,   176,   178,   180,   185,   194,
     203,   212,   221,   223,   225,   227,   228,   229,   231,   233,
     235,   236,   238,   239,   240,   241,   242,   243,   245,   246,
     247,   248,   250,   252,   253,   254,   255,   258,   259,   262,
     263,   264,   265,   266,   267,   270,   271,   272,   275,   277,
     278,   279,   280,   281,   282,   283,   284,   285,   286,   287,
     288,   289,   290,   291,   292,   293,   294,   295,   296,   297,
     298,   299,   300,   301,   302,   303,   304,   305,   306,   307,
     308,   309,   310,   311,   312,   313,   315,   316,   317,   318,
     319,   320,   321,   322,   323,   325,   326,   327,   329,   330,
     331,   333,   334,   336,   337,   338,   339,   340,   341,   342,
     343,   344,   345,   346,   347,   348,   350,   351,   352,   353,
     354,   356,   357,   359,   360,   361,   362,   363,   364,   365,
     366,   368,   369,   370,   371,   374,   375,   377,   378,   379,
     380,   382,   389,   390
};
#endif


#if (YYDEBUG) || defined YYERROR_VERBOSE

/* YYTNAME[TOKEN_NUM] -- String name of the token TOKEN_NUM. */
static const char *const yytname[] =
{
  "$", "error", "$undefined.", "DST", "SRC", "HOST", "GATEWAY", "NET", 
  "MASK", "PORT", "LESS", "GREATER", "PROTO", "PROTOCHAIN", "CBYTE", 
  "ARP", "RARP", "IP", "SCTP", "TCP", "UDP", "ICMP", "IGMP", "IGRP", 
  "PIM", "VRRP", "ATALK", "AARP", "DECNET", "LAT", "SCA", "MOPRC", 
  "MOPDL", "TK_BROADCAST", "TK_MULTICAST", "NUM", "INBOUND", "OUTBOUND", 
  "LINK", "GEQ", "LEQ", "NEQ", "ID", "EID", "HID", "HID6", "AID", "LSH", 
  "RSH", "LEN", "IPV6", "ICMPV6", "AH", "ESP", "VLAN", "ISO", "ESIS", 
  "CLNP", "ISIS", "L1", "L2", "IIH", "LSP", "SNP", "CSNP", "PSNP", "STP", 
  "IPX", "NETBEUI", "LANE", "LLC", "METAC", "BCC", "SC", "ILMIC", 
  "OAMF4EC", "OAMF4SC", "OAM", "OAMF4", "CONNECTMSG", "METACONNECT", 
  "VPI", "VCI", "OR", "AND", "'!'", "'|'", "'&'", "'+'", "'-'", "'*'", 
  "'/'", "UMINUS", "')'", "'('", "'>'", "'='", "'<'", "'['", "']'", "':'", 
  "prog", "null", "expr", "and", "or", "id", "nid", "not", "paren", "pid", 
  "qid", "term", "head", "rterm", "pqual", "dqual", "aqual", "ndaqual", 
  "pname", "other", "relop", "irelop", "arth", "narth", "byteop", "pnum", 
  "atmtype", "atmmultitype", "atmfield", "atmvalue", "atmfieldvalue", 
  "atmlistvalue", 0
};
#endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives. */
static const short yyr1[] =
{
       0,   101,   101,   102,   103,   103,   103,   103,   103,   104,
     105,   106,   106,   106,   107,   107,   107,   107,   107,   107,
     107,   107,   107,   108,   109,   110,   110,   110,   111,   111,
     112,   112,   113,   113,   113,   113,   113,   113,   114,   114,
     114,   114,   114,   114,   114,   114,   114,   115,   115,   116,
     116,   116,   116,   116,   116,   117,   117,   117,   118,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   119,   119,   119,   119,   119,   120,   120,   120,   120,
     120,   120,   120,   120,   120,   121,   121,   121,   122,   122,
     122,   123,   123,   124,   124,   124,   124,   124,   124,   124,
     124,   124,   124,   124,   124,   124,   125,   125,   125,   125,
     125,   126,   126,   127,   127,   127,   127,   127,   127,   127,
     127,   128,   128,   128,   128,   129,   129,   130,   130,   130,
     130,   131,   132,   132
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN. */
static const short yyr2[] =
{
       0,     2,     1,     0,     1,     3,     3,     3,     3,     1,
       1,     1,     1,     3,     1,     3,     3,     1,     3,     1,
       1,     1,     2,     1,     1,     1,     3,     3,     1,     1,
       1,     2,     3,     2,     2,     2,     2,     2,     2,     3,
       1,     3,     3,     1,     1,     1,     2,     1,     0,     1,
       1,     3,     3,     3,     3,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     2,     2,     2,     2,
       4,     1,     1,     2,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     4,     6,     3,     3,     3,     3,     3,
       3,     3,     3,     2,     3,     1,     1,     1,     1,     1,
       1,     1,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       3,     1,     1,     3
};

/* YYDEFACT[S] -- default rule to reduce with in state S when YYTABLE
   doesn't specify something else to do.  Zero means the default is an
   error. */
static const short yydefact[] =
{
       3,    48,     0,     0,     0,    61,    62,    60,    63,    64,
      65,    66,    67,    68,    69,    70,    71,    72,    73,    74,
      75,    77,    76,   131,   101,   102,    59,   125,    78,    79,
      80,    81,   104,    82,    83,    92,    84,    85,    86,    87,
      88,    89,    91,    90,    93,    94,    95,   133,   134,   135,
     136,   139,   140,   137,   138,   141,   142,   143,   144,   145,
     146,    23,     0,    24,     1,    48,    48,     4,     0,    30,
       0,    47,    43,     0,   112,   111,    44,    45,     0,    98,
      99,     0,     0,   103,     0,     0,   123,    10,     9,    48,
      48,    31,     0,   112,   111,    14,    20,    17,    19,    21,
      38,    11,     0,     0,    12,    50,    49,    55,    58,    56,
      57,    35,    36,    96,    97,    33,    34,    37,     0,   106,
     108,   110,     0,     0,     0,     0,     0,     0,     0,     0,
     105,   107,   109,     0,     0,   151,     0,     0,     0,    46,
     147,   127,   126,   129,   130,   128,     0,     0,     0,     6,
      48,    48,     5,   111,     8,     7,    39,   124,   132,     0,
       0,     0,    22,    25,    29,     0,    28,     0,     0,     0,
       0,    32,     0,   121,   122,   120,   119,   115,   116,   117,
     118,    41,    42,   152,     0,   148,   149,   100,   111,    16,
      15,    18,    13,     0,     0,    52,    54,    51,    53,   113,
       0,   150,     0,    26,    27,     0,   153,   114,     0,     0,
       0
};

static const short yydefgoto[] =
{
     208,     1,    92,    89,    90,   162,   101,   102,    84,   164,
     165,    67,    68,    69,    70,   115,   116,   117,    85,    72,
     133,   134,    73,    74,   146,    75,    76,    77,    78,   139,
     140,   184
};

static const short yypact[] =
{
  -32768,   180,   -18,   -14,    -3,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,   -19,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,-32768,
  -32768,-32768,   422,-32768,   -58,   342,   342,-32768,    94,-32768,
     500,     7,-32768,   452,-32768,-32768,-32768,-32768,    21,-32768,
  -32768,   -67,   -19,-32768,   422,   -53,-32768,-32768,-32768,   261,
     261,-32768,   -71,   -38,   -36,-32768,-32768,    -5,   -39,-32768,
  -32768,-32768,    94,    94,-32768,   -43,    -4,-32768,-32768,-32768,
  -32768,-32768,-32768,-32768,-32768,    37,-32768,-32768,   422,-32768,
  -32768,-32768,   422,   422,   422,   422,   422,   422,   422,   422,
  -32768,-32768,-32768,   422,   422,-32768,    28,    31,    43,-32768,
  -32768,-32768,-32768,-32768,-32768,-32768,    52,   -36,   136,-32768,
     261,   261,-32768,    11,-32768,-32768,-32768,-32768,-32768,    26,
      61,    71,-32768,-32768,    14,   -58,   -36,   104,   105,   108,
     109,-32768,    35,   -41,   -41,   297,   378,    -6,    -6,-32768,
  -32768,   136,   136,-32768,   -78,-32768,-32768,-32768,   -50,-32768,
  -32768,-32768,-32768,    94,    94,-32768,-32768,-32768,-32768,-32768,
      78,-32768,    28,-32768,-32768,    15,-32768,-32768,   130,   131,
  -32768
};

static const short yypgoto[] =
{
  -32768,-32768,   132,   -24,  -157,   -66,   -97,     3,    -1,-32768,
  -32768,   -51,-32768,-32768,-32768,-32768,    27,-32768,     8,-32768,
      65,    66,    48,   -48,-32768,   -31,-32768,-32768,-32768,-32768,
    -126,-32768
};


#define	YYLAST		549


static const short yytable[] =
{
      66,    83,   100,   159,    65,    87,   163,   -40,   194,    71,
     183,   -12,    87,    88,    91,   201,    23,    79,    93,   141,
     142,    80,   156,   149,   154,    87,    88,   202,   143,   144,
     145,    82,    81,   -28,   -28,    94,    93,   104,   152,   155,
     167,   168,   107,   158,   109,   118,   110,   126,   127,   128,
     129,   147,   161,    94,   163,   157,   135,   158,   153,   153,
     119,   120,   121,   135,    66,    66,   185,   103,    65,    65,
     189,   104,   166,    71,    71,    63,   206,   136,   186,   169,
     170,    82,   122,   123,   128,   129,   160,   187,   151,   151,
     -40,   -40,   150,   150,   -12,   -12,   190,    71,    71,    91,
     -40,   103,    82,    93,   -12,   118,   191,   192,   195,   196,
      86,   197,   198,   205,   207,    63,   130,   131,   132,   153,
     188,   124,   125,   126,   127,   128,   129,   203,   204,    23,
     209,   210,   148,    64,   199,   200,    95,    96,    97,    98,
      99,   193,   171,   137,   138,     0,     0,     0,     0,   151,
      66,     0,     0,   150,   150,     0,     0,     0,    71,    71,
       0,     0,   104,   104,     0,     0,   172,     0,     0,     0,
     173,   174,   175,   176,   177,   178,   179,   180,     0,    61,
      -2,   181,   182,   122,   123,     0,     0,     0,    63,     0,
       2,     3,   103,   103,     4,     5,     6,     7,     8,     9,
      10,    11,    12,    13,    14,    15,    16,    17,    18,    19,
      20,    21,    22,     0,     0,    23,    24,    25,    26,     0,
       0,     0,   124,   125,   126,   127,   128,   129,     0,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    41,    42,    43,    44,    45,    46,    47,
      48,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,     0,     0,    61,     0,     0,     0,    62,
       0,     2,     3,     0,    63,     4,     5,     6,     7,     8,
       9,    10,    11,    12,    13,    14,    15,    16,    17,    18,
      19,    20,    21,    22,     0,     0,    23,    24,    25,    26,
       0,     0,     0,    95,    96,    97,    98,    99,     0,     0,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    43,    44,    45,    46,
      47,    48,    49,    50,    51,    52,    53,    54,    55,    56,
      57,    58,    59,    60,   122,   123,    61,     0,     0,     0,
      62,     0,     2,     3,     0,    63,     4,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,     0,    23,    24,    25,
      26,     0,     0,     0,   125,   126,   127,   128,   129,     0,
       0,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,    47,    48,    49,    50,    51,    52,    53,    54,    55,
      56,    57,    58,    59,    60,   122,   123,    61,     0,     0,
       0,    62,     0,     0,     0,     0,    63,     5,     6,     7,
       8,     9,    10,    11,    12,    13,    14,    15,    16,    17,
      18,    19,    20,    21,    22,     0,     0,    23,     0,     0,
      26,     0,     0,     0,     0,     0,   126,   127,   128,   129,
       0,    27,    28,    29,    30,    31,     0,    33,    34,    35,
      36,    37,    38,    39,    40,    41,    42,    43,    44,    45,
      46,   119,   120,   121,     0,     0,     0,     0,     0,   122,
     123,     0,     0,   105,   106,   107,   108,   109,     0,   110,
       0,    62,   111,   112,     0,     0,    63,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,   113,   114,     0,     0,     0,   124,   125,
     126,   127,   128,   129,     0,     0,     0,   130,   131,   132
};

static const short yycheck[] =
{
       1,    32,    68,     8,     1,    83,   103,     0,   165,     1,
     136,     0,    83,    84,    65,    93,    35,    35,    66,    86,
      87,    35,    93,    89,    90,    83,    84,   184,    95,    96,
      97,    32,    35,    83,    84,    66,    84,    68,    89,    90,
      83,    84,     5,    93,     7,    98,     9,    88,    89,    90,
      91,    82,    91,    84,   151,    93,    35,    93,    89,    90,
      39,    40,    41,    35,    65,    66,    35,    68,    65,    66,
      44,   102,   103,    65,    66,    94,   202,    78,    35,    83,
      84,    82,    47,    48,    90,    91,    91,    35,    89,    90,
      83,    84,    89,    90,    83,    84,    35,    89,    90,   150,
      93,   102,   103,   151,    93,    98,    35,    93,     4,     4,
      62,     3,     3,    35,    99,    94,    95,    96,    97,   150,
     151,    86,    87,    88,    89,    90,    91,   193,   194,    35,
       0,     0,    84,     1,    99,   100,    42,    43,    44,    45,
      46,   165,   115,    78,    78,    -1,    -1,    -1,    -1,   150,
     151,    -1,    -1,   150,   151,    -1,    -1,    -1,   150,   151,
      -1,    -1,   193,   194,    -1,    -1,   118,    -1,    -1,    -1,
     122,   123,   124,   125,   126,   127,   128,   129,    -1,    85,
       0,   133,   134,    47,    48,    -1,    -1,    -1,    94,    -1,
      10,    11,   193,   194,    14,    15,    16,    17,    18,    19,
      20,    21,    22,    23,    24,    25,    26,    27,    28,    29,
      30,    31,    32,    -1,    -1,    35,    36,    37,    38,    -1,
      -1,    -1,    86,    87,    88,    89,    90,    91,    -1,    49,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79,
      80,    81,    82,    -1,    -1,    85,    -1,    -1,    -1,    89,
      -1,    10,    11,    -1,    94,    14,    15,    16,    17,    18,
      19,    20,    21,    22,    23,    24,    25,    26,    27,    28,
      29,    30,    31,    32,    -1,    -1,    35,    36,    37,    38,
      -1,    -1,    -1,    42,    43,    44,    45,    46,    -1,    -1,
      49,    50,    51,    52,    53,    54,    55,    56,    57,    58,
      59,    60,    61,    62,    63,    64,    65,    66,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    77,    78,
      79,    80,    81,    82,    47,    48,    85,    -1,    -1,    -1,
      89,    -1,    10,    11,    -1,    94,    14,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    -1,    35,    36,    37,
      38,    -1,    -1,    -1,    87,    88,    89,    90,    91,    -1,
      -1,    49,    50,    51,    52,    53,    54,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    77,
      78,    79,    80,    81,    82,    47,    48,    85,    -1,    -1,
      -1,    89,    -1,    -1,    -1,    -1,    94,    15,    16,    17,
      18,    19,    20,    21,    22,    23,    24,    25,    26,    27,
      28,    29,    30,    31,    32,    -1,    -1,    35,    -1,    -1,
      38,    -1,    -1,    -1,    -1,    -1,    88,    89,    90,    91,
      -1,    49,    50,    51,    52,    53,    -1,    55,    56,    57,
      58,    59,    60,    61,    62,    63,    64,    65,    66,    67,
      68,    39,    40,    41,    -1,    -1,    -1,    -1,    -1,    47,
      48,    -1,    -1,     3,     4,     5,     6,     7,    -1,     9,
      -1,    89,    12,    13,    -1,    -1,    94,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      -1,    -1,    -1,    33,    34,    -1,    -1,    -1,    86,    87,
      88,    89,    90,    91,    -1,    -1,    -1,    95,    96,    97
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "/usr/share/bison/bison.simple"

/* Skeleton output parser for bison,

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002 Free Software
   Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

/* This is the parser code that is written into each bison parser when
   the %semantic_parser declaration is not specified in the grammar.
   It was written by Richard Stallman by simplifying the hairy parser
   used when %semantic_parser is specified.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

#if ! defined (yyoverflow) || defined (YYERROR_VERBOSE)

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# if YYSTACK_USE_ALLOCA
#  define YYSTACK_ALLOC alloca
# else
#  ifndef YYSTACK_USE_ALLOCA
#   if defined (alloca) || defined (_ALLOCA_H)
#    define YYSTACK_ALLOC alloca
#   else
#    ifdef __GNUC__
#     define YYSTACK_ALLOC __builtin_alloca
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning. */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
# else
#  if defined (__STDC__) || defined (__cplusplus)
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   define YYSIZE_T size_t
#  endif
#  define YYSTACK_ALLOC malloc
#  define YYSTACK_FREE free
# endif
#endif /* ! defined (yyoverflow) || defined (YYERROR_VERBOSE) */


#if (! defined (yyoverflow) \
     && (! defined (__cplusplus) \
	 || (YYLTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  short yyss;
  YYSTYPE yyvs;
# if YYLSP_NEEDED
  YYLTYPE yyls;
# endif
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAX (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# if YYLSP_NEEDED
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE) + sizeof (YYLTYPE))	\
      + 2 * YYSTACK_GAP_MAX)
# else
#  define YYSTACK_BYTES(N) \
     ((N) * (sizeof (short) + sizeof (YYSTYPE))				\
      + YYSTACK_GAP_MAX)
# endif

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  register YYSIZE_T yyi;		\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (0)
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack)					\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack, Stack, yysize);				\
	Stack = &yyptr->Stack;						\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAX;	\
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (0)

#endif


#if ! defined (YYSIZE_T) && defined (__SIZE_TYPE__)
# define YYSIZE_T __SIZE_TYPE__
#endif
#if ! defined (YYSIZE_T) && defined (size_t)
# define YYSIZE_T size_t
#endif
#if ! defined (YYSIZE_T)
# if defined (__STDC__) || defined (__cplusplus)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# endif
#endif
#if ! defined (YYSIZE_T)
# define YYSIZE_T unsigned int
#endif

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	goto yyacceptlab
#define YYABORT 	goto yyabortlab
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { 								\
      yyerror ("syntax error: cannot back up");			\
      YYERROR;							\
    }								\
while (0)

#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Compute the default location (before the actions
   are run).

   When YYLLOC_DEFAULT is run, CURRENT is set the location of the
   first token.  By default, to implement support for ranges, extend
   its range to the last symbol.  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)       	\
   Current.last_line   = Rhs[N].last_line;	\
   Current.last_column = Rhs[N].last_column;
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#if YYPURE
# if YYLSP_NEEDED
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, &yylloc, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval, &yylloc)
#  endif
# else /* !YYLSP_NEEDED */
#  ifdef YYLEX_PARAM
#   define YYLEX		yylex (&yylval, YYLEX_PARAM)
#  else
#   define YYLEX		yylex (&yylval)
#  endif
# endif /* !YYLSP_NEEDED */
#else /* !YYPURE */
# define YYLEX			yylex ()
#endif /* !YYPURE */


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (0)
/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
#endif /* !YYDEBUG */

/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   SIZE_MAX < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#if YYMAXDEPTH == 0
# undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif

#ifdef YYERROR_VERBOSE

# ifndef yystrlen
#  if defined (__GLIBC__) && defined (_STRING_H)
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
#   if defined (__STDC__) || defined (__cplusplus)
yystrlen (const char *yystr)
#   else
yystrlen (yystr)
     const char *yystr;
#   endif
{
  register const char *yys = yystr;

  while (*yys++ != '\0')
    continue;

  return yys - yystr - 1;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined (__GLIBC__) && defined (_STRING_H) && defined (_GNU_SOURCE)
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
#   if defined (__STDC__) || defined (__cplusplus)
yystpcpy (char *yydest, const char *yysrc)
#   else
yystpcpy (yydest, yysrc)
     char *yydest;
     const char *yysrc;
#   endif
{
  register char *yyd = yydest;
  register const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif
#endif

#line 315 "/usr/share/bison/bison.simple"


/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
# if defined (__STDC__) || defined (__cplusplus)
#  define YYPARSE_PARAM_ARG void *YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL
# else
#  define YYPARSE_PARAM_ARG YYPARSE_PARAM
#  define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
# endif
#else /* !YYPARSE_PARAM */
# define YYPARSE_PARAM_ARG
# define YYPARSE_PARAM_DECL
#endif /* !YYPARSE_PARAM */

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
# ifdef YYPARSE_PARAM
int yyparse (void *);
# else
int yyparse (void);
# endif
#endif

/* YY_DECL_VARIABLES -- depending whether we use a pure parser,
   variables are global, or local to YYPARSE.  */

#define YY_DECL_NON_LSP_VARIABLES			\
/* The lookahead symbol.  */				\
int yychar;						\
							\
/* The semantic value of the lookahead symbol. */	\
YYSTYPE yylval;						\
							\
/* Number of parse errors so far.  */			\
int yynerrs;

#if YYLSP_NEEDED
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES			\
						\
/* Location data for the lookahead symbol.  */	\
YYLTYPE yylloc;
#else
# define YY_DECL_VARIABLES			\
YY_DECL_NON_LSP_VARIABLES
#endif


/* If nonreentrant, generate the variables here. */

#if !YYPURE
YY_DECL_VARIABLES
#endif  /* !YYPURE */

int
yyparse (YYPARSE_PARAM_ARG)
     YYPARSE_PARAM_DECL
{
  /* If reentrant, generate the variables here. */
#if YYPURE
  YY_DECL_VARIABLES
#endif  /* !YYPURE */

  register int yystate;
  register int yyn;
  int yyresult;
  /* Number of tokens to shift before error messages enabled.  */
  int yyerrstatus;
  /* Lookahead token as an internal (translated) token number.  */
  int yychar1 = 0;

  /* Three stacks and their tools:
     `yyss': related to states,
     `yyvs': related to semantic values,
     `yyls': related to locations.

     Refer to the stacks thru separate pointers, to allow yyoverflow
     to reallocate them elsewhere.  */

  /* The state stack. */
  short	yyssa[YYINITDEPTH];
  short *yyss = yyssa;
  register short *yyssp;

  /* The semantic value stack.  */
  YYSTYPE yyvsa[YYINITDEPTH];
  YYSTYPE *yyvs = yyvsa;
  register YYSTYPE *yyvsp;

#if YYLSP_NEEDED
  /* The location stack.  */
  YYLTYPE yylsa[YYINITDEPTH];
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;
#endif

#if YYLSP_NEEDED
# define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
# define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  YYSIZE_T yystacksize = YYINITDEPTH;


  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
#if YYLSP_NEEDED
  YYLTYPE yyloc;
#endif

  /* When reducing, the number of symbols on the RHS of the reduced
     rule. */
  int yylen;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss;
  yyvsp = yyvs;
#if YYLSP_NEEDED
  yylsp = yyls;
#endif
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed. so pushing a state here evens the stacks.
     */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack. Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	short *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  */
# if YYLSP_NEEDED
	YYLTYPE *yyls1 = yyls;
	/* This used to be a conditional around just the two extra args,
	   but that might be undefined if yyoverflow is a macro.  */
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yyls1, yysize * sizeof (*yylsp),
		    &yystacksize);
	yyls = yyls1;
# else
	yyoverflow ("parser stack overflow",
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);
# endif
	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyoverflowlab;
# else
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	goto yyoverflowlab;
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;

      {
	short *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyoverflowlab;
	YYSTACK_RELOCATE (yyss);
	YYSTACK_RELOCATE (yyvs);
# if YYLSP_NEEDED
	YYSTACK_RELOCATE (yyls);
# endif
# undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
#if YYLSP_NEEDED
      yylsp = yyls + yysize - 1;
#endif

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yychar1 = YYTRANSLATE (yychar);

#if YYDEBUG
     /* We have to keep this `#if YYDEBUG', since we use variables
	which are defined only if `YYDEBUG' is set.  */
      if (yydebug)
	{
	  YYFPRINTF (stderr, "Next token is %d (%s",
		     yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise
	     meaning of a token, for further debugging info.  */
# ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
# endif
	  YYFPRINTF (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */
  YYDPRINTF ((stderr, "Shifting token %d (%s), ",
	      yychar, yytname[yychar1]));

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  yystate = yyn;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to the semantic value of
     the lookahead token.  This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];

#if YYLSP_NEEDED
  /* Similarly for the default location.  Let the user run additional
     commands if for instance locations are ranges.  */
  yyloc = yylsp[1-yylen];
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
#endif

#if YYDEBUG
  /* We have to keep this `#if YYDEBUG', since we use variables which
     are defined only if `YYDEBUG' is set.  */
  if (yydebug)
    {
      int yyi;

      YYFPRINTF (stderr, "Reducing via rule %d (line %d), ",
		 yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (yyi = yyprhs[yyn]; yyrhs[yyi] > 0; yyi++)
	YYFPRINTF (stderr, "%s ", yytname[yyrhs[yyi]]);
      YYFPRINTF (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif

  switch (yyn) {

case 1:
#line 153 "../libpcap/grammar.y"
{
	finish_parse(yyvsp[0].blk.b);
}
    break;
case 3:
#line 158 "../libpcap/grammar.y"
{ yyval.blk.q = qerr; }
    break;
case 5:
#line 161 "../libpcap/grammar.y"
{ gen_and(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;
case 6:
#line 162 "../libpcap/grammar.y"
{ gen_and(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;
case 7:
#line 163 "../libpcap/grammar.y"
{ gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;
case 8:
#line 164 "../libpcap/grammar.y"
{ gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;
case 9:
#line 166 "../libpcap/grammar.y"
{ yyval.blk = yyvsp[-1].blk; }
    break;
case 10:
#line 168 "../libpcap/grammar.y"
{ yyval.blk = yyvsp[-1].blk; }
    break;
case 12:
#line 171 "../libpcap/grammar.y"
{ yyval.blk.b = gen_ncode(NULL, (bpf_u_int32)yyvsp[0].i,
						   yyval.blk.q = yyvsp[-1].blk.q); }
    break;
case 13:
#line 173 "../libpcap/grammar.y"
{ yyval.blk = yyvsp[-1].blk; }
    break;
case 14:
#line 175 "../libpcap/grammar.y"
{ yyval.blk.b = gen_scode(yyvsp[0].s, yyval.blk.q = yyvsp[-1].blk.q); }
    break;
case 15:
#line 176 "../libpcap/grammar.y"
{ yyval.blk.b = gen_mcode(yyvsp[-2].s, NULL, yyvsp[0].i,
				    yyval.blk.q = yyvsp[-3].blk.q); }
    break;
case 16:
#line 178 "../libpcap/grammar.y"
{ yyval.blk.b = gen_mcode(yyvsp[-2].s, yyvsp[0].s, 0,
				    yyval.blk.q = yyvsp[-3].blk.q); }
    break;
case 17:
#line 180 "../libpcap/grammar.y"
{
				  /* Decide how to parse HID based on proto */
				  yyval.blk.q = yyvsp[-1].blk.q;
				  yyval.blk.b = gen_ncode(yyvsp[0].s, 0, yyval.blk.q);
				}
    break;
case 18:
#line 185 "../libpcap/grammar.y"
{
#ifdef INET6
				  yyval.blk.b = gen_mcode6(yyvsp[-2].s, NULL, yyvsp[0].i,
				    yyval.blk.q = yyvsp[-3].blk.q);
#else
				  bpf_error("'ip6addr/prefixlen' not supported "
					"in this configuration");
#endif /*INET6*/
				}
    break;
case 19:
#line 194 "../libpcap/grammar.y"
{
#ifdef INET6
				  yyval.blk.b = gen_mcode6(yyvsp[0].s, 0, 128,
				    yyval.blk.q = yyvsp[-1].blk.q);
#else
				  bpf_error("'ip6addr' not supported "
					"in this configuration");
#endif /*INET6*/
				}
    break;
case 20:
#line 203 "../libpcap/grammar.y"
{ 
				  yyval.blk.b = gen_ecode(yyvsp[0].e, yyval.blk.q = yyvsp[-1].blk.q);
				  /*
				   * $1 was allocated by "pcap_ether_aton()",
				   * so we must free it now that we're done
				   * with it.
				   */
				  free(yyvsp[0].e);
				}
    break;
case 21:
#line 212 "../libpcap/grammar.y"
{
				  yyval.blk.b = gen_acode(yyvsp[0].e, yyval.blk.q = yyvsp[-1].blk.q);
				  /*
				   * $1 was allocated by "pcap_ether_aton()",
				   * so we must free it now that we're done
				   * with it.
				   */
				  free(yyvsp[0].e);
				}
    break;
case 22:
#line 221 "../libpcap/grammar.y"
{ gen_not(yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;
case 23:
#line 223 "../libpcap/grammar.y"
{ yyval.blk = yyvsp[-1].blk; }
    break;
case 24:
#line 225 "../libpcap/grammar.y"
{ yyval.blk = yyvsp[-1].blk; }
    break;
case 26:
#line 228 "../libpcap/grammar.y"
{ gen_and(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;
case 27:
#line 229 "../libpcap/grammar.y"
{ gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;
case 28:
#line 231 "../libpcap/grammar.y"
{ yyval.blk.b = gen_ncode(NULL, (bpf_u_int32)yyvsp[0].i,
						   yyval.blk.q = yyvsp[-1].blk.q); }
    break;
case 31:
#line 236 "../libpcap/grammar.y"
{ gen_not(yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;
case 32:
#line 238 "../libpcap/grammar.y"
{ QSET(yyval.blk.q, yyvsp[-2].i, yyvsp[-1].i, yyvsp[0].i); }
    break;
case 33:
#line 239 "../libpcap/grammar.y"
{ QSET(yyval.blk.q, yyvsp[-1].i, yyvsp[0].i, Q_DEFAULT); }
    break;
case 34:
#line 240 "../libpcap/grammar.y"
{ QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, yyvsp[0].i); }
    break;
case 35:
#line 241 "../libpcap/grammar.y"
{ QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, Q_PROTO); }
    break;
case 36:
#line 242 "../libpcap/grammar.y"
{ QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, Q_PROTOCHAIN); }
    break;
case 37:
#line 243 "../libpcap/grammar.y"
{ QSET(yyval.blk.q, yyvsp[-1].i, Q_DEFAULT, yyvsp[0].i); }
    break;
case 38:
#line 245 "../libpcap/grammar.y"
{ yyval.blk = yyvsp[0].blk; }
    break;
case 39:
#line 246 "../libpcap/grammar.y"
{ yyval.blk.b = yyvsp[-1].blk.b; yyval.blk.q = yyvsp[-2].blk.q; }
    break;
case 40:
#line 247 "../libpcap/grammar.y"
{ yyval.blk.b = gen_proto_abbrev(yyvsp[0].i); yyval.blk.q = qerr; }
    break;
case 41:
#line 248 "../libpcap/grammar.y"
{ yyval.blk.b = gen_relation(yyvsp[-1].i, yyvsp[-2].a, yyvsp[0].a, 0);
				  yyval.blk.q = qerr; }
    break;
case 42:
#line 250 "../libpcap/grammar.y"
{ yyval.blk.b = gen_relation(yyvsp[-1].i, yyvsp[-2].a, yyvsp[0].a, 1);
				  yyval.blk.q = qerr; }
    break;
case 43:
#line 252 "../libpcap/grammar.y"
{ yyval.blk.b = yyvsp[0].rblk; yyval.blk.q = qerr; }
    break;
case 44:
#line 253 "../libpcap/grammar.y"
{ yyval.blk.b = gen_atmtype_abbrev(yyvsp[0].i); yyval.blk.q = qerr; }
    break;
case 45:
#line 254 "../libpcap/grammar.y"
{ yyval.blk.b = gen_atmmulti_abbrev(yyvsp[0].i); yyval.blk.q = qerr; }
    break;
case 46:
#line 255 "../libpcap/grammar.y"
{ yyval.blk.b = yyvsp[0].blk.b; yyval.blk.q = qerr; }
    break;
case 48:
#line 259 "../libpcap/grammar.y"
{ yyval.i = Q_DEFAULT; }
    break;
case 49:
#line 262 "../libpcap/grammar.y"
{ yyval.i = Q_SRC; }
    break;
case 50:
#line 263 "../libpcap/grammar.y"
{ yyval.i = Q_DST; }
    break;
case 51:
#line 264 "../libpcap/grammar.y"
{ yyval.i = Q_OR; }
    break;
case 52:
#line 265 "../libpcap/grammar.y"
{ yyval.i = Q_OR; }
    break;
case 53:
#line 266 "../libpcap/grammar.y"
{ yyval.i = Q_AND; }
    break;
case 54:
#line 267 "../libpcap/grammar.y"
{ yyval.i = Q_AND; }
    break;
case 55:
#line 270 "../libpcap/grammar.y"
{ yyval.i = Q_HOST; }
    break;
case 56:
#line 271 "../libpcap/grammar.y"
{ yyval.i = Q_NET; }
    break;
case 57:
#line 272 "../libpcap/grammar.y"
{ yyval.i = Q_PORT; }
    break;
case 58:
#line 275 "../libpcap/grammar.y"
{ yyval.i = Q_GATEWAY; }
    break;
case 59:
#line 277 "../libpcap/grammar.y"
{ yyval.i = Q_LINK; }
    break;
case 60:
#line 278 "../libpcap/grammar.y"
{ yyval.i = Q_IP; }
    break;
case 61:
#line 279 "../libpcap/grammar.y"
{ yyval.i = Q_ARP; }
    break;
case 62:
#line 280 "../libpcap/grammar.y"
{ yyval.i = Q_RARP; }
    break;
case 63:
#line 281 "../libpcap/grammar.y"
{ yyval.i = Q_SCTP; }
    break;
case 64:
#line 282 "../libpcap/grammar.y"
{ yyval.i = Q_TCP; }
    break;
case 65:
#line 283 "../libpcap/grammar.y"
{ yyval.i = Q_UDP; }
    break;
case 66:
#line 284 "../libpcap/grammar.y"
{ yyval.i = Q_ICMP; }
    break;
case 67:
#line 285 "../libpcap/grammar.y"
{ yyval.i = Q_IGMP; }
    break;
case 68:
#line 286 "../libpcap/grammar.y"
{ yyval.i = Q_IGRP; }
    break;
case 69:
#line 287 "../libpcap/grammar.y"
{ yyval.i = Q_PIM; }
    break;
case 70:
#line 288 "../libpcap/grammar.y"
{ yyval.i = Q_VRRP; }
    break;
case 71:
#line 289 "../libpcap/grammar.y"
{ yyval.i = Q_ATALK; }
    break;
case 72:
#line 290 "../libpcap/grammar.y"
{ yyval.i = Q_AARP; }
    break;
case 73:
#line 291 "../libpcap/grammar.y"
{ yyval.i = Q_DECNET; }
    break;
case 74:
#line 292 "../libpcap/grammar.y"
{ yyval.i = Q_LAT; }
    break;
case 75:
#line 293 "../libpcap/grammar.y"
{ yyval.i = Q_SCA; }
    break;
case 76:
#line 294 "../libpcap/grammar.y"
{ yyval.i = Q_MOPDL; }
    break;
case 77:
#line 295 "../libpcap/grammar.y"
{ yyval.i = Q_MOPRC; }
    break;
case 78:
#line 296 "../libpcap/grammar.y"
{ yyval.i = Q_IPV6; }
    break;
case 79:
#line 297 "../libpcap/grammar.y"
{ yyval.i = Q_ICMPV6; }
    break;
case 80:
#line 298 "../libpcap/grammar.y"
{ yyval.i = Q_AH; }
    break;
case 81:
#line 299 "../libpcap/grammar.y"
{ yyval.i = Q_ESP; }
    break;
case 82:
#line 300 "../libpcap/grammar.y"
{ yyval.i = Q_ISO; }
    break;
case 83:
#line 301 "../libpcap/grammar.y"
{ yyval.i = Q_ESIS; }
    break;
case 84:
#line 302 "../libpcap/grammar.y"
{ yyval.i = Q_ISIS; }
    break;
case 85:
#line 303 "../libpcap/grammar.y"
{ yyval.i = Q_ISIS_L1; }
    break;
case 86:
#line 304 "../libpcap/grammar.y"
{ yyval.i = Q_ISIS_L2; }
    break;
case 87:
#line 305 "../libpcap/grammar.y"
{ yyval.i = Q_ISIS_IIH; }
    break;
case 88:
#line 306 "../libpcap/grammar.y"
{ yyval.i = Q_ISIS_LSP; }
    break;
case 89:
#line 307 "../libpcap/grammar.y"
{ yyval.i = Q_ISIS_SNP; }
    break;
case 90:
#line 308 "../libpcap/grammar.y"
{ yyval.i = Q_ISIS_PSNP; }
    break;
case 91:
#line 309 "../libpcap/grammar.y"
{ yyval.i = Q_ISIS_CSNP; }
    break;
case 92:
#line 310 "../libpcap/grammar.y"
{ yyval.i = Q_CLNP; }
    break;
case 93:
#line 311 "../libpcap/grammar.y"
{ yyval.i = Q_STP; }
    break;
case 94:
#line 312 "../libpcap/grammar.y"
{ yyval.i = Q_IPX; }
    break;
case 95:
#line 313 "../libpcap/grammar.y"
{ yyval.i = Q_NETBEUI; }
    break;
case 96:
#line 315 "../libpcap/grammar.y"
{ yyval.rblk = gen_broadcast(yyvsp[-1].i); }
    break;
case 97:
#line 316 "../libpcap/grammar.y"
{ yyval.rblk = gen_multicast(yyvsp[-1].i); }
    break;
case 98:
#line 317 "../libpcap/grammar.y"
{ yyval.rblk = gen_less(yyvsp[0].i); }
    break;
case 99:
#line 318 "../libpcap/grammar.y"
{ yyval.rblk = gen_greater(yyvsp[0].i); }
    break;
case 100:
#line 319 "../libpcap/grammar.y"
{ yyval.rblk = gen_byteop(yyvsp[-1].i, yyvsp[-2].i, yyvsp[0].i); }
    break;
case 101:
#line 320 "../libpcap/grammar.y"
{ yyval.rblk = gen_inbound(0); }
    break;
case 102:
#line 321 "../libpcap/grammar.y"
{ yyval.rblk = gen_inbound(1); }
    break;
case 103:
#line 322 "../libpcap/grammar.y"
{ yyval.rblk = gen_vlan(yyvsp[0].i); }
    break;
case 104:
#line 323 "../libpcap/grammar.y"
{ yyval.rblk = gen_vlan(-1); }
    break;
case 105:
#line 325 "../libpcap/grammar.y"
{ yyval.i = BPF_JGT; }
    break;
case 106:
#line 326 "../libpcap/grammar.y"
{ yyval.i = BPF_JGE; }
    break;
case 107:
#line 327 "../libpcap/grammar.y"
{ yyval.i = BPF_JEQ; }
    break;
case 108:
#line 329 "../libpcap/grammar.y"
{ yyval.i = BPF_JGT; }
    break;
case 109:
#line 330 "../libpcap/grammar.y"
{ yyval.i = BPF_JGE; }
    break;
case 110:
#line 331 "../libpcap/grammar.y"
{ yyval.i = BPF_JEQ; }
    break;
case 111:
#line 333 "../libpcap/grammar.y"
{ yyval.a = gen_loadi(yyvsp[0].i); }
    break;
case 113:
#line 336 "../libpcap/grammar.y"
{ yyval.a = gen_load(yyvsp[-3].i, yyvsp[-1].a, 1); }
    break;
case 114:
#line 337 "../libpcap/grammar.y"
{ yyval.a = gen_load(yyvsp[-5].i, yyvsp[-3].a, yyvsp[-1].i); }
    break;
case 115:
#line 338 "../libpcap/grammar.y"
{ yyval.a = gen_arth(BPF_ADD, yyvsp[-2].a, yyvsp[0].a); }
    break;
case 116:
#line 339 "../libpcap/grammar.y"
{ yyval.a = gen_arth(BPF_SUB, yyvsp[-2].a, yyvsp[0].a); }
    break;
case 117:
#line 340 "../libpcap/grammar.y"
{ yyval.a = gen_arth(BPF_MUL, yyvsp[-2].a, yyvsp[0].a); }
    break;
case 118:
#line 341 "../libpcap/grammar.y"
{ yyval.a = gen_arth(BPF_DIV, yyvsp[-2].a, yyvsp[0].a); }
    break;
case 119:
#line 342 "../libpcap/grammar.y"
{ yyval.a = gen_arth(BPF_AND, yyvsp[-2].a, yyvsp[0].a); }
    break;
case 120:
#line 343 "../libpcap/grammar.y"
{ yyval.a = gen_arth(BPF_OR, yyvsp[-2].a, yyvsp[0].a); }
    break;
case 121:
#line 344 "../libpcap/grammar.y"
{ yyval.a = gen_arth(BPF_LSH, yyvsp[-2].a, yyvsp[0].a); }
    break;
case 122:
#line 345 "../libpcap/grammar.y"
{ yyval.a = gen_arth(BPF_RSH, yyvsp[-2].a, yyvsp[0].a); }
    break;
case 123:
#line 346 "../libpcap/grammar.y"
{ yyval.a = gen_neg(yyvsp[0].a); }
    break;
case 124:
#line 347 "../libpcap/grammar.y"
{ yyval.a = yyvsp[-1].a; }
    break;
case 125:
#line 348 "../libpcap/grammar.y"
{ yyval.a = gen_loadlen(); }
    break;
case 126:
#line 350 "../libpcap/grammar.y"
{ yyval.i = '&'; }
    break;
case 127:
#line 351 "../libpcap/grammar.y"
{ yyval.i = '|'; }
    break;
case 128:
#line 352 "../libpcap/grammar.y"
{ yyval.i = '<'; }
    break;
case 129:
#line 353 "../libpcap/grammar.y"
{ yyval.i = '>'; }
    break;
case 130:
#line 354 "../libpcap/grammar.y"
{ yyval.i = '='; }
    break;
case 132:
#line 357 "../libpcap/grammar.y"
{ yyval.i = yyvsp[-1].i; }
    break;
case 133:
#line 359 "../libpcap/grammar.y"
{ yyval.i = A_LANE; }
    break;
case 134:
#line 360 "../libpcap/grammar.y"
{ yyval.i = A_LLC; }
    break;
case 135:
#line 361 "../libpcap/grammar.y"
{ yyval.i = A_METAC;	}
    break;
case 136:
#line 362 "../libpcap/grammar.y"
{ yyval.i = A_BCC; }
    break;
case 137:
#line 363 "../libpcap/grammar.y"
{ yyval.i = A_OAMF4EC; }
    break;
case 138:
#line 364 "../libpcap/grammar.y"
{ yyval.i = A_OAMF4SC; }
    break;
case 139:
#line 365 "../libpcap/grammar.y"
{ yyval.i = A_SC; }
    break;
case 140:
#line 366 "../libpcap/grammar.y"
{ yyval.i = A_ILMIC; }
    break;
case 141:
#line 368 "../libpcap/grammar.y"
{ yyval.i = A_OAM; }
    break;
case 142:
#line 369 "../libpcap/grammar.y"
{ yyval.i = A_OAMF4; }
    break;
case 143:
#line 370 "../libpcap/grammar.y"
{ yyval.i = A_CONNECTMSG; }
    break;
case 144:
#line 371 "../libpcap/grammar.y"
{ yyval.i = A_METACONNECT; }
    break;
case 145:
#line 374 "../libpcap/grammar.y"
{ yyval.blk.atmfieldtype = A_VPI; }
    break;
case 146:
#line 375 "../libpcap/grammar.y"
{ yyval.blk.atmfieldtype = A_VCI; }
    break;
case 148:
#line 378 "../libpcap/grammar.y"
{ yyval.blk.b = gen_atmfield_code(yyvsp[-2].blk.atmfieldtype, (u_int)yyvsp[0].i, (u_int)yyvsp[-1].i, 0); }
    break;
case 149:
#line 379 "../libpcap/grammar.y"
{ yyval.blk.b = gen_atmfield_code(yyvsp[-2].blk.atmfieldtype, (u_int)yyvsp[0].i, (u_int)yyvsp[-1].i, 1); }
    break;
case 150:
#line 380 "../libpcap/grammar.y"
{ yyval.blk.b = yyvsp[-1].blk.b; yyval.blk.q = qerr; }
    break;
case 151:
#line 382 "../libpcap/grammar.y"
{
	yyval.blk.atmfieldtype = yyvsp[-1].blk.atmfieldtype;
	if (yyval.blk.atmfieldtype == A_VPI ||
	    yyval.blk.atmfieldtype == A_VCI)
		yyval.blk.b = gen_atmfield_code(yyval.blk.atmfieldtype, (u_int) yyvsp[0].i, BPF_JEQ, 0);
	}
    break;
case 153:
#line 390 "../libpcap/grammar.y"
{ gen_or(yyvsp[-2].blk.b, yyvsp[0].blk.b); yyval.blk = yyvsp[0].blk; }
    break;
}

#line 705 "/usr/share/bison/bison.simple"


  yyvsp -= yylen;
  yyssp -= yylen;
#if YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;
#if YYLSP_NEEDED
  *++yylsp = yyloc;
#endif

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  YYSIZE_T yysize = 0;
	  char *yymsg;
	  int yyx, yycount;

	  yycount = 0;
	  /* Start YYX at -YYN if negative to avoid negative indexes in
	     YYCHECK.  */
	  for (yyx = yyn < 0 ? -yyn : 0;
	       yyx < (int) (sizeof (yytname) / sizeof (char *)); yyx++)
	    if (yycheck[yyx + yyn] == yyx)
	      yysize += yystrlen (yytname[yyx]) + 15, yycount++;
	  yysize += yystrlen ("parse error, unexpected ") + 1;
	  yysize += yystrlen (yytname[YYTRANSLATE (yychar)]);
	  yymsg = (char *) YYSTACK_ALLOC (yysize);
	  if (yymsg != 0)
	    {
	      char *yyp = yystpcpy (yymsg, "parse error, unexpected ");
	      yyp = yystpcpy (yyp, yytname[YYTRANSLATE (yychar)]);

	      if (yycount < 5)
		{
		  yycount = 0;
		  for (yyx = yyn < 0 ? -yyn : 0;
		       yyx < (int) (sizeof (yytname) / sizeof (char *));
		       yyx++)
		    if (yycheck[yyx + yyn] == yyx)
		      {
			const char *yyq = ! yycount ? ", expecting " : " or ";
			yyp = yystpcpy (yyp, yyq);
			yyp = yystpcpy (yyp, yytname[yyx]);
			yycount++;
		      }
		}
	      yyerror (yymsg);
	      YYSTACK_FREE (yymsg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exhausted");
	}
      else
#endif /* defined (YYERROR_VERBOSE) */
	yyerror ("parse error");
    }
  goto yyerrlab1;


/*--------------------------------------------------.
| yyerrlab1 -- error raised explicitly by an action |
`--------------------------------------------------*/
yyerrlab1:
  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;
      YYDPRINTF ((stderr, "Discarding token %d (%s).\n",
		  yychar, yytname[yychar1]));
      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;


/*-------------------------------------------------------------------.
| yyerrdefault -- current state does not do anything special for the |
| error token.                                                       |
`-------------------------------------------------------------------*/
yyerrdefault:
#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */

  /* If its default is to accept any token, ok.  Otherwise pop it.  */
  yyn = yydefact[yystate];
  if (yyn)
    goto yydefault;
#endif


/*---------------------------------------------------------------.
| yyerrpop -- pop the current state because it cannot handle the |
| error token                                                    |
`---------------------------------------------------------------*/
yyerrpop:
  if (yyssp == yyss)
    YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#if YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG
  if (yydebug)
    {
      short *yyssp1 = yyss - 1;
      YYFPRINTF (stderr, "Error: state stack now");
      while (yyssp1 != yyssp)
	YYFPRINTF (stderr, " %d", *++yyssp1);
      YYFPRINTF (stderr, "\n");
    }
#endif

/*--------------.
| yyerrhandle.  |
`--------------*/
yyerrhandle:
  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

  YYDPRINTF ((stderr, "Shifting error token, "));

  *++yyvsp = yylval;
#if YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

/*---------------------------------------------.
| yyoverflowab -- parser overflow comes here.  |
`---------------------------------------------*/
yyoverflowlab:
  yyerror ("parser stack overflow");
  yyresult = 2;
  /* Fall through.  */

yyreturn:
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
  return yyresult;
}
#line 392 "../libpcap/grammar.y"

