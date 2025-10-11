// GTK-3.0 compatibility file

#ifndef GABEDIT_GTK3_COMPAT_H
#define GABEDIT_GTK3_COMPAT_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <cairo.h>

// GTK_OBJECT -> G_OBJECT
#ifndef GTK_OBJECT
#define GTK_OBJECT(o) (G_OBJECT(o))
#endif

// gtk_signal_connect -> g_signal_connect
#ifndef gtk_signal_connect
#define gtk_signal_connect(object, detailed_signal, c_handler, data) \
        g_signal_connect((object), (detailed_signal), (GCallback)(c_handler), (data))
#endif

// GtkObject -> GObject
#ifndef __GABEDIT_HAVE_GTKOBJECT
typedef GObject GtkObject;
#define __GABEDIT_HAVE_GTKOBJECT 1
#endif

// Provide GdkDrawable alias
#ifndef __GABEDIT_HAVE_GDKDRAWABLE
typedef GdkWindow GdkDrawable;
#define __GABEDIT_HAVE_GDKDRAWABLE 1
#endif

// GdkPixmap -> GdkPixbuf alias
#ifndef __GABEDIT_HAVE_GDKPIXMAP
typedef GdkPixbuf GdkPixmap;
#define __GABEDIT_HAVE_GDKPIXMAP 1
#endif

#ifndef __GABEDIT_HAVE_GDKLINESTYLE
typedef enum {
    GDK_LINE_SOLID = 0,
    GDK_LINE_ON_OFF_DASH = 1,
    GDK_LINE_DOUBLE_DASH = 2
} GdkLineStyle;
#define __GABEDIT_HAVE_GDKLINESTYLE 1
#endif

// Account for old cap/join enums
#ifndef GDK_LINE_SOLID
#define GDK_LINE_SOLID 0
#endif 
#ifndef GDK_LINE_ON_OFF_DASH
#define GDK_LINE_ON_OFF_DASH 1
#endif
#ifndef GDK_LINE_DOUBLE_DASH
#define GDK_LINE_DOUBLE_DASH 2
#endif
#ifndef GDK_CAP_NOT_LAST
#define GDK_CAP_NOT_LAST 0
#endif
#ifndef GDK_CAP_BUTT
#define GDK_CAP_BUTT 1
#endif
#ifndef GDK_CAP_ROUND
#define GDK_CAP_ROUND 2
#endif
#ifndef GDK_CAP_PROJECTING
#define GDK_CAP_PROJECTING 3
#endif
#ifndef GDK_JOIN_MITER
#define GDK_JOIN_MITER 0
#endif
#ifndef GDK_JOIN_ROUND
#define GDK_JOIN_ROUND 1
#endif
#ifndef GDK_JOIN_BEVEL
#define GDK_JOIN_BEVEL 2
#endif

// Legacy flags shim
#ifndef GTK_CAN_DEFAULT
#define GTK_CAN_DEFAULT (1u<<7)
#endif

static inline void gabedit_widget_set_flags(GtkWidget* w, unsigned f) {
    if (w && (f == GTK_CAN_DEFAULT)) gtk_widget_set_can_default(w, TRUE);
}
static inline void gabedit_widget_unset_flags(GtkWidget* w, unsigned f) {
    if (w && (f == GTK_CAN_DEFAULT)) gtk_widget_set_can_default(w, FALSE);
}

#ifndef GTK_WIDGET_SET_FLAGS
#define GTK_WIDGET_SET_FLAGS(w, f) gabedit_widget_set_flags(GTK_WIDGET(w), (f))
#endif
#ifndef GTK_WIDGET_UNSET_FLAGS
#define GTK_WIDGET_UNSET_FLAGS(w, f) gabedit_widget_unset_flags(GTK_WIDGET(w), (f))
#endif

// Legacy horizontal separator helper
#ifndef gtk_hseparator_new
#define gtk_hseparator_new() gtk_separator_new(GTK_ORIENTATION_HORIZONTAL)
#endif

