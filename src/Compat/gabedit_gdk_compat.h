#pragma once
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <pango/pangocairo.h>
#include "../../gabeditGTK3compat.h"

#ifndef GABEDIT_GDK_COMPAT_H
#define GABEDIT_GDK_COMPAT_H

// Minimal GdkGC values & mask for calls in this codebase
typedef struct {
    GdkColor     foreground;
    GdkColor     background;
    gint         line_width;
    gint         line_style; // GDK_LINE_*
    gint         cap_style; // GDK_CAP_*
    gint         join_style; // GDK_JOIN_*
} GdkGCValues;

typedef guint GdkGCValuesMask;
#ifndef GDK_GC_FOREGROUND
#define GDK_GC_FOREGROUND   (1u<<0)
#endif
#ifndef GDK_GC_BACKGROUND
#define GDK_GC_BACKGROUND   (1u<<1)
#endif
#ifndef GDK_GC_LINE_WIDTH
#define GDK_GC_LINE_WIDTH   (1u<<2)
#endif
#ifndef GDK_GC_LINE_STYLE
#define GDK_GC_LINE_STYLE   (1u<<3)
#endif
#ifndef GDK_GC_CAP_STYLE
#define GDK_GC_CAP_STYLE    (1u<<4)
#endif
#ifndef GDK_GC_JOIN_STYLE
#define GDK_GC_JOIN_STYLE   (1u<<5)
#endif
#ifndef gtk_object_destroy
#define gtk_object_destroy(obj) gtk_widget_destroy(GTK_WIDGET(obj))
#endif

// GdkColormap placeholder type for legacy code
typedef struct _GabeditColormapPlaceholder { int unused; } GdkColormap;

// Lightweight replacement for GdkGC
typedef struct {
    GdkWindow* window;
    GdkGCValues values;
} GdkGC;

// GC API
GdkGC* gdk_gc_new(GdkWindow* window);
GdkGC* gdk_gc_new_with_values(GdkWindow* window, const GdkGCValues* values, GdkGCValuesMask mask);
void gdk_gc_unref(GdkGC* gc);

void gdk_gc_get_values(GdkGC* gc, GdkGCValues* values);
void gdk_gc_set_foreground(GdkGC* gc, const GdkColor* color);
void gdk_gc_set_rgb_fg_color(GdkGC* gc, const GdkColor* color);
void gdk_gc_set_line_attributes(GdkGC* gc, gint line_width, gint line_style, gint cap_style, gint join_style);
void gdk_gc_set_fill(GdkGC* gc, gint fill_style);

// Drawing API (attempt to use gdk_cairo_create)
void gdk_draw_line(GdkWindow* window, GdkGC* gc, gint x1, gint y1, gint x2, gint y2);
void gdk_draw_rectangle(GdkWindow* window, GdkGC* gc, gboolean filled, gint x, gint y, gint width, gint height);
void gdk_draw_polygon(GdkWindow* window, GdkGC* gc, gboolean filled, const GdkPoint* points, gint n_points);
void gdk_draw_arc(GdkWindow* window, GdkGC* gc, gboolean filled, gint x, gint y, gint width, gint height, gint angle1_64, gint angle2_64);
void gdk_draw_drawable(GdkWindow* window, GdkGC* gc, GdkPixbuf* src, gint src_x, gint src_y, gint dest_x, gint dest_y, gint width, gint height);
void gdk_draw_layout(GdkWindow* window, GdkGC* gc, gint x, gint y, PangoLayout* layout);

// Pixmap/XPM helpers mapped to GdkPixbuf
typedef struct _GabeditBitmapPlaceholder { int unused; } GdkBitmap;
GdkPixbuf* gdk_pixmap_new(GdkWindow* window, gint width, gint height, gint depth);
GdkPixbuf* gdk_pixmap_create_from_xpm_d(GdkWindow* window, GdkBitmap** mask, const GdkColor* transparent_color, const char** xpm_data);

// Colormap shim for removed GTK2 API
GdkColormap* gdk_window_get_colormap(GdkWindow* window);
void gdk_colormap_query_color(GdkColormap* cmap, guint32 pixel, GdkColor* out);

// Extremely old helper used few places, return NULL (colormaps removed in GTK3)
static inline gpointer gdk_drawable_get_colormap(GdkWindow* window)
{
    (void)window;
    return NULL;
}

// Utility converters
static inline void gabedit_color_to_rgba(const GdkColor* c, double* r, double* g, double* b, double* a) 
{
    *r = (c->red   / 65535.0);
    *g = (c->green / 65535.0);
    *b = (c->blue  / 65535.0);
    *a = 1.0;
}

static inline GdkWindow* gabedit_widget_get_window(GtkWidget *w) 
{
    return GTK_IS_WIDGET(w) ? gtk_widget_get_window(GTK_WIDGET(w)) : NULL;
}
static inline void gabedit_widget_get_allocation(GtkWidget *w, GtkAllocation *alloc) 
{
    if (GTK_IS_WIDGET(w)) gtk_widget_get_allocation(GTK_WIDGET(w), alloc);
    else { alloc->x = alloc->y = alloc->width = alloc->height = 0; }
}

static inline gboolean gabedit_parse_color_as_gdkcolor(const gchar *name, GdkColor *out) {
    GdkRGBA rgba;
    if (!gdk_rgba_parse(&rgba, name)) return FALSE;
    out->red   = (gushort)(rgba.red   * 65535.0);
    out->green = (gushort)(rgba.green * 65535.0);
    out->blue  = (gushort)(rgba.blue  * 65535.0);
    return TRUE;
}

static inline void gdk_rgba_to_gdk_color(const GdkRGBA *rgba, GdkColor *out)
{
    out->red   = (gushort)(rgba->red   * 65535.0);
    out->green = (gushort)(rgba->green * 65535.0);
    out->blue  = (gushort)(rgba->blue  * 65535.0);
}

static inline void gdk_color_to_gdk_rgba(const GdkColor *c, GdkRGBA *out)
{
    out->red   = c->red   / 65535.0;
    out->green = c->green / 65535.0;
    out->blue  = c->blue  / 65535.0;
    out->alpha = 1.0;
}

#ifndef gtk_object_destroy
#define gtk_object_destroy(obj) gtk_widget_destroy(GTK_WIDGET(obj))
#endif
#ifndef GDK_DRAWABLE
typedef GdkWindow GdkDrawable;
#endif

#endif