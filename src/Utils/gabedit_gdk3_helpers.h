/* gabedit_gdk3_helpers.h */
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

#ifndef __GABEDIT_GDK3_HELPERS_H__
#define __GABEDIT_GDK3_HELPERS_H__

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo.h>
#include <pango/pangocairo.h>

/**
 * capture_widget_to_pixbuf:
 * @widget: A GtkWidget to capture
 *
 * Captures the contents of a widget into a GdkPixbuf using GTK3 APIs.
 * This function uses gdk_pixbuf_get_from_window() to capture the widget's
 * rendered content from its GdkWindow.
 *
 * Returns: A newly created GdkPixbuf containing the widget's content,
 *          or NULL if the widget is not realized or has no window.
 *          The caller must free the returned pixbuf with g_object_unref().
 */
GdkPixbuf* capture_widget_to_pixbuf(GtkWidget *widget);

/**
 * draw_pango_layout_with_cairo:
 * @cr: A cairo context to draw on
 * @layout: A PangoLayout to render
 * @x: X coordinate for the layout position
 * @y: Y coordinate for the layout position
 * @centerX: Whether to center the layout horizontally at x
 * @centerY: Whether to center the layout vertically at y
 * @angle: Rotation angle in radians
 *
 * Draws a PangoLayout on a cairo context with optional centering and rotation.
 * The rotation is applied around the specified (x, y) point. If centerX or
 * centerY are TRUE, the layout is centered at the given coordinates before
 * rotation is applied.
 *
 * This function saves and restores the cairo context state, so it won't
 * affect subsequent drawing operations.
 */
void draw_pango_layout_with_cairo(cairo_t *cr, PangoLayout *layout, int x, int y, gboolean centerX, gboolean centerY, double angle);

#endif /* __GABEDIT_GDK3_HELPERS_H__ */
