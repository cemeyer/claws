/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 2001-2003 Match Grun
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/*
 * Definitions necessary to access LDIF files (LDAP Data Interchange Format
 * files). These files are used to load LDAP servers and to interchange data
 * between servers. They are also used by several E-Mail client programs and
 * other programs as a means of interchange address book data.
 */

#ifndef __LDIF_H__
#define __LDIF_H__

#include <stdio.h>
#include <glib.h>

#include "addrcache.h"

#define LDIFBUFSIZE         2048

/* Common tag names - for address book import/export */
#define	LDIF_TAG_DN            "dn"
#define	LDIF_TAG_COMMONNAME    "cn"
#define	LDIF_TAG_FIRSTNAME     "givenname"
#define	LDIF_TAG_LASTNAME      "sn"
#define LDIF_TAG_NICKNAME      "xmozillanickname"
#define	LDIF_TAG_EMAIL         "mail"
#define	LDIF_TAG_OBJECTCLASS   "objectclass"

/* Object classes */
#define LDIF_CLASS_PERSON      "person"
#define LDIF_CLASS_INET_PERSON "inetOrgPerson"

/*
* Typical LDIF entry (similar to that generated by Netscape):
*
* dn: uid=axel, dc=axel, dc=com
* cn: Axel Rose
* sn: Rose
* givenname: Arnold
* xmozillanickname: Axel
* mail: axel@axelrose.com
* mail: axelrose@aol.com
* mail: axel@netscape.net
* mail: axel@hotmail.com
* uid: axelrose
* o: The Company
* locality: Denver
* st: CO
* streetaddress: 777 Lexington Avenue
* postalcode: 80298
* countryname: USA
* telephonenumber: 303-555-1234
* homephone: 303-555-2345
* cellphone: 303-555-3456
* homeurl: http://www.axelrose.com
* objectclass: top
* objectclass: person
*
* Note that first entry is always dn. An empty line defines end of
* record. Note that attribute names are case insensitive. There may
* also be several occurrences of an attribute, for example, as
* illustrated for "mail" and "objectclass" attributes. LDIF files
* can also use binary data using base-64 encoding.
*
*/

/* LDIF file object */
typedef struct _LdifFile LdifFile;
struct _LdifFile {
	FILE       *file;
	gchar      *path;
	gchar      *bufptr;
	gchar      buffer[ LDIFBUFSIZE ];
	gint       retVal;
	GHashTable *hashFields;
	GList      *tempList;
	gboolean   dirtyFlag;
	gboolean   accessFlag;
	void       (*cbProgress)( void *, void *, void * );
	gint       importCount;
};

/* LDIF field record structure */
typedef struct _Ldif_FieldRec_ Ldif_FieldRec;
struct _Ldif_FieldRec_ {
	gchar *tagName;
	gchar *userName;
	gboolean reserved;
	gboolean selected;
};

/* Function prototypes */
LdifFile *ldif_create		( void );
void ldif_set_file		( LdifFile* ldifFile, const gchar *value );
void ldif_set_accessed		( LdifFile* ldifFile, const gboolean value );
void ldif_set_callback		( LdifFile *ldifFile, void *func );
void ldif_field_set_name	( Ldif_FieldRec *rec, const gchar *value );
void ldif_field_set_selected	( Ldif_FieldRec *rec, const gboolean value );
void ldif_field_toggle		( Ldif_FieldRec *rec );
void ldif_free			( LdifFile *ldifFile );
void ldif_print_fieldrec	( Ldif_FieldRec *rec, FILE *stream );
void ldif_print_file		( LdifFile *ldifFile, FILE *stream );
gint ldif_import_data		( LdifFile *ldifFile, AddressCache *cache );
gint ldif_read_tags		( LdifFile *ldifFile );
GList *ldif_get_fieldlist	( LdifFile *ldifFile );
gboolean ldif_write_value	( FILE *stream, const gchar *name,
				  const gchar *value );
void ldif_write_eor		( FILE *stream );

#endif /* __LDIF_H__ */

