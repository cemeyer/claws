/*
 * Sylpheed -- a GTK+ based, lightweight, and fast e-mail client
 * Copyright (C) 2001 Match Grun
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * Edit JPilot address book data.
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#ifdef USE_JPILOT

#include "defs.h"

#include <glib.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkwindow.h>
#include <gtk/gtksignal.h>
#include <gtk/gtkhbox.h>
#include <gtk/gtklabel.h>
#include <gtk/gtkentry.h>
#include <gtk/gtkhbbox.h>
#include <gtk/gtkbutton.h>

#include "intl.h"
#include "addressbook.h"
#include "prefs_common.h"
#include "addressitem.h"
#include "jpilot.h"

#define ADDRESSBOOK_GUESS_JPILOT "MyJPilot"
#define JPILOT_NUM_CUSTOM_LABEL	4

static struct _JPilotEdit {
	GtkWidget *window;
	GtkWidget *name_entry;
	GtkWidget *file_entry;
	GtkWidget *custom_check[JPILOT_NUM_CUSTOM_LABEL];
	GtkWidget *custom_label[JPILOT_NUM_CUSTOM_LABEL];
	GtkWidget *ok_btn;
	GtkWidget *cancel_btn;
	GtkWidget *statusbar;
	gint status_cid;
} jpilotedit;

static struct _AddressFileSelection jpilot_file_selector;

/*
* Edit functions.
*/
void edit_jpilot_status_show( gchar *msg ) {
	if( jpilotedit.statusbar != NULL ) {
		gtk_statusbar_pop( GTK_STATUSBAR(jpilotedit.statusbar), jpilotedit.status_cid );
		if( msg ) {
			gtk_statusbar_push( GTK_STATUSBAR(jpilotedit.statusbar), jpilotedit.status_cid, msg );
		}
	}
}

static gint edit_jpilot_delete_event( GtkWidget *widget, GdkEventAny *event, gboolean *cancelled ) {
	*cancelled = TRUE;
	gtk_main_quit();
	return TRUE;
}

static void edit_jpilot_key_pressed( GtkWidget *widget, GdkEventKey *event, gboolean *cancelled ) {
	if (event && event->keyval == GDK_Escape) {
		*cancelled = TRUE;
		gtk_main_quit();
	}
}

static void edit_jpilot_ok( GtkWidget *widget, gboolean *cancelled ) {
	*cancelled = FALSE;
	gtk_main_quit();
}

static void edit_jpilot_cancel( GtkWidget *widget, gboolean *cancelled ) {
	*cancelled = TRUE;
	gtk_main_quit();
}

static void edit_jpilot_fill_check_box( JPilotFile *jpf ) {
	gint i;
	GList *node, *customLbl = NULL;
	gchar *labelName;
	gboolean done, checked;
	for( i = 0; i < JPILOT_NUM_CUSTOM_LABEL; i++ ) {
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( jpilotedit.custom_check[i] ), FALSE );
		gtk_label_set_text( GTK_LABEL( jpilotedit.custom_label[i] ), "" );
	}

	done = FALSE;
	i = 0;
	customLbl = jpilot_load_custom_label( jpf, customLbl );
	node = customLbl;
	while( ! done ) {
		if( node ) {
			labelName = node->data;
			gtk_label_set_text( GTK_LABEL( jpilotedit.custom_label[i] ), labelName );
			checked = jpilot_test_custom_label( jpf, labelName );
			gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( jpilotedit.custom_check[i] ), checked );
			i++;
			if( i >= JPILOT_NUM_CUSTOM_LABEL ) done = TRUE;
			node = g_list_next( node );
		}
		else {
			done = TRUE;
		}
	}
	mgu_free_dlist( customLbl );
	customLbl = NULL;
}

static void edit_jpilot_fill_check_box_new() {
	gint i;
	for( i = 0; i < JPILOT_NUM_CUSTOM_LABEL; i++ ) {
		gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( jpilotedit.custom_check[i] ), FALSE );
		gtk_label_set_text( GTK_LABEL( jpilotedit.custom_label[i] ), "" );
	}
}

static void edit_jpilot_read_check_box( JPilotFile *pilotFile ) {
	gint i;
	gchar *labelName;
	jpilot_clear_custom_labels( pilotFile );
	for( i = 0; i < JPILOT_NUM_CUSTOM_LABEL; i++ ) {
		if( gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(jpilotedit.custom_check[i]) ) ) {
			labelName = g_strdup( GTK_LABEL(jpilotedit.custom_label[i])->label );
			jpilot_add_custom_label( pilotFile, labelName );
		}
	}
}

