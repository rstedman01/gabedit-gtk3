/* EnergiesCurve.c */
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
#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>

#include "../Common/Global.h"
#include "ResultsAnalise.h"
#include "../Utils/UtilsInterface.h"
#include "../Geometry/GeomGlobal.h"
#include "../Utils/AtomsProp.h"
#include "../Utils/Utils.h"
#include "../Geometry/EnergiesCurves.h"
#include "../Geometry/GeomXYZ.h"
#include "../Common/Windows.h"
#include "../Geometry/Fragments.h"
#include "../Geometry/DrawGeom.h"

#define Nmax 10
/* extern of .h */
gboolean GeomConvIsOpen;


gboolean DrawEnergies(GtkWidget *dessin,GdkEventConfigure *ev);
gboolean grille(GtkWidget *dessin,GdkEventConfigure *ev);
void set_point(GtkWidget *widget, gint x,gint y, gint k);
gint get_coord(gint len,gdouble min, gdouble max,gdouble v,gboolean renv);
void get_coord_ecran(GtkWidget *widget,gint *tabx,gint *taby);
static void set_geom(GtkWidget *widget,gpointer data);

/*****************************************************/
/********************* Utilities *********************/
/*****************************************************/

static EnergiesCurvesData *energies_curves_get_data(GtkWidget *widget)
{
	return (EnergiesCurvesData*)g_object_get_data(G_OBJECT(widget), "EnergiesCurvesData");
}

static double clampd(double v, double a, double b)
{
	return v < a ? a : (v > b ? b : v);
}

static void energies_curves_data_to_pixel(EnergiesCurvesData *d, double xv, double yv, int width, int height, int *px, int* py)
{
	if(d->xmax == d->xmin) { *px = width/2; }
    else {
        double rx = (xv - d->xmin) / (d->xmax - d->xmin);
        *px = (int)round((0.10 + 0.80 * rx) * width);
	}
	if(d->ymax == d->ymin) { *py = height/2; }
    else {
        double ry = (yv - d->ymin) / (d->ymax - d->ymin);
		*py = (int)round((0.90 - 0.80 * ry) * height);
	}
}

static void energies_curves_pixel_to_data(EnergiesCurvesData* d, int px, int py,
                                          int width, int height,
                                          double *xv, double *yv)
{
    double rx = ((double)px / (double)width - 0.10) / 0.80;
    double ry = (0.90 - (double)py / (double)height) / 0.80;
    *xv = d->xmin + rx * (d->xmax - d->xmin);
    *yv = d->ymin + ry * (d->ymax - d->ymin);
}

static void energies_curves_autorange(EnergiesCurvesData* d)
{
    if(d->n <= 0) {
        d->xmin = d->xmax = 0.0;
        d->ymin = d->ymax = 0.0;
        return;
    }
    d->xmin = d->xmax = d->x[0];
    d->ymin = d->ymax = d->y[0];
    for(int i=1;i<d->n;i++) {
        if(d->x[i] < d->xmin) d->xmin = d->x[i];
        if(d->x[i] > d->xmax) d->xmax = d->x[i];
        if(d->y[i] < d->ymin) d->ymin = d->y[i];
        if(d->y[i] > d->ymax) d->ymax = d->y[i];
    }
    double dx = (d->xmax - d->xmin) * 0.05;
    double dy = (d->ymax - d->ymin) * 0.05;
    if(dx == 0.0) dx = 1.0;
    if(dy == 0.0) dy = 1.0;
    d->xmin -= dx; d->xmax += dx;
    d->ymin -= dy; d->ymax += dy;
}
static int energies_curves_hit_point(EnergiesCurvesData* d, int mx, int my, int width, int height)
{
    if(!d || d->n <= 0) return -1;
    const int radius = 6; /* detection radius */
    for(int i=0;i<d->n;i++) {
        int px, py;
        energies_curves_data_to_pixel(d, d->x[i], d->y[i], width, height, &px, &py);
        int dx = mx - px;
        int dy = my - py;
        if(dx*dx + dy*dy <= radius*radius) return i;
    }
    return -1;
}

/*****************************************************/
/********************* Rendering *********************/
/*****************************************************/

