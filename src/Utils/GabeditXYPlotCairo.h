#ifndef __GABEDIT_XYPLOT_CAIRO_H__
#define __GABEDIT_XYPLOT_CAIRO_H__

#include "../../gabeditGTK3compat.h"
#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

typedef struct {
    GdkRGBA color;
    gint line_width;
    GdkLineStyle line_style;
    cairo_line_cap_t cap_style;
    cairo_line_join_t join_style;
} XYPlotDrawAttr;

// Create a new drawing attribute structure 
XYPlotDrawAttr* xyplot_draw_attr_new(void);

// Free a drawing attribute structure
void xyplot_draw_attr_free(XYPlotDrawAttr* attr);

// Set color from GdkColor 
void xyplot_draw_attr_set_color_gdk(XYPlotDrawAttr* attr, const GdkColor* color);

// Set color from RGB values 
void xyplot_draw_attr_set_color_rgb(XYPlotDrawAttr* attr, gdouble red, gdouble green, gdouble blue);

// Set color from RGBA values 
void xyplot_draw_attr_set_color_rgba(XYPlotDrawAttr* attr, const GdkRGBA* color);

// Set line attributes 
void xyplot_draw_attr_set_line(XYPlotDrawAttr* attr, gint line_width, GdkLineStyle line_style, 
                               cairo_line_cap_t cap_style, cairo_line_join_t join_style);

// Apply drawing attributes to a Cairo context 
void xyplot_draw_attr_apply(XYPlotDrawAttr* attr, cairo_t* cr);

G_END_DECLS

#endif // __GABEDIT_XYPLOT_CAIRO_H__