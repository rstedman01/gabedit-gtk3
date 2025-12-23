#include "GTK3DrawCompat.h"
#include <string.h>

static GabeditDrawContext *s_ctx = NULL;

void gabedit_draw_context_begin(GabeditDrawContext *ctx, GtkWidget *widget, cairo_t *cr)
{
    ctx->widget = widget;
    ctx->cr = cr;
    ctx->width = gtk_widget_get_allocated_width(widget);
    ctx->height = gtk_widget_get_allocated_height(widget);
    s_ctx = ctx;
}

void gabedit_draw_context_end(GabeditDrawContext* ctx)
{
    (void)ctx;
    s_ctx = NULL;
}

void gdk_draw_line(void* pixmap, void* gc, int x1, int y1, int x2, int y2)
{
    (void)pixmap; (void)gc;
    if(!s_ctx || !s_ctx->cr) return;
    cairo_move_to(s_ctx->cr, x1, y1);
    cairo_line_to(s_ctx->cr, x2, y2);
    cairo_stroke(s_ctx->cr);
}

void gdk_draw_rectangle(void* pixmap, void* gc, gboolean filled, int x, int y, int w, int h)
{
    (void)pixmap; (void)gc;
    if(!s_ctx || !s_ctx->cr) return;
    cairo_rectangle(s_ctx->cr, x, y, w, h);
    if(filled) cairo_fill(s_ctx->cr);
    else       cairo_stroke(s_ctx->cr);
}

void gdk_draw_arc(void* pixmap, void* gc, gboolean filled, int x, int y, int w, int h,
                  int a1_64deg, int a2_64deg)
{
    (void)pixmap; (void)gc;
    if(!s_ctx || !s_ctx->cr) return;

    /* Convert legacy 64ths-of-degree to radians */
    double a1_deg = a1_64deg / 64.0;
    double a2_deg = (a2_64deg) / 64.0;
    double a1_rad = a1_deg * (G_PI/180.0);
    double a2_rad = a2_deg * (G_PI/180.0);

    double cx = x + w/2.0;
    double cy = y + h/2.0;
    double rx = w/2.0;
    double ry = h/2.0;

    /* If ellipse, approximate by scaling */
    cairo_save(s_ctx->cr);
    if (fabs(rx - ry) > 1e-6) {
        cairo_translate(s_ctx->cr, cx, cy);
        cairo_scale(s_ctx->cr, rx, ry);
        cairo_arc(s_ctx->cr, 0, 0, 1.0, a1_rad, a2_rad);
    } else {
        cairo_arc(s_ctx->cr, cx, cy, rx, a1_rad, a2_rad);
    }
    if(filled) cairo_fill(s_ctx->cr); else cairo_stroke(s_ctx->cr);
    cairo_restore(s_ctx->cr);
}

void gabedit_draw_string(GtkWidget* widget, void* pixmap,
                        PangoFontDescription* font_desc, void* gc,
                        int x, int y, const char* text, gboolean centerX, gboolean centerY)
{
    (void)pixmap; (void)gc;
    if(!s_ctx || !s_ctx->cr) return;
    GtkWidget* ctx_widget = widget ? widget : s_ctx->widget;
    PangoLayout* layout = gtk_widget_create_pango_layout(ctx_widget, text ? text : "");
    if(font_desc) pango_layout_set_font_description(layout, font_desc);
    int tw, th; pango_layout_get_pixel_size(layout, &tw, &th);
    double dx = centerX ? (double)tw / 2.0 : 0.0;
    double dy = centerY ? (double)th / 2.0 : 0.0;
    cairo_move_to(s_ctx->cr, x - dx, y - dy);
    pango_cairo_show_layout(s_ctx->cr, layout);
    g_object_unref(layout);
}