static void energies_curves_draw_grid(EnergiesCurvesData* d, cairo_t* cr, GtkWidget *widget, int width, int height)
{
    if(!d->show_grid) return;

    cairo_set_source_rgba(cr, d->grid_color.red, d->grid_color.green,
                          d->grid_color.blue, d->grid_color.alpha);
    cairo_set_line_width(cr, 1.0);

    int nDiv = 10;
    for(int i=0;i<=nDiv;i++){
        double rx = 0.10 + 0.80 * ((double)i / nDiv);
        int x = (int)round(rx * width);
        cairo_move_to(cr, x, (int)(0.10*height));
        cairo_line_to(cr, x, (int)(0.90*height));
    }
    for(int j=0;j<=nDiv;j++){
        double ry = 0.10 + 0.80 * ((double)j / nDiv);
        int y = (int)round(ry * height);
        cairo_move_to(cr, (int)(0.10*width), y);
        cairo_line_to(cr, (int)(0.90*width), y);
    }
    cairo_stroke(cr);
    
    cairo_set_source_rgba(cr, d->text_color.red, d->text_color.green, d->text_color.blue, d->text_color.alpha);

	PangoLayout* layout = gtk_widget_create_pango_layout(widget, "");

    for(int i=0;i<=nDiv;i++){
        double xv = d->xmin + (double)i*(d->xmax - d->xmin)/nDiv;
        gchar* s = g_strdup_printf("%.2f", xv);
        pango_layout_set_text(layout, s, -1);
        int tw, th; pango_layout_get_pixel_size(layout, &tw, &th);
        double rx = 0.10 + 0.80 * ((double)i / nDiv);
        int x = (int)round(rx * width);
        cairo_move_to(cr, x - tw/2, (int)(0.92*height));
        pango_cairo_show_layout(cr, layout);
        g_free(s);
    }
    for(int j=0;j<=nDiv;j++){
        double yv = d->ymin + (double)j*(d->ymax - d->ymin)/nDiv;
        gchar* s = g_strdup_printf("%.2f", yv);
        pango_layout_set_text(layout, s, -1);
        int tw, th; pango_layout_get_pixel_size(layout, &tw, &th);
        double ry = 0.10 + 0.80 * ((double)j / nDiv);
        int y = (int)round(ry * height);
        cairo_move_to(cr, (int)(0.08*width) - tw, y - th/2);
        pango_cairo_show_layout(cr, layout);
        g_free(s);
    }
    g_object_unref(layout);
}

static void energies_curves_draw_curve(EnergiesCurvesData* d, cairo_t* cr,
                                       int width, int height)
{
    if(!d->show_curve || d->n < 2) return;

    cairo_set_source_rgba(cr, d->curve_color.red, d->curve_color.green,
                          d->curve_color.blue, d->curve_color.alpha);
    cairo_set_line_width(cr, 1.5);

    int px, py;
    energies_curves_data_to_pixel(d, d->x[0], d->y[0], width, height, &px, &py);
    cairo_move_to(cr, px, py);
    for(int i=1;i<d->n;i++){
        energies_curves_data_to_pixel(d, d->x[i], d->y[i], width, height, &px, &py);
        cairo_line_to(cr, px, py);
    }
    cairo_stroke(cr);
}

static void energies_curves_draw_points(EnergiesCurvesData* d, cairo_t* cr,
                                        int width, int height)
{
    if(!d->show_points || d->n <= 0) return;
    const double rNorm = 4.0;

    for(int i=0;i<d->n;i++){
        int px, py;
        energies_curves_data_to_pixel(d, d->x[i], d->y[i], width, height, &px, &py);

        gboolean isHover = (i == d->hover_index);
        gboolean isSel   = (i == d->selected_index);

        double r = rNorm;
        GdkRGBA fill = d->point_fill;
        GdkRGBA border = d->point_border;

        if(isSel){
            r = rNorm + 2.0;
            fill = d->select_color;
        }
        else if(isHover){
            r = rNorm + 1.0;
            fill = d->hover_color;
        }

        cairo_set_source_rgba(cr, fill.red, fill.green, fill.blue, fill.alpha);
        cairo_arc(cr, px, py, r, 0, 2*G_PI);
        cairo_fill_preserve(cr);

        cairo_set_source_rgba(cr, border.red, border.green, border.blue, border.alpha);
        cairo_set_line_width(cr, 1.0);
        cairo_stroke(cr);
    }
}


