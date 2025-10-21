
#ifndef __GABEDIT_GDK3_HELPERS_H__
#define __GABEDIT_GDK3_HELPERS_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo.h>
#include <pango/pangocairo.h>

GdkPixbuf* capture_widget_to_pixbuf(GtkWidget *widget);
void draw_pango_layout_with_cairo(cairo_t *cr, PangoLayout *layout, int x, int y, gboolean centerX, gboolean centerY, double angle);

#endif