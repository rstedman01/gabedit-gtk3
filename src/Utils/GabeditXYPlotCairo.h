/* GabeditXYPlotCairo.h */
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

#ifndef __GABEDIT_XYPLOT_CAIRO_H__
#define __GABEDIT_XYPLOT_CAIRO_H__

#include "../../gabeditGTK3compat.h"
#include <gtk/gtk.h>
#include <cairo.h>

G_BEGIN_DECLS

/* Structure to replace GdkGC for Cairo drawing */
typedef struct {
    GdkRGBA color;
    gint line_width;
    GdkLineStyle line_style;
    cairo_line_cap_t cap_style;
    cairo_line_join_t join_style;
} XYPlotDrawAttr;

/* Create a new drawing attribute structure */
XYPlotDrawAttr* xyplot_draw_attr_new(void);

/* Free a drawing attribute structure */
void xyplot_draw_attr_free(XYPlotDrawAttr* attr);

/* Set color from GdkColor */
void xyplot_draw_attr_set_color_gdk(XYPlotDrawAttr* attr, const GdkColor* color);

/* Set color from RGB values */
void xyplot_draw_attr_set_color_rgb(XYPlotDrawAttr* attr, gdouble red, gdouble green, gdouble blue);

/* Set color from RGBA values */
void xyplot_draw_attr_set_color_rgba(XYPlotDrawAttr* attr, const GdkRGBA* color);

/* Set line attributes */
void xyplot_draw_attr_set_line(XYPlotDrawAttr* attr, gint line_width, GdkLineStyle line_style, 
                               cairo_line_cap_t cap_style, cairo_line_join_t join_style);

/* Apply drawing attributes to a Cairo context */
void xyplot_draw_attr_apply(XYPlotDrawAttr* attr, cairo_t* cr);

G_END_DECLS

#endif /* __GABEDIT_XYPLOT_CAIRO_H__ */