static void edit_jpilot_file_check( void ) {
	gint t;
	gchar *sFile;
	gchar *sMsg;
	gboolean flg;

	flg = FALSE;
	sFile = gtk_editable_get_chars( GTK_EDITABLE(jpilotedit.file_entry), 0, -1 );
	if( sFile ) {
		g_strchomp( sFile ); g_strchug( sFile );
		if( *sFile != '\0' ) {
			// Attempt to read file
			JPilotFile *jpf = jpilot_create_path( sFile );
			t = jpilot_read_data( jpf );
			if( t == MGU_SUCCESS ) {
				// Set check boxes
				edit_jpilot_fill_check_box( jpf );
				flg = TRUE;
			}
			jpilot_free( jpf );
		}
	}
	if( ! flg ) {
		// Clear all check boxes
		edit_jpilot_fill_check_box_new();
	}
	g_free( sFile );

	// Display appropriate message
	if( t == MGU_SUCCESS ) {
		sMsg = "";
	}
	else if( t == MGU_BAD_FORMAT || t == MGU_OO_MEMORY ) {
		sMsg = _("File does not appear to be JPilot format.");
	}
	else {
		sMsg = _("Could not read file.");
	}
	edit_jpilot_status_show( sMsg );
}

static void edit_jpilot_file_ok( GtkWidget *widget, gpointer data ) {
	gchar *sFile;
	AddressFileSelection *afs;
	GtkWidget *fileSel;

	afs = ( AddressFileSelection * ) data;
	fileSel = afs->fileSelector;
	sFile = gtk_file_selection_get_filename( GTK_FILE_SELECTION(fileSel) );

	afs->cancelled = FALSE;
	gtk_entry_set_text( GTK_ENTRY(jpilotedit.file_entry), sFile );
	gtk_widget_hide( afs->fileSelector );
	gtk_grab_remove( afs->fileSelector );
	edit_jpilot_file_check();
	gtk_widget_grab_focus( jpilotedit.file_entry );
}

static void edit_jpilot_file_cancel( GtkWidget *widget, gpointer data ) {
	AddressFileSelection *afs = ( AddressFileSelection * ) data;
	afs->cancelled = TRUE;
	gtk_widget_hide( afs->fileSelector );
	gtk_grab_remove( afs->fileSelector );
	gtk_widget_grab_focus( jpilotedit.file_entry );
}

static void edit_jpilot_file_select_create( AddressFileSelection *afs ) {
	GtkWidget *fileSelector;

	fileSelector = gtk_file_selection_new( _("Select JPilot File") );
	gtk_file_selection_hide_fileop_buttons( GTK_FILE_SELECTION(fileSelector) );
	gtk_signal_connect( GTK_OBJECT (GTK_FILE_SELECTION(fileSelector)->ok_button),
                             "clicked", GTK_SIGNAL_FUNC (edit_jpilot_file_ok), ( gpointer ) afs );
	gtk_signal_connect( GTK_OBJECT (GTK_FILE_SELECTION(fileSelector)->cancel_button),
                             "clicked", GTK_SIGNAL_FUNC (edit_jpilot_file_cancel), ( gpointer ) afs );
	afs->fileSelector = fileSelector;
	afs->cancelled = TRUE;
}

static void edit_jpilot_file_select( void ) {
	gchar *sFile;

	if (! jpilot_file_selector.fileSelector )
		edit_jpilot_file_select_create( & jpilot_file_selector );

	sFile = gtk_editable_get_chars( GTK_EDITABLE(jpilotedit.file_entry), 0, -1 );
	gtk_file_selection_set_filename( GTK_FILE_SELECTION( jpilot_file_selector.fileSelector ), sFile );
	g_free( sFile );
	gtk_widget_show( jpilot_file_selector.fileSelector );
	gtk_grab_add( jpilot_file_selector.fileSelector );
}

