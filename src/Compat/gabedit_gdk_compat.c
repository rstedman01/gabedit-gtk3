#include <math.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <pango/pangocairo.h>
#include "gabedit_gdk_compat.h"

static void apply_gc_to_cr(GdkGC* gc, cairo_t* cr)
{
    double r,g,b,a;
    gabedit_color_to_rgba(&gc->values.foreground, &r, &g, &b, &a);
    cairo_set_source_rgba(cr, r, g, b, a);

    if (gc->values.line_width > 0)
    {
        cairo_set_line_width(cr, gc->values.line_width);
    }

    // Line style -> dash pattern
    if (gc -> values.line_style == GDK_LINE_SOLID)
    {
        cairo_set_dash(cr, NULL, 0, 0);
    } else if (gc -> values.line_style == GDK_LINE_ON_OFF_DASH)
    {
        double dashes[] = {6.0, 6.0};
        cairo_set_dash(cr, dashes, 2, 0);
    } else if (gc -> values.line_style == GDK_LINE_DOUBLE_DASH)
    {
        double dashes[] = {2.0, 2.0};
        cairo_set_dash(cr, dashes, 2, 0);
    }

    // Caps and joins
    cairo_line_cap_t cap = CAIRO_LINE_CAP_BUTT;
    if (gc->values.cap_style == GDK_CAP_ROUND) cap = CAIRO_LINE_CAP_ROUND;
    else if (gc->values.cap_style == GDK_CAP_PROJECTING) cap = CAIRO_LINE_CAP_SQUARE;
    cairo_set_line_cap(cr, cap);

    cairo_line_join_t join = CAIRO_LINE_JOIN_MITER;
    if (gc->values.join_style == GDK_JOIN_ROUND) join = CAIRO_LINE_JOIN_ROUND;
    else if (gc->values.join_style == GDK_JOIN_BEVEL) join = CAIRO_LINE_JOIN_BEVEL;
    cairo_set_line_join(cr, join);
}

/***************************************/
/***************** GC ******************/
/***************************************/
GdkGC* gdk_gc_new(GdkWindow* window)
{
    GdkGC* gc = g_new0(GdkGC, 1);
    gc->window = window;
    // Default = black, 1px, solid
    gc->values.foreground.pixel = 0;
    gc->values.foreground.red = 0;
    gc->values.foreground.green = 0;
    gc->values.foreground.blue = 0;
    gc->values.line_width = 1;
    gc->values.line_style = GDK_LINE_SOLID;
    gc->values.cap_style  = GDK_CAP_BUTT;
    gc->values.join_style = GDK_JOIN_MITER;
    return gc;
}

GdkGC* gdk_gc_new_with_values(GdkWindow* window, const GdkGCValues* values, GdkGCValuesMask mask) 
{
    GdkGC* gc = gdk_gc_new(window);
    if (values) {
        if (mask & GDK_GC_FOREGROUND) gc->values.foreground = values->foreground;
        if (mask & GDK_GC_BACKGROUND) gc->values.background = values->background;
        if (mask & GDK_GC_LINE_WIDTH) gc->values.line_width = values->line_width;
        if (mask & GDK_GC_LINE_STYLE) gc->values.line_style = values->line_style;
        if (mask & GDK_GC_CAP_STYLE)  gc->values.cap_style  = values->cap_style;
        if (mask & GDK_GC_JOIN_STYLE) gc->values.join_style = values->join_style;
    }
    return gc;
}

void gdk_gc_unref(GdkGC* gc)
{
    if (gc) g_free(gc);
}

void gdk_gc_get_values(GdkGC* gc, GdkGCValues* values)
{
    if (!gc || !values) return;
    *values = gc->values;
}

void gdk_gc_set_foreground(GdkGC* gc, const GdkColor* color)
{
    if (!gc || !color) return;
    gc->values.foreground = *color;
}

void gdk_gc_set_rgb_fg_color(GdkGC* gc, const GdkColor* color)
{
    gdk_gc_set_foreground(gc, color);
}

void gdk_gc_set_line_attributes(GdkGC* gc, gint line_width, gint line_style, gint cap_style, gint join_style)
{
    if (!gc) return;
    if (line_width > 0) gc -> values.line_width = line_width;
    gc -> values.line_style = line_style;
    gc -> values.cap_style = cap_style;
    gc -> values.join_style = join_style;
}

void gdk_gc_set_fill(GdkGC* gc, gint fill_style)
{
    (void)gc; (void)fill_style;
}

/**************************************/
/************   Drawing   *************/
/**************************************/

