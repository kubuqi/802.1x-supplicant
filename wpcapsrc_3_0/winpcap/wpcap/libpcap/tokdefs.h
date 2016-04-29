#ifndef BISON_Y_TAB_H
# define BISON_Y_TAB_H

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


extern YYSTYPE pcap_lval;

#endif /* not BISON_Y_TAB_H */
