#include "GabeditGDK3Helpers.h"
#include <math.h>

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

void draw_pango_layout_with_cairo(cairo_t *cr, PangoLayout *layout, int x, int y, gboolean centerX, gboolean centerY, double angle)
{
	gint w, h;
	gdouble xpos, ypos;

	if (!cr || !layout)
		return;

	cairo_save(cr);

	xpos = (gdouble)x;
	ypos = (gdouble)y;

    if (centerX || centerY)
	{
		pango_layout_get_size(layout, &w, &h);
		if (centerX)
			xpos -= w / 2.0 / PANGO_SCALE * cos(angle) - h / 2.0 / PANGO_SCALE * sin(angle);
		if (centerY)
			ypos -= w / 2.0 / PANGO_SCALE * sin(angle) + h / 2.0 / PANGO_SCALE * cos(angle);
	}

    cairo_move_to(cr, xpos, ypos);
    cairo_rotate(cr, angle);
    pango_cairo_show_layout(cr, layout);

	cairo_restore(cr);
}