static void addressbook_edit_jpilot_create( gboolean *cancelled ) {
	GtkWidget *window;
	GtkWidget *vbox;
	GtkWidget *table;
	GtkWidget *label;
	GtkWidget *name_entry;
	GtkWidget *file_entry;
	GtkWidget *vbox_custom;
	GtkWidget *frame_custom;
	GtkWidget *custom_check[JPILOT_NUM_CUSTOM_LABEL];
	GtkWidget *custom_label[JPILOT_NUM_CUSTOM_LABEL];
	GtkWidget *hlbox;
	GtkWidget *hbbox;
	GtkWidget *hsep;
	GtkWidget *ok_btn;
	GtkWidget *cancel_btn;
	GtkWidget *check_btn;
	GtkWidget *file_btn;
	GtkWidget *hsbox;
	GtkWidget *statusbar;
	gint top, i;

	window = gtk_window_new(GTK_WINDOW_DIALOG);
	gtk_widget_set_usize(window, 450, -1);
	gtk_container_set_border_width(GTK_CONTAINER(window), 0);
	gtk_window_set_title(GTK_WINDOW(window), _("Edit JPilot Entry"));
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_modal(GTK_WINDOW(window), TRUE);	
	gtk_signal_connect(GTK_OBJECT(window), "delete_event",
			   GTK_SIGNAL_FUNC(edit_jpilot_delete_event),
			   cancelled);
	gtk_signal_connect(GTK_OBJECT(window), "key_press_event",
			   GTK_SIGNAL_FUNC(edit_jpilot_key_pressed),
			   cancelled);

	vbox = gtk_vbox_new(FALSE, 8);
	gtk_container_add(GTK_CONTAINER(window), vbox);
	gtk_container_set_border_width( GTK_CONTAINER(vbox), 0 );

	table = gtk_table_new(2 + JPILOT_NUM_CUSTOM_LABEL, 3, FALSE);
	gtk_box_pack_start(GTK_BOX(vbox), table, FALSE, FALSE, 0);
	gtk_container_set_border_width( GTK_CONTAINER(table), 8 );
	gtk_table_set_row_spacings(GTK_TABLE(table), 8);
	gtk_table_set_col_spacings(GTK_TABLE(table), 8);

	// First row
	top = 0;
	label = gtk_label_new(_("Name"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, top, (top + 1), GTK_FILL, 0, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	name_entry = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table), name_entry, 1, 2, top, (top + 1), GTK_EXPAND|GTK_SHRINK|GTK_FILL, 0, 0, 0);

	check_btn = gtk_button_new_with_label( _(" Check File "));
	gtk_table_attach(GTK_TABLE(table), check_btn, 2, 3, top, (top + 1), GTK_FILL, 0, 3, 0);

	// Second row
	top = 1;
	label = gtk_label_new(_("File"));
	gtk_table_attach(GTK_TABLE(table), label, 0, 1, top, (top + 1), GTK_FILL, 0, 0, 0);
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);

	file_entry = gtk_entry_new();
	gtk_table_attach(GTK_TABLE(table), file_entry, 1, 2, top, (top + 1), GTK_EXPAND|GTK_SHRINK|GTK_FILL, 0, 0, 0);

	file_btn = gtk_button_new_with_label( _(" ... "));
	gtk_table_attach(GTK_TABLE(table), file_btn, 2, 3, top, (top + 1), GTK_FILL, 0, 3, 0);

	// Third row
	top = 2;
	frame_custom = gtk_frame_new(_("Additional e-Mail address item(s)"));
	gtk_table_attach(GTK_TABLE(table), frame_custom, 1, 2, top, (top + JPILOT_NUM_CUSTOM_LABEL), GTK_FILL, 0, 0, 0);

	// Now do custom labels.
	vbox_custom = gtk_vbox_new (FALSE, 8);
	for( i = 0; i < JPILOT_NUM_CUSTOM_LABEL; i++ ) {
		hlbox = gtk_hbox_new( FALSE, 0 );
		custom_check[i] = gtk_check_button_new();
		custom_label[i] = gtk_label_new( "" );
		gtk_box_pack_start( GTK_BOX(hlbox), custom_check[i], FALSE, FALSE, 0 );
		gtk_box_pack_start( GTK_BOX(hlbox), custom_label[i], TRUE, TRUE, 0 );
		gtk_box_pack_start( GTK_BOX(vbox_custom), hlbox, TRUE, TRUE, 0 );
		gtk_misc_set_alignment(GTK_MISC(custom_label[i]), 0, 0.5);
		top++;
	}
	gtk_container_add (GTK_CONTAINER (frame_custom), vbox_custom);
	gtk_container_set_border_width( GTK_CONTAINER(vbox_custom), 8 );

	// Status line
	hsbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_end(GTK_BOX(vbox), hsbox, FALSE, FALSE, BORDER_WIDTH);
	statusbar = gtk_statusbar_new();
	gtk_box_pack_start(GTK_BOX(hsbox), statusbar, TRUE, TRUE, BORDER_WIDTH);

	// Button panel
	gtkut_button_set_create(&hbbox, &ok_btn, _("OK"),
				&cancel_btn, _("Cancel"), NULL, NULL);
	gtk_box_pack_end(GTK_BOX(vbox), hbbox, FALSE, FALSE, 0);
	gtk_container_set_border_width( GTK_CONTAINER(hbbox), 0 );
	gtk_widget_grab_default(ok_btn);

	hsep = gtk_hseparator_new();
	gtk_box_pack_end(GTK_BOX(vbox), hsep, FALSE, FALSE, 0);

	gtk_signal_connect(GTK_OBJECT(ok_btn), "clicked",
			   GTK_SIGNAL_FUNC(edit_jpilot_ok), cancelled);
	gtk_signal_connect(GTK_OBJECT(cancel_btn), "clicked",
			   GTK_SIGNAL_FUNC(edit_jpilot_cancel), cancelled);
	gtk_signal_connect(GTK_OBJECT(file_btn), "clicked",
			   GTK_SIGNAL_FUNC(edit_jpilot_file_select), NULL);
	gtk_signal_connect(GTK_OBJECT(check_btn), "clicked",
			   GTK_SIGNAL_FUNC(edit_jpilot_file_check), NULL);

	gtk_widget_show_all(vbox);

	jpilotedit.window     = window;
	jpilotedit.name_entry = name_entry;
	jpilotedit.file_entry = file_entry;
	jpilotedit.ok_btn     = ok_btn;
	jpilotedit.cancel_btn = cancel_btn;
	jpilotedit.statusbar  = statusbar;
	jpilotedit.status_cid = gtk_statusbar_get_context_id( GTK_STATUSBAR(statusbar), "Edit JPilot Dialog" );
	for( i = 0; i < JPILOT_NUM_CUSTOM_LABEL; i++ ) {
		jpilotedit.custom_check[i] = custom_check[i];
		jpilotedit.custom_label[i] = custom_label[i];
	}
}

