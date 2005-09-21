/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 2002 by the Sylpheed Claws Team and Hiroyuki Yamamoto
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

#ifndef REMOTEFOLDER_H
#define REMOTEFOLDER_H 1

typedef struct _RemoteFolder	RemoteFolder;

#define REMOTE_FOLDER(obj)	((RemoteFolder *)obj)

#include <glib.h>

#include "folder.h"
#include "session.h"

struct _RemoteFolder
{
	Folder folder;

	Session *session;
};

void        folder_remote_folder_init	(Folder		*folder,
					 const gchar	*name,
					 const gchar	*path);
void        folder_remote_folder_destroy(RemoteFolder	*rfolder);

#endif /* REMOTEFOLDER_H */