void gdk_draw_line(GdkWindow* window, GdkGC* gc, gint x1, gint y1, gint x2, gint y2)
{
    if (!window || !gc) return;
    cairo_t* cr = gdk_cairo_create(window);
    apply_gc_to_cr(gc, cr);
    cairo_move_to(cr, x1+0.5, y1+0.5);
    cairo_line_to(cr, x2+0.5, y2+0.5);
    cairo_stroke(cr);
    cairo_destroy(cr);
}
void gdk_draw_rectangle(GdkWindow* window, GdkGC* gc, gboolean filled, gint x, gint y, gint width, gint height) {
    if (!window || !gc) return;
    cairo_t* cr = gdk_cairo_create(window);
    apply_gc_to_cr(gc, cr);
    cairo_rectangle(cr, x, y, width, height);
    if (filled) cairo_fill(cr);
    else cairo_stroke(cr);
    cairo_destroy(cr);
}

void gdk_draw_polygon(GdkWindow* window, GdkGC* gc, gboolean filled, const GdkPoint* points, gint n_points) {
    if (!window || !gc || !points || n_points <= 0) return;
    cairo_t* cr = gdk_cairo_create(window);
    apply_gc_to_cr(gc, cr);
    cairo_move_to(cr, points[0].x, points[0].y);
    for (gint i = 1; i < n_points; i++)
        cairo_line_to(cr, points[i].x, points[i].y);
    cairo_close_path(cr);
    if (filled) cairo_fill(cr);
    else cairo_stroke(cr);
    cairo_destroy(cr);
}

static inline double deg64_to_rad(gint d64) {
    return (d64/64.0) * (G_PI/180.0);
}

void gdk_draw_arc(GdkWindow* window, GdkGC* gc, gboolean filled, gint x, gint y, gint width, gint height, gint angle1_64, gint angle2_64) {
    if (!window || !gc) return;
    cairo_t* cr = gdk_cairo_create(window);
    apply_gc_to_cr(gc, cr);
    double rx = width/2.0;
    double ry = height/2.0;
    double cx = x + rx;
    double cy = y + ry;
    cairo_save(cr);
    cairo_translate(cr, cx, cy);
    cairo_scale(cr, rx, ry);
    double a1 = deg64_to_rad(angle1_64);
    double a2 = deg64_to_rad(angle1_64 + angle2_64);
    cairo_arc(cr, 0, 0, 1.0, a1, a2);
    if (filled) cairo_fill(cr);
    else cairo_stroke(cr);
    cairo_restore(cr);
    cairo_destroy(cr);
}

void gdk_draw_drawable(GdkWindow* window, GdkGC* gc, GdkPixbuf* src, gint src_x, gint src_y, gint dest_x, gint dest_y, gint width, gint height) {
    if (!window || !src) return;
    cairo_t* cr = gdk_cairo_create(window);
    gdk_cairo_set_source_pixbuf(cr, src, dest_x - src_x, dest_y - src_y);
    cairo_rectangle(cr, dest_x, dest_y, width, height);
    cairo_fill(cr);
    cairo_destroy(cr);
    (void)gc; 
}

void gdk_draw_layout(GdkWindow* window, GdkGC* gc, gint x, gint y, PangoLayout* layout) {
    if (!window || !gc || !layout) return;
    cairo_t* cr = gdk_cairo_create(window);
    apply_gc_to_cr(gc, cr);
    cairo_move_to(cr, x, y);
    pango_cairo_show_layout(cr, layout);
    cairo_destroy(cr);
}

/**************************************/
/**********   Pixmaps/XPM   ***********/
/**************************************/

GdkPixbuf* gdk_pixmap_new(GdkWindow* window, gint width, gint height, gint depth)
{
    (void)window; (void)depth;
    return gdk_pixbuf_new(GDK_COLORSPACE_RGB, TRUE, 8, width, height);
}

GdkPixbuf* gdk_pixmap_create_from_xpm_d(GdkWindow* window, GdkBitmap** mask, const GdkColor* transparent_color, const char** xpm_data) {
    (void)window; (void)transparent_color;
    GdkPixbuf* pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)xpm_data);
    if (mask) *mask = NULL;
    return pixbuf;
}

/**************************************/
/*************   Color   **************/
/**************************************/

GdkColormap* gdk_window_get_colormap(GdkWindow* window)
{
    (void)window;
    static GdkColormap dummy;
    return &dummy;
}

void gdk_colormap_query_color(GdkColormap* cmap, guint32 pixel, GdkColor* out)
{
    (void)cmap;
    if (!out) return;
    guint32 p = pixel & 0x00FFFFFFu;
    guint8 r8 = (p >> 16) & 0xFF;
    guint8 g8 = (p >> 16) & 0xFF;
    guint8 b8 = (p >> 16) & 0xFF;
    out->pixel = pixel;
    out->red = (guint16)(r8 * 257u);
    out->green = (guint16)(r8 * 257u);
    out->blue = (guint16)(b8 * 257u);
}