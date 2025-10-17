/* gabedit_gdk3_helpers.c */
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

#include "gabedit_gdk3_helpers.h"
#include <math.h>

/**
 * capture_widget_to_pixbuf:
 * @widget: A GtkWidget to capture
 *
 * Captures the contents of a widget into a GdkPixbuf.
 * Uses GTK3's gdk_pixbuf_get_from_window() function.
 */
GdkPixbuf* capture_widget_to_pixbuf(GtkWidget *widget)
{
	GdkWindow *window;
	gint width, height;
	
	if (!widget || !GTK_IS_WIDGET(widget))
		return NULL;
	
	if (!gtk_widget_get_realized(widget))
		return NULL;
	
	window = gtk_widget_get_window(widget);
	if (!window)
		return NULL;
	
	width = gtk_widget_get_allocated_width(widget);
	height = gtk_widget_get_allocated_height(widget);
	
	return gdk_pixbuf_get_from_window(window, 0, 0, width, height);
}

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
 * Draws a PangoLayout with optional centering and rotation.
 */
void draw_pango_layout_with_cairo(cairo_t *cr, PangoLayout *layout, int x, int y, gboolean centerX, gboolean centerY, double angle)
{
	gint w, h;
	gdouble xpos, ypos;
	
	if (!cr || !layout)
		return;
	
	cairo_save(cr);
	
	xpos = (gdouble)x;
	ypos = (gdouble)y;
	
	/* Calculate centering offsets with rotation if needed */
	if (centerX || centerY)
	{
		pango_layout_get_size(layout, &w, &h);
		if (centerX)
			xpos -= w / 2.0 / PANGO_SCALE * cos(angle) - h / 2.0 / PANGO_SCALE * sin(angle);
		if (centerY)
			ypos -= w / 2.0 / PANGO_SCALE * sin(angle) + h / 2.0 / PANGO_SCALE * cos(angle);
	}
	
	/* Move to the drawing position */
	cairo_move_to(cr, xpos, ypos);
	
	/* Apply rotation */
	cairo_rotate(cr, angle);
	
	/* Draw the layout */
	pango_cairo_show_layout(cr, layout);
	
	cairo_restore(cr);
}
