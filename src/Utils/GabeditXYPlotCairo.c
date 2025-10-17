/* GabeditXYPlotCairo.c */
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

#include "../../gabeditGTK3compat.h"
#include "GabeditXYPlotCairo.h"
#include <stdlib.h>
#include <string.h>

/* Create a new drawing attribute structure */
XYPlotDrawAttr* xyplot_draw_attr_new(void)
{
    XYPlotDrawAttr* attr = g_new0(XYPlotDrawAttr, 1);
    
    /* Default values */
    attr->color.red = 0.0;
    attr->color.green = 0.0;
    attr->color.blue = 0.0;
    attr->color.alpha = 1.0;
    attr->line_width = 1;
    attr->line_style = GDK_LINE_SOLID;
    attr->cap_style = CAIRO_LINE_CAP_BUTT;
    attr->join_style = CAIRO_LINE_JOIN_MITER;
    
    return attr;
}

/* Free a drawing attribute structure */
void xyplot_draw_attr_free(XYPlotDrawAttr* attr)
{
    if (attr) {
        g_free(attr);
    }
}

/* Set color from GdkColor (16-bit per channel) */
void xyplot_draw_attr_set_color_gdk(XYPlotDrawAttr* attr, const GdkColor* color)
{
    if (attr && color) {
        attr->color.red = color->red / 65535.0;
        attr->color.green = color->green / 65535.0;
        attr->color.blue = color->blue / 65535.0;
        attr->color.alpha = 1.0;
    }
}

/* Set color from RGB values (0.0-1.0) */
void xyplot_draw_attr_set_color_rgb(XYPlotDrawAttr* attr, gdouble red, gdouble green, gdouble blue)
{
    if (attr) {
        attr->color.red = red;
        attr->color.green = green;
        attr->color.blue = blue;
        attr->color.alpha = 1.0;
    }
}

/* Set color from RGBA values */
void xyplot_draw_attr_set_color_rgba(XYPlotDrawAttr* attr, const GdkRGBA* color)
{
    if (attr && color) {
        attr->color = *color;
    }
}

/* Set line attributes */
void xyplot_draw_attr_set_line(XYPlotDrawAttr* attr, gint line_width, GdkLineStyle line_style,
                               cairo_line_cap_t cap_style, cairo_line_join_t join_style)
{
    if (attr) {
        attr->line_width = line_width;
        attr->line_style = line_style;
        attr->cap_style = cap_style;
        attr->join_style = join_style;
    }
}

/* Apply drawing attributes to a Cairo context */
void xyplot_draw_attr_apply(XYPlotDrawAttr* attr, cairo_t* cr)
{
    if (!attr || !cr) return;
    
    /* Set color */
    cairo_set_source_rgba(cr, attr->color.red, attr->color.green, attr->color.blue, attr->color.alpha);
    
    /* Set line width */
    cairo_set_line_width(cr, attr->line_width > 0 ? attr->line_width : 1);
    
    /* Set line cap */
    cairo_set_line_cap(cr, attr->cap_style);
    
    /* Set line join */
    cairo_set_line_join(cr, attr->join_style);
    
    /* Set line style (dash pattern) */
    switch (attr->line_style) {
        case GDK_LINE_SOLID:
            cairo_set_dash(cr, NULL, 0, 0);
            break;
        case GDK_LINE_ON_OFF_DASH: {
            double dashes[] = {5.0, 5.0};
            cairo_set_dash(cr, dashes, 2, 0);
            break;
        }
        case GDK_LINE_DOUBLE_DASH: {
            double dashes[] = {5.0, 5.0, 10.0, 5.0};
            cairo_set_dash(cr, dashes, 4, 0);
            break;
        }
        default:
            cairo_set_dash(cr, NULL, 0, 0);
            break;
    }
}