AddressJPilot *addressbook_edit_jpilot( AddressJPilot *jpilot ) {
	static gboolean cancelled;
	gchar *sName;
	gchar *sFile;
	JPilotFile *jpf;
	gboolean fin;

	if (!jpilotedit.window)
		addressbook_edit_jpilot_create(&cancelled);
	gtk_widget_grab_focus(jpilotedit.ok_btn);
	gtk_widget_grab_focus(jpilotedit.name_entry);
	gtk_widget_show(jpilotedit.window);
	manage_window_set_transient(GTK_WINDOW(jpilotedit.window));

	edit_jpilot_status_show( "" );
	if( jpilot ) {
		jpf = jpilot->pilotFile;
		if (jpf->name)
			gtk_entry_set_text(GTK_ENTRY(jpilotedit.name_entry), jpf->name);
		if (jpf->path)
			gtk_entry_set_text(GTK_ENTRY(jpilotedit.file_entry), jpf->path);
		gtk_window_set_title( GTK_WINDOW(jpilotedit.window), _("Edit JPilot Entry"));
		edit_jpilot_fill_check_box( jpf );
	}
	else {
		gchar *guessFile = jpilot_find_pilotdb();
		gtk_entry_set_text(GTK_ENTRY(jpilotedit.name_entry), ADDRESSBOOK_GUESS_JPILOT );
		gtk_entry_set_text(GTK_ENTRY(jpilotedit.file_entry), guessFile );
		gtk_window_set_title( GTK_WINDOW(jpilotedit.window), _("Add New JPilot Entry"));
		edit_jpilot_fill_check_box_new();
		// Attempt to load labels
		if( *guessFile != '\0' ) {
			edit_jpilot_file_check();
		}
	}

	gtk_main();
	gtk_widget_hide(jpilotedit.window);
	if (cancelled == TRUE) return NULL;

	fin = FALSE;
	sName = gtk_editable_get_chars( GTK_EDITABLE(jpilotedit.name_entry), 0, -1 );
	sFile = gtk_editable_get_chars( GTK_EDITABLE(jpilotedit.file_entry), 0, -1 );
	if( *sName == '\0' ) fin = TRUE;
	if( *sFile == '\0' ) fin = TRUE;

	if( ! fin ) {
		if( ! jpilot ) {
			jpilot = g_new0(AddressJPilot, 1);
			ADDRESS_OBJECT_TYPE(jpilot) = ADDR_JPILOT;
			jpf = jpilot_create();
			jpilot->pilotFile = jpf;
		}
		g_free( jpilot->name );
		jpilot->name = g_strdup( sName );
		jpilot_set_name( jpf, sName );
		jpilot_set_file( jpf, sFile );
		edit_jpilot_read_check_box( jpf );
	}
	g_free( sName );
	g_free( sFile );

	return jpilot;
}

#endif /* USE_JPILOT */

/*
* End of Source.
*/

