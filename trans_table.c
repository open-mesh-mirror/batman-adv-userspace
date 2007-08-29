/*
 * Copyright (C) 2007 B.A.T.M.A.N. contributors:
 * Simon Wunderlich
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 * trans_table.c - implements a MAC translation table
 *
 */

#include "trans_table.h"	/* our prototype */
#include "hash.h"			/* hash_* functions */
#include "originator.h"		/* compare_orig(), choose_orig() */
#include "os.h"				/* debug_output(), addr_to_string() */
#include <string.h>			/* memcpy() */
#include <stdlib.h>			/* malloc(), free() */

struct hashtable_t *trans_hash;
unsigned char bcast_addr[6]= { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
			  zero_addr[6]= {     0,    0,    0,    0,    0,    0};
/* initialize the translation table hash */
int transtable_init() {
	trans_hash= hash_new( 4096, compare_orig, choose_orig);
	return ( (trans_hash == NULL)?-1:0 );
}

int transtable_quit() {
	hash_delete(trans_hash, free);
	return (0);
}

/* check if a payload MAC-address is already in the hash-table and add it if it's new */
int transtable_add( unsigned char *mac, unsigned char *batman_mac) {
	struct trans_element_t *elem;
	/* ignore broadcast, multicast and zero */
	if ( 0==memcmp( mac, bcast_addr, 6)) return(-1);
	if ( 0==memcmp( mac, zero_addr, 6)) return(-1);
	if (( mac[0] == 0x01 ) && (mac[1] == 0x00) && (mac[2] == 0x5e) && (!(mac[3] & 0x80))) return(-1);

	elem= hash_find(trans_hash, mac);	/* first check, then add saves us a malloc() in 
										 * most of the times where no mac is to add,
										 * so this should be faster */
	if ( elem == NULL ) {
		elem= malloc(sizeof(struct trans_element_t));
		memcpy(elem->mac, mac, 6);
		memcpy(elem->batman_mac, batman_mac, 6);
		hash_add(trans_hash, elem);
		debug_output( 4, "Added MAC %s to originator %s\n",	addr_to_string(mac), addr_to_string(batman_mac));
		return(1);
	}
	return(0);
}

/* Returns the batman interface address which matches to the payload MAC-address, or NULL if it could not be found. */
unsigned char *transtable_search( unsigned char *mac) {
	struct trans_element_t *elem;

	elem= hash_find(trans_hash, mac);
	if ( elem != NULL)	{
/*		debug_output( 4, "Found MAC %s at %s\n",	addr_to_string(elem->mac), addr_to_string(elem->batman_mac)); */
		return( elem->batman_mac );
	}
	else 					return( NULL );

}