// GdkFont -> PangoFontDescription (compile-time stand in)
#ifndef __GABEDIT_HAVE_GDKFONT
typedef PangoFontDescription GdkFont;
#define __GABEDIT_HAVE_GDKFONT 1
#endif

#ifndef gtk_hseparator_new
#define gtk_hseparator_new() gtk_separator_new(GTK_ORIENTATION_HORIZONTAL)
#endif

// VBox/HBox -> GtkBox; preserves orientation, homogeneous & spacing
static inline GtkWidget* gb_vbox_new(gboolean homogeneous, gint spacing)
{
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing);
    gtk_box_set_homogeneous(GTK_BOX(box), homogeneous);
    return box;
}
static inline GtkWidget* gb_hbox_new(gboolean homogeneous, gint spacing)
{
    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
    gtk_box_set_homogeneous(GTK_BOX(box), homogeneous);
    return box;
}

// Table -> GtkGrid; rows/cols maintained for compatibility (unused) 
static inline GtkWidget* gb_table_new(gint rows, gint cols, gboolean homogeneous)
{
    GtkWidget* grid = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), homogeneous);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid), homogeneous);
    (void)rows; (void)cols;
    return grid;
}

// Attach children to grid, translate span, approximate expand/fill
// guint used to avoid GtkAttachOptions clashes
static inline void gb_table_attach(GtkWidget* table,
                                   GtkWidget* child,
                                   gint left, gint right,
                                   gint top,  gint bottom,
                                   guint xopts,
                                   guint yopts,
                                   guint xpad, guint ypad)
{
    gint width  = (right  > left) ? (right  - left) : 1;
    gint height = (bottom > top)  ? (bottom - top)  : 1;

    gtk_grid_attach(GTK_GRID(table), child, left, top, width, height);

    gboolean hexpand = (xopts != 0);
    gboolean vexpand = (yopts != 0);
    gtk_widget_set_hexpand(child, hexpand);
    gtk_widget_set_vexpand(child, vexpand);

    gtk_widget_set_margin_start(child, xpad);
    gtk_widget_set_margin_end(child,   xpad);
    gtk_widget_set_margin_top(child,   ypad);
    gtk_widget_set_margin_bottom(child, ypad);
}

// Spacing helpers
static inline void gb_table_set_row_spacings(GtkWidget* table, guint spacing)
{
    gtk_grid_set_row_spacing(GTK_GRID(table), spacing);
}

static inline void gb_table_set_col_spacings(GtkWidget* table, guint spacing)
{
    gtk_grid_set_column_spacing(GTK_GRID(table), spacing);
}

// Remap macros, keep call sites unchanged
#undef gtk_vbox_new
#undef gtk_hbox_new
#undef gtk_table_new
#undef gtk_table_attach
#undef gtk_table_set_row_spacings
#undef gtk_table_set_col_spacings

#define gtk_vbox_new(h,s)           gb_vbox_new((h),(s))
#define gtk_hbox_new(h,s)           gb_hbox_new((h),(s))
#define gtk_table_new(r,c,h)        gb_table_new((r),(c),(h))
#define gtk_table_attach(t,child,l,r,t1,b,xo,yo,xp,yp) \
        gb_table_attach(GTK_WIDGET(t),(child),(l),(r),(t1),(b),(guint)(xo),(guint)(yo),(xp),(yp))
#define gtk_table_set_row_spacings(t,s) gb_table_set_row_spacings(GTK_WIDGET(t),(s))
#define gtk_table_set_col_spacings(t,s) gb_table_set_col_spacings(GTK_WIDGET(t),(s))

// gtkglext init null until GtkGLArea migrated
#ifndef gtk_gl_init
#define gtk_gl_init(argc, argv) ((void)0)
#endif

#endif /* __GABEDIT_GTK3_COMPAT__ */