static void energies_curves_draw_title(EnergiesCurvesData* d, cairo_t* cr,
                                       GtkWidget* widget, int width, int height)
{
    if(!d->show_title || !d->title) return;
    cairo_set_source_rgba(cr, d->text_color.red, d->text_color.green,
                          d->text_color.blue, d->text_color.alpha);

    PangoLayout* layout = gtk_widget_create_pango_layout(widget, d->title);
    int tw, th; pango_layout_get_pixel_size(layout, &tw, &th);
    cairo_move_to(cr, (width - tw)/2.0, 0.02*height);
    pango_cairo_show_layout(cr, layout);
    g_object_unref(layout);
}

// Primary draw handler
static gboolean energies_curves_on_draw(GtkWidget* widget, cairo_t* cr, gpointer user_data)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return FALSE;

    int width  = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);
    d->last_width = width;
    d->last_height = height;

    cairo_set_source_rgba(cr, d->bg_color.red, d->bg_color.green, d->bg_color.blue, d->bg_color.alpha);
    cairo_rectangle(cr,0,0,width,height);
    cairo_fill(cr);

    energies_curves_draw_grid(d, cr, widget, width, height);
    energies_curves_draw_curve(d, cr, width, height);
    energies_curves_draw_points(d, cr, width, height);
    energies_curves_draw_title(d, cr, widget, width, height);

    return TRUE;
}

/*****************************************************/
/******************* Event Handling ******************/
/*****************************************************/

static gboolean energies_curves_on_button_press(GtkWidget* widget, GdkEventButton* event, gpointer user_data)
{
    if(event->button != 1) return FALSE;
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return FALSE;

    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    int hit = energies_curves_hit_point(d, (int)event->x, (int)event->y, w, h);
    if(hit >= 0){
        d->selected_index = hit;
        gtk_widget_queue_draw(widget);
    }
    return TRUE;
}

static gboolean energies_curves_on_motion(GtkWidget* widget, GdkEventMotion* event, gpointer user_data)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return FALSE;
    int w = gtk_widget_get_allocated_width(widget);
    int h = gtk_widget_get_allocated_height(widget);
    int hit = energies_curves_hit_point(d, (int)event->x, (int)event->y, w, h);
    if(hit != d->hover_index){
        d->hover_index = hit;
        gtk_widget_queue_draw(widget);
    }
    return TRUE;
}

static gboolean energies_curves_on_leave(GtkWidget* widget, GdkEventCrossing* event, gpointer user_data)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return FALSE;
    if(d->hover_index != -1){
        d->hover_index = -1;
        gtk_widget_queue_draw(widget);
    }
    return TRUE;
}

static gboolean energies_curves_on_key(GtkWidget* widget, GdkEventKey* event, gpointer user_data)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return FALSE;

    switch(event->keyval){
        case GDK_KEY_Left:
            if(d->selected_index > 0) d->selected_index--;
            gtk_widget_queue_draw(widget);
            break;
        case GDK_KEY_Right:
            if(d->selected_index < d->n - 1) d->selected_index++;
            gtk_widget_queue_draw(widget);
            break;
        case GDK_KEY_Escape:
            d->selected_index = -1;
            gtk_widget_queue_draw(widget);
            break;
        default: break;
    }
    return TRUE;
}

/*****************************************************/
/********************* Public API ********************/
/*****************************************************/

