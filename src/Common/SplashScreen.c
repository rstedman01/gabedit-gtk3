/* SplashScreen.c */
/**********************************************************************************************************
Copyright (c) 2002-2021 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the Gabedit), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
************************************************************************************************************/

#include "../../Config.h"
#include <stdio.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <cairo.h>
#include <pango/pangocairo.h>
#include "../Common/Global.h"
#include "../Utils/UtilsInterface.h"
#include "../Utils/Utils.h"
#include "../Utils/AtomsProp.h"
#include "../Common/Status.h"
#include "../Common/Windows.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"
#include "../Geometry/PersonalFragments.h"
#include "../MolecularMechanics/Atom.h"
#include "../MolecularMechanics/Molecule.h"
#include "../MolecularMechanics/ForceField.h"
#include "../MolecularMechanics/MolecularMechanics.h"
#include "../MolecularMechanics/PDBTemplate.h"
#include "../Molpro/MolproBasisLibrary.h"
#include "../Molcas/MolcasBasisLibrary.h"
#include "../MPQC/MPQCBasisLibrary.h"
#include "../Display/UtilsOrb.h"
#include "../Compat/gabedit_gdk_compat.h"

#define SPLASH_SURFACE_KEY "splash-surface"

static GdkGC *gc = NULL;
/*static guint IdTimer = 0;*/
static	GtkWidget *status = NULL;
static guint idStatus = 0;
static gint splash_screen_cb(gpointer data);
static gint progress( gpointer data,gdouble step);
static cairo_surface_t *splash_surface = NULL;
static GtkWidget *status;
static guint idStatus;

static gboolean progress(gpointer data, gdouble step);
static void draw_splash_contents_on_surface(GtkWidget *widget, cairo_surface_t *surface);
static gboolean splash_configure_cb(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data);
static gboolean splash_draw_cb(GtkWidget *widget, cairo_t *cr, gpointer user_data);
static void splash_destroy_cb(GtkWidget *widget, gpointer user_data);
static void create_surface_for_widget(GtkWidget *widget);

