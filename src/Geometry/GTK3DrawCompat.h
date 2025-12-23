#ifndef __GABEDIT_GTK3_DRAW_COMPAT_H__
#define __GABEDIT_GTK3_DRAW_COMPAT_H__

/****************************************************
 * GTK3DrawCompat.h
 * 
 * Compatibility layer to ease GTK2->GTK3 migration.
 * - Provides begin/end context functions utilizing
 *   cairo_t and GtkWidget to ensure compatibility
 *   with GTK3 drawing
 * - Implement legacy-style wrappers for gdk_draw_* 
 *   that accepts legacy parameters and translates
 *   to work with Cairo
 * - Provides gabedit_draw_string that uses Pango &
 *   Cairo
 * - Defines convenience macros
 * Usage:
 *   In draw handler:
 *     GabeditDrawContext ctx;
 *     gabedit_draw_context_begin(&ctx, drawing_area_widget, cr);
 *     // legacy drawing code that calls gdk_draw_line, gdk_draw_rectangle, ...
 *     gabedit_draw_context_end(&ctx);
*****************************************************/

#include <gtk/gtk.h>
#include <cairo.h>
#include <pango/pangocairo.h>
#include <math.h>

typedef struct {
    GtkWidget *widget;
    cairo_t *cr;
    int width;
    int height;
} GabeditDrawContext;


// Context management
void gabedit_draw_context_begin(GabeditDrawContext *ctx, GtkWidget *widget, cairo_t *cr);
void gabedit_draw_context_end(GabeditDrawContext *ctx);

// Legacy style drawing wrappers
void gdk_draw_line(void* pixmap, void* gc, int x1, int y1, int x2, int y2);
void gdk_draw_rectangle(void* pixmap, void* gc, gboolean filled, int x, int y, int w, int h);
void gdk_draw_arc(void* pixmap, void* gc, gboolean filled, int x, int y, int w, int h,
                  int a1_64deg, int a2_64deg);

// Legacy text helper 
void gabedit_draw_string(GtkWidget* widget, void* pixmap,
                        PangoFontDescription* font_desc, void* gc,
                        int x, int y, const char* text, gboolean centerX, gboolean centerY);

// Convenience getters that replace widget->allocation and widget->window
static inline int gabedit_widget_width(GtkWidget* w)  { return gtk_widget_get_allocated_width(w); }
static inline int gabedit_widget_height(GtkWidget* w) { return gtk_widget_get_allocated_height(w); }
static inline GdkWindow* gabedit_widget_window(GtkWidget* w) { return gtk_widget_get_window(w); }

#ifndef GDK_Shift_L
#define GDK_Shift_L     GDK_KEY_Shift_L
#endif
#ifndef GDK_Shift_R
#define GDK_Shift_R     GDK_KEY_Shift_R
#endif
#ifndef GDK_Control_L
#define GDK_Control_L   GDK_KEY_Control_L
#endif
#ifndef GDK_Control_R
#define GDK_Control_R   GDK_KEY_Control_R
#endif
#ifndef GDK_Alt_L
#define GDK_Alt_L       GDK_KEY_Alt_L
#endif
#ifndef GDK_Alt_R
#define GDK_Alt_R       GDK_KEY_Alt_R
#endif
#ifndef GDK_rightarrow
#define GDK_rightarrow  GDK_KEY_Right
#endif
#ifndef GDK_leftarrow
#define GDK_leftarrow   GDK_KEY_Left
#endif
#ifndef GDK_uparrow
#define GDK_uparrow     GDK_KEY_Up
#endif
#ifndef GDK_downarrow
#define GDK_downarrow   GDK_KEY_Down
#endif

/* Stipple / solid helpers (replace GDK_STIPPLED / GDK_SOLID usage) */
static inline void gabedit_set_solid(cairo_t* cr)  { cairo_set_dash(cr, NULL, 0, 0.0); }
static inline void gabedit_set_stippled(cairo_t* cr){ double dashes[] = {4.0,4.0}; cairo_set_dash(cr, dashes, 2, 0.0); }

/* Provide small helpers for color conversion convenience */
static inline void gabedit_cairo_set_rgba(cairo_t* cr, const GdkRGBA* c) {
    cairo_set_source_rgba(cr, c->red, c->green, c->blue, c->alpha);
}

#endif