GtkWidget* energies_curves_new(void)
{
    GtkWidget* area = gtk_drawing_area_new();
    EnergiesCurvesData* d = g_malloc0(sizeof(EnergiesCurvesData));

    d->x = NULL;
    d->y = NULL;
    d->n = 0;
    d->title  = g_strdup("Energies");
    d->xlabel = g_strdup("X");
    d->ylabel = g_strdup("Energy");

    d->hover_index    = -1;
    d->selected_index = -1;

    d->show_grid   = TRUE;
    d->show_points = TRUE;
    d->show_curve  = TRUE;
    d->show_title  = TRUE;

    d->bg_color      = (GdkRGBA){1.0,1.0,1.0,1.0};
    d->grid_color    = (GdkRGBA){0.85,0.85,0.85,1.0};
    d->curve_color   = (GdkRGBA){0.10,0.10,0.10,1.0};
    d->point_fill    = (GdkRGBA){0.15,0.15,0.70,1.0};
    d->point_border  = (GdkRGBA){0.0,0.0,0.0,1.0};
    d->hover_color   = (GdkRGBA){0.90,0.40,0.10,1.0};
    d->select_color  = (GdkRGBA){0.20,0.80,0.20,1.0};
    d->text_color    = (GdkRGBA){0.0,0.0,0.0,1.0};

    d->cache_surface = NULL;
    d->cache_dirty   = TRUE;

    g_object_set_data_full(G_OBJECT(area), "EnergiesCurvesData", d,
                           (GDestroyNotify)g_free);

    /* Events */
    gtk_widget_add_events(area,
        GDK_BUTTON_PRESS_MASK |
        GDK_POINTER_MOTION_MASK |
        GDK_LEAVE_NOTIFY_MASK |
        GDK_KEY_PRESS_MASK);

    g_signal_connect(area,"draw",          G_CALLBACK(energies_curves_on_draw), NULL);
    g_signal_connect(area,"button-press-event", G_CALLBACK(energies_curves_on_button_press), NULL);
    g_signal_connect(area,"motion-notify-event", G_CALLBACK(energies_curves_on_motion), NULL);
    g_signal_connect(area,"leave-notify-event",  G_CALLBACK(energies_curves_on_leave), NULL);
    g_signal_connect(area,"key-press-event",     G_CALLBACK(energies_curves_on_key), NULL);

    gtk_widget_set_can_focus(area, TRUE);

    return area;
}

/* Set data arrays (copies or takes ownership based on copy flag) */
void energies_curves_set_points(GtkWidget* widget,
                                const double* xs, const double* ys, int n,
                                gboolean copy_data)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return;

    /* Free old */
    if(d->x) g_free(d->x);
    if(d->y) g_free(d->y);
    d->x = d->y = NULL;
    d->n = 0;

    if(n > 0){
        if(copy_data){
            d->x = g_malloc(sizeof(double)*n);
            d->y = g_malloc(sizeof(double)*n);
            memcpy(d->x, xs, sizeof(double)*n);
            memcpy(d->y, ys, sizeof(double)*n);
        } else {
            /* Cast away const (ensure caller will not free until replaced) */
            d->x = (double*)xs;
            d->y = (double*)ys;
        }
        d->n = n;
    }

    energies_curves_autorange(d);
    d->cache_dirty = TRUE;
    gtk_widget_queue_draw(widget);
}

void energies_curves_set_title(GtkWidget* widget, const gchar* title)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return;
    if(d->title) g_free(d->title);
    d->title = g_strdup(title ? title : "");
    gtk_widget_queue_draw(widget);
}

void energies_curves_set_show_grid(GtkWidget* widget, gboolean show)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return;
    d->show_grid = show;
    gtk_widget_queue_draw(widget);
}

void energies_curves_set_colors(GtkWidget* widget,
                                const GdkRGBA* bg,
                                const GdkRGBA* curve,
                                const GdkRGBA* point)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return;
    if(bg)    d->bg_color = *bg;
    if(curve) d->curve_color = *curve;
    if(point) d->point_fill = *point;
    gtk_widget_queue_draw(widget);
}

/* Retrieve selected point index */
int energies_curves_get_selected_index(GtkWidget* widget)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return -1;
    return d->selected_index;
}

/* Get coordinates of selected point (returns FALSE if none) */
gboolean energies_curves_get_selected_point(GtkWidget* widget, double* xv, double* yv)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d || d->selected_index < 0 || d->selected_index >= d->n) return FALSE;
    if(xv) *xv = d->x[d->selected_index];
    if(yv) *yv = d->y[d->selected_index];
    return TRUE;
}

/* Clear selection */
void energies_curves_clear_selection(GtkWidget* widget)
{
    EnergiesCurvesData* d = energies_curves_get_data(widget);
    if(!d) return;
    d->selected_index = -1;
    gtk_widget_queue_draw(widget);
}

/* Convenience factory for quick test */
GtkWidget* energies_curves_new_with_data(const double* xs, const double* ys, int n)
{
    GtkWidget* w = energies_curves_new();
    energies_curves_set_points(w, xs, ys, n, TRUE);
    return w;
}