/********************************************************************************/
static void splash_create_surface(GtkWidget *widget)
{
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);
    GdkWindow *win = gtk_widget_get_window(widget);
    if (!win) return;

    if (splash_surface) {
        cairo_surface_destroy(splash_surface);
        splash_surface = NULL;
    }

    /* create a surface compatible with the widget window */
    splash_surface = gdk_window_create_similar_surface(win,
                                                       CAIRO_CONTENT_COLOR_ALPHA,
                                                       alloc.width,
                                                       alloc.height);
    if (!splash_surface) return;

    cairo_t *cr = cairo_create(splash_surface);

    /* Example: gradient background */
    cairo_rectangle(cr, 0, 0, alloc.width, alloc.height);
    cairo_pattern_t *pat = cairo_pattern_create_linear(0, 0, 0, alloc.height);
    cairo_pattern_add_color_stop_rgba(pat, 0.0, 0.95, 0.97, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pat, 1.0, 0.75, 0.85, 0.95, 1.0);
    cairo_set_source(cr, pat);
    cairo_fill(cr);
    cairo_pattern_destroy(pat);

    /* Example: draw arcs / decorations */
    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgba(cr, 0.2, 0.2, 0.2, 0.8);
    /* draw a corner arc (top-left) */
    double radius = MIN(alloc.width, alloc.height) * 0.15;
    cairo_arc(cr, radius, radius, radius, G_PI, 1.5 * G_PI);
    cairo_stroke(cr);

    /* Example: render text using PangoCairo */
    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *fd = pango_font_description_from_string("sans bold 20");
    pango_layout_set_font_description(layout, fd);
    pango_layout_set_text(layout, "Gabedit", -1);
    int tw, th;
    pango_layout_get_pixel_size(layout, &tw, &th);
    cairo_set_source_rgb(cr, 0.05, 0.05, 0.05);
    cairo_move_to(cr, 20.0, 20.0);
    pango_cairo_show_layout(cr, layout);

    g_object_unref(layout);
    pango_font_description_free(fd);

    cairo_destroy(cr);
}
/********************************************************************************/
static void gdkcolor_to_rgba(const GdkColor *c, double *r, double *g, double *b)
{
    if (!c) { *r = *g = *b = 0.0; return; }
    *r = c->red   / 65535.0;
    *g = c->green / 65535.0;
    *b = c->blue  / 65535.0;
}
/********************************************************************************/
static void splash_destroy_cb(GtkWidget *widget, gpointer user_data)
{
    (void)user_data;
    cairo_surface_t *surface = (cairo_surface_t*)g_object_get_data(G_OBJECT(widget), SPLASH_SURFACE_KEY);
    if (surface) {
        g_object_set_data(G_OBJECT(widget), SPLASH_SURFACE_KEY, NULL);
        cairo_surface_destroy(surface);
    }
}
/********************************************************************************/
static gboolean splash_configure_cb(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
    (void)event; (void)user_data;
    create_surface_for_widget(widget);
	gtk_widget_queue_draw(widget);
    return TRUE;
}
/********************************************************************************/
static gboolean splash_draw_cb(GtkWidget *widget, cairo_t *cr, gpointer user_data)
{
	(void)user_data;
	cairo_surface_t *surface = (cairo_surface_t*)g_object_get_data(G_OBJECT(widget), SPLASH_SURFACE_KEY);
    if (splash_surface) {
        cairo_set_source_surface(cr, splash_surface, 0, 0);
        cairo_paint(cr);
		return FALSE;
    } else {
        
	    GtkAllocation alloc;
	    gtk_widget_get_allocation(widget, &alloc);
	    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
	    cairo_rectangle(cr, 0, 0, alloc.width, alloc.height);
	    cairo_fill(cr);

        return FALSE;
    }
}
/********************************************************************************/
static void splash_draw_text(cairo_t *cr, const char *text, const char *font_desc, double x, double y, double r, double g, double b)
{
    PangoLayout *layout = pango_cairo_create_layout(cr);
    PangoFontDescription *fd = pango_font_description_from_string(font_desc);
    pango_layout_set_font_description(layout, fd);
    pango_layout_set_text(layout, text, -1);
    cairo_set_source_rgb(cr, r, g, b);
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, layout);
    g_object_unref(layout);
    pango_font_description_free(fd);
}
/********************************************************************************/
static void set_statubar_str(gchar* str)
{
	if(str)
	{
		gtk_statusbar_pop(GTK_STATUSBAR(status), idStatus);
		gtk_statusbar_push(GTK_STATUSBAR(status), idStatus, str);
	}
	while(gtk_events_pending())
		gtk_main_iteration();
}
/*************************************************************************************/
static void read_ressource_files(GtkWidget* MainFrame,GtkWidget* ProgressBar)
{
 
	gdouble t[] = {0.20,0.10,0.10,0.10,0.10,0.20,0.20};
	gint i = 0;
	static gdouble step = 2;

	set_statubar_str(" ");
	define_default_atoms_prop();
	set_statubar_str(_("Load atom properties....."));

	if(!read_atoms_prop())
	{
		Waiting(step*t[i]);
		progress( ProgressBar,t[i]);
 		set_statubar_str(_("I can not load atom properties from file, define default values....."));
 		define_default_atoms_prop();
	}
	else
	{
		Waiting(step*t[i]);
		progress( ProgressBar,t[i]);
	}

	i++;
	set_statubar_str(_("Load default commands from \"commands\" file....."));
	read_commands_file();
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	i++;
	set_statubar_str(_("Load recent fonts styles....."));
	read_fonts_file();
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	i++;
	set_statubar_str(_("Load recent hosts list....."));
	read_hosts_file();
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	i++;
	set_statubar_str(_("Load recent surfaces colors....."));
	read_color_surfaces_file();
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	i++;
	set_statubar_str(_("Load list of recent projects....."));
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	i++;
	set_statubar_str(_("Load Molecular mechanics parameters....."));
	loadAmberParameters();
	LoadPDBTpl();	
	Waiting(step*t[i]);
	progress( ProgressBar,t[i]);

	set_statubar_str(_("Load Personal Fragments....."));
	loadPersonalFragments(NULL);

	set_statubar_str(_("Load the list of the bases supported by Molpro......"));
	loadMolproBasis();

	set_statubar_str(_("Load the list of the bases supported by Molcas......"));
	loadMolcasBasis();

	set_statubar_str(_("Load the list of the bases supported by MPQC......"));
	loadMPQCBasis();

	set_statubar_str("   ");

	Waiting(step*0.05);

	splash_screen_cb(MainFrame);
}
/********************************************************************************/
static gint show_gabedit(gpointer data)
{
	gtk_widget_show_all(Fenetre);
	hide_progress_connection();
  	gtk_widget_hide (GTK_WIDGET(FrameWins));
	{
		gchar* fileName = g_strdup_printf("%s%sdump.txt",  g_get_home_dir(), G_DIR_SEPARATOR_S);
		set_last_directory(fileName);
		g_free(fileName);
	}
	return FALSE;
}
/********************************************************************************/
static gint splash_screen_cb(gpointer data)
{
	gtk_widget_hide(GTK_WIDGET(data));
	gtk_object_destroy(GTK_OBJECT(data));
	show_gabedit(NULL);
	return FALSE;
}
/********************************************************************************/
static gint progress(gpointer data, gdouble step)
{
    gdouble new_val;

    if(!this_is_an_object((GtkObject*)data))
    {
	return FALSE;	
    }
    new_val = gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(data)) + step;
    if (new_val > 1) new_val = 1;

     gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR(data), (gdouble)(new_val));

    return TRUE;
} 
/***********************************************************/
static GtkWidget *create_progress_bar_splash(GtkWidget *box)
{
	GtkWidget *table;
  	GtkWidget *ProgressBar;

	table = gtk_table_new(1,2,FALSE);
	
	gtk_box_pack_start (GTK_BOX(box), table, FALSE, TRUE, 2);

	status = gtk_statusbar_new();
	gtk_widget_show(status);
	gtk_table_attach(GTK_TABLE(table),status,0,1,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			(GtkAttachOptions)(GTK_FILL | GTK_EXPAND),
			1,1);
	idStatus = gtk_statusbar_get_context_id(GTK_STATUSBAR(status)," ");

	ProgressBar = gtk_progress_bar_new();

	gtk_table_attach(GTK_TABLE(table),ProgressBar,1,2,0,1,
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			(GtkAttachOptions)(GTK_FILL | GTK_SHRINK),
			1,1);
	gtk_widget_show (table);

	return ProgressBar;
}
/********************************************************************************/
static gint configure_event(GtkWidget *widget, GdkEventConfigure *event)
{
    (void)event;
    if (!GTK_IS_WIDGET(widget)) return FALSE;
    create_surface_for_widget(widget);
    gtk_widget_queue_draw(widget);
    return TRUE;
}
/********************************************************************************/   
static gint expose_event(GtkWidget *widget,GdkEventExpose *event, gpointer user_data)
{
  	(void)user_data;
    if (!GTK_IS_WIDGET(widget)) return FALSE;

    cairo_surface_t *surface = (cairo_surface_t*)g_object_get_data(G_OBJECT(widget), SPLASH_SURFACE_KEY);
    if (!surface) return FALSE;

    GdkWindow *win = gtk_widget_get_window(widget);
    if (!win) return FALSE;

    cairo_t *cr = gdk_cairo_create(win);
    cairo_rectangle(cr, event->area.x, event->area.y, event->area.width, event->area.height);
    cairo_clip(cr);

    cairo_set_source_surface(cr, surface, 0, 0);
    cairo_paint(cr);
    cairo_destroy(cr);
    return FALSE;
}
/********************************************************************************/
static void setup_text_darea(GtkWidget *darea, gchar *txt, PangoFontDescription *font_desc, GdkColor *color, gint *lentxt)
{
    g_object_set_data(G_OBJECT(darea), "Text", txt);
    g_object_set_data(G_OBJECT(darea), "FontDesc", font_desc);
    g_object_set_data(G_OBJECT(darea), "Color", color);
    g_object_set_data(G_OBJECT(darea), "LenTxt", lentxt);

    /* connect modern handlers */
    g_signal_connect(G_OBJECT(darea), "configure-event", G_CALLBACK(splash_configure_cb), NULL);
    g_signal_connect(G_OBJECT(darea), "draw", G_CALLBACK(splash_draw_cb), NULL);
    g_signal_connect(G_OBJECT(darea), "destroy", G_CALLBACK(splash_destroy_cb), NULL);
}
/********************************************************************************/
static void create_welcome_frame_popup(GtkWidget *vbox, GtkWidget *MainFrame)
{
    GtkWidget *darea;
    gchar *txt = g_strdup(_("       Welcome to :"));
    GdkColor* color = g_malloc(sizeof(GdkColor));
    PangoFontDescription *font_desc = pango_font_description_from_string ("sans bold 20");
    gint* lentxt = g_malloc(sizeof(gint));

    if(!gdk_color_parse("sky blue", color)) {
        color->red = 40000;
        color->green = 40000;
        color->blue = 40000;
    }

    gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, NULL);
    gint height = 0;
    gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, &height);
    gint width = *lentxt + 0; /* conservative; version string not included here */

    darea = gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(darea), width, height + 10);
    gtk_box_pack_start(GTK_BOX(vbox), darea, FALSE, FALSE, 0);

    setup_text_darea(darea, txt, font_desc, color, lentxt);
}
/********************************************************************************/
static void create_name_frame_popup(GtkWidget *vbox, GtkWidget *MainFrame)
{
    GtkWidget *darea;
    gchar *txt = g_strdup(_("Abdul-Rahman Allouche presents : "));
    GdkColor* color = g_malloc(sizeof(GdkColor));
    PangoFontDescription *font_desc = pango_font_description_from_string ("sans bold 16");
    gint* lentxt = g_malloc(sizeof(gint));

    if(!gdk_color_parse("sky blue",color))
    {
        color->red = 40000;
        color->green = 40000;
        color->blue = 40000;
    }

    gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, NULL);
    gint height = 0;
    gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, &height);
    gint width = *lentxt + 0;

    darea = gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(darea), width, height + 10);
    gtk_box_pack_start(GTK_BOX(vbox), darea, FALSE, FALSE, 0);

    setup_text_darea(darea, txt, font_desc, color, lentxt);
}
/********************************************************************************/
static void create_program_frame_popup(GtkWidget *vbox, GtkWidget *MainFrame)
{
    GtkWidget *darea;
    gchar *txt = g_strdup("  The Gabedit ");
    GdkColor* color = g_malloc(sizeof(GdkColor));
    PangoFontDescription *font_desc = pango_font_description_from_string ("sans bold 50");
    gint* lentxt = g_malloc(sizeof(gint));

    if(!gdk_color_parse("sky blue",color))
    {
        color->red = 40000;
        color->green = 40000;
        color->blue = 40000;
    }

    gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, NULL);
    gint height = 0;
    gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, &height);
    *lentxt = *lentxt + (gint)(0.5 * (*lentxt));
    gint width = *lentxt + 2;

    darea = gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(darea), width, height + 10);
    gtk_box_pack_start(GTK_BOX(vbox), darea, FALSE, FALSE, 0);

    setup_text_darea(darea, txt, font_desc, color, lentxt);
}
/********************************************************************************/
static void create_gui_frame_popup(GtkWidget *vbox, GtkWidget *MainFrame)
{
    GtkWidget *darea;
    gchar *txt = g_strdup(_("The Graphical User Interface for computational chemistry packages"));
    GdkColor* color = g_malloc(sizeof(GdkColor));
    PangoFontDescription *font_desc = pango_font_description_from_string ("sans bold 16");
    gint* lentxt = g_malloc(sizeof(gint));

    if(!gdk_color_parse("sky blue",color))
    {
        color->red = 40000;
        color->green = 40000;
        color->blue = 40000;
    }

    gint height = 0;
    gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, &height);
    gint width = *lentxt + height;

    darea = gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(darea), width, height + 10);
    gtk_box_pack_start(GTK_BOX(vbox), darea, FALSE, FALSE, 0);

    setup_text_darea(darea, txt, font_desc, color, lentxt);
}
/********************************************************************************/
static void create_splash_popupwin()
{
    GtkWidget *MainFrame;
    GtkWidget *ProgressBar;
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    MainFrame = gtk_window_new(GTK_WINDOW_POPUP);

    /* center it on the screen */
    gtk_window_set_position(GTK_WINDOW(MainFrame), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(MainFrame), 4);

    gtk_widget_add_events(MainFrame,
                          GDK_BUTTON_PRESS_MASK |
                          GDK_BUTTON_RELEASE_MASK |
                          GDK_KEY_PRESS_MASK);

    gtk_widget_realize(MainFrame);

    gtk_container_add(GTK_CONTAINER(MainFrame), vbox);

    /* create the sub-popups (they create and pack their drawing areas into vbox) */
    create_welcome_frame_popup(vbox, MainFrame);
    create_program_frame_popup(vbox, MainFrame);
    create_gui_frame_popup(vbox, MainFrame);

    ProgressBar = create_progress_bar_splash(vbox);
    g_object_set_data(G_OBJECT(ProgressBar), "MainFrame", MainFrame);

    gtk_widget_show_all(MainFrame);

    /* process pending events before loading resources */
    while (gtk_events_pending())
        gtk_main_iteration();

    read_ressource_files(MainFrame, ProgressBar);
}
/********************************************************************************/
static void create_copyright_frame_popup(GtkWidget *vbox, GtkWidget *MainFrame)
{
    GtkWidget *darea;
    gchar *txt = g_strdup("Copyright (c) 2002-2021 Abdul-Rahman Allouche.");
    GdkColor* color = g_malloc(sizeof(GdkColor));
    PangoFontDescription *font_desc = pango_font_description_from_string("sans bold 16");
    gint* lentxt = g_malloc(sizeof(gint));

    if(!gdk_color_parse("sky blue", color)) {
        color->red = 40000; color->green = 40000; color->blue = 40000;
    }

    gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, NULL);
    gint height = 0;
    gabedit_string_get_pixel_size(Fenetre, font_desc, txt, lentxt, &height);
    gint width = *lentxt + 10;

    darea = gtk_drawing_area_new();
    gtk_widget_set_size_request(GTK_WIDGET(darea), width, height + 10);
    gtk_box_pack_start(GTK_BOX(vbox), darea, FALSE, FALSE, 0);

    setup_text_darea(darea, txt, font_desc, color, lentxt);
}
/********************************************************************************/
void splash_screen()
{
      create_splash_popupwin();
}
/********************************************************************************/
void create_about_frame()
{
    GtkWidget *MainFrame;
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

    MainFrame = gtk_window_new(GTK_WINDOW_POPUP);

    gtk_window_set_position(GTK_WINDOW(MainFrame), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(MainFrame), 4);

    gtk_widget_add_events(MainFrame, GDK_BUTTON_PRESS_MASK | GDK_BUTTON_RELEASE_MASK | GDK_KEY_PRESS_MASK | GDK_KEY_RELEASE_MASK);

    g_signal_connect(G_OBJECT(MainFrame), "button_release_event", G_CALLBACK(gtk_widget_destroy), NULL);
    g_signal_connect(G_OBJECT(MainFrame), "key_press_event", G_CALLBACK(gtk_widget_destroy), NULL);

    gtk_widget_realize(MainFrame);

    gtk_container_add(GTK_CONTAINER(MainFrame), vbox);

    create_name_frame_popup(vbox, MainFrame);
    create_program_frame_popup(vbox, MainFrame);
    create_gui_frame_popup(vbox, MainFrame);
    create_copyright_frame_popup(vbox, MainFrame);

    gtk_widget_show_all(MainFrame);
}
/********************************************************************************/
static void draw_splash_contents_on_surface(GtkWidget *widget, cairo_surface_t *surface)
{
    if (!GTK_IS_WIDGET(widget) || !surface) return;

    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);
    int width = alloc.width;
    int height = alloc.height;
    if (width <= 0 || height <= 0) return;

    /* Obtain per-widget metadata */
    gchar *txt = (gchar*)g_object_get_data(G_OBJECT(widget), "Text");
    PangoFontDescription *font_desc = (PangoFontDescription*)g_object_get_data(G_OBJECT(widget), "FontDesc");
    GdkColor *pcolor = (GdkColor*)g_object_get_data(G_OBJECT(widget), "Color");
    gint *lentxt = (gint*)g_object_get_data(G_OBJECT(widget), "LenTxt");

    cairo_t *cr = cairo_create(surface);

    /* Background: simple vertical gradient based on pcolor if available */
    if (pcolor) {
        double r,g,b;
        gdkcolor_to_rgba(pcolor, &r, &g, &b);
        cairo_pattern_t *pat = cairo_pattern_create_linear(0, 0, 0, height);
        /* Slight variation of color to create subtle gradient */
        cairo_pattern_add_color_stop_rgba(pat, 0.0, MIN(1.0, r+0.1), MIN(1.0, g+0.05), MIN(1.0, b+0.02), 1.0);
        cairo_pattern_add_color_stop_rgba(pat, 1.0, r*0.75, g*0.75, b*0.75, 1.0);
        cairo_set_source(cr, pat);
        cairo_rectangle(cr, 0, 0, width, height);
        cairo_fill(cr);
        cairo_pattern_destroy(pat);
    } else {
        /* fallback plain background */
        cairo_set_source_rgb(cr, 0.95, 0.97, 1.0);
        cairo_rectangle(cr, 0, 0, width, height);
        cairo_fill(cr);
    }

    /* Decorative arcs (corner accents), use proportions similar to original */
    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgba(cr, 0.15, 0.15, 0.15, 0.6);
    double radius = MIN(width, height) * 0.15;
    /* top-left */
    cairo_arc(cr, radius, radius, radius, G_PI, 1.5 * G_PI);
    cairo_stroke(cr);
    /* top-right */
    cairo_arc(cr, width - radius, radius, radius, 1.5 * G_PI, 2.0 * G_PI);
    cairo_stroke(cr);
    /* bottom-left */
    cairo_arc(cr, radius, height - radius, radius, 0.5 * G_PI, G_PI);
    cairo_stroke(cr);
    /* bottom-right */
    cairo_arc(cr, width - radius, height - radius, radius, 0.0, 0.5 * G_PI);
    cairo_stroke(cr);

    /* Render main text (txt) using PangoCairo */
    if (txt && font_desc) {
        PangoLayout *layout = pango_cairo_create_layout(cr);
        pango_layout_set_font_description(layout, font_desc);
        pango_layout_set_text(layout, txt, -1);

        int tw=0, th=0;
        pango_layout_get_pixel_size(layout, &tw, &th);

        /* Positioning: replicate original heuristics */
        int x = 6;
        int y = 10;
        if (g_strstr_len(txt, -1, "Gabedit") && !g_strstr_len(txt, -1, "Copyright")) {
            x = 20;
            y = 10;
        } else if (g_strstr_len(txt, -1, "Allouche") && !g_strstr_len(txt, -1, "Copyright")) {
            x = width / 4;
            y = 10;
        } else if (g_strstr_len(txt, -1, "Graphical")) {
            x = 6;
            y = 10;
        }

        /* Draw drop-shadow / highlight effect similar to original black/white layering */
        cairo_move_to(cr, x + (height / 20.0), y + (height / 18.0));
        cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.85);
        pango_cairo_show_layout(cr, layout);

        cairo_move_to(cr, x, y);
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
        pango_cairo_show_layout(cr, layout);

        g_object_unref(layout);
    }

    /* Draw Version string on the right when text contains "Gabedit" */
    if (txt && g_strstr_len(txt, -1, "Gabedit")) {
        gchar *Version_S = g_strdup_printf("%d.%d.%d", MAJOR_VERSION, MINOR_VERSION, MICRO_VERSION);
        PangoFontDescription *vfd = pango_font_description_from_string("sans 12");
        PangoLayout *vlayout = pango_cairo_create_layout(cr);
        pango_layout_set_font_description(vlayout, vfd);
        pango_layout_set_text(vlayout, Version_S, -1);
        int vw, vh;
        pango_layout_get_pixel_size(vlayout, &vw, &vh);

        int vx = (lentxt && *lentxt > 0) ? (*lentxt + (int)(0.05 * vw)) : (width - vw - 10);
        int vy = 5;

        cairo_move_to(cr, vx, vy);
        cairo_set_source_rgba(cr, 0.05, 0.05, 0.5, 1.0);
        pango_cairo_show_layout(cr, vlayout);

        g_object_unref(vlayout);
        pango_font_description_free(vfd);
        g_free(Version_S);
    }

    cairo_destroy(cr);
}
/********************************************************************************/
