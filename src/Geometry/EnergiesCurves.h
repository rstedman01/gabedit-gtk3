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

#ifndef __GABEDIT_ENERGIESCURVES_H__
#define __GABEDIT_ENERGIESCURVES_H__

extern gboolean GeomConvIsOpen;
void create_energies_curves(DataGeomConv*,gint );

typedef struct _EnergiesCurvesData 
{
  // Raw data points
  double *x; 
  double *y;
  int n;

  // Data ranges
  double xmin, xmax;
  double ymin, ymax;

  // Title/Labels
  gchar *title;
  gchar *xlabel;
  gchar *ylabel;

  // Interaction
  int hover_index;
  int selected_index;
  gboolean show_grid;
  gboolean show_points;
  gboolean show_curve;
  gboolean show_title;

  GdkRGBA bg_color;  
  GdkRGBA grid_color;
  GdkRGBA curve_color;
  GdkRGBA point_fill;
  GdkRGBA point_border;
  GdkRGBA hover_color;
  GdkRGBA select_color;
  GdkRGBA text_color;

  // Cache previous allocation for coordinate mapping
  int last_width;
  int last_height;

  // To-do: offscreen surface caching
  cairo_surface_t *cache_surface;
  gboolean cache_dirty;

} EnergiesCurvesData;

#endif
