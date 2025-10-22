# Key Changes - Detailed View

## 1. Makefile Fix

```diff
--- a/Makefile
+++ b/Makefile
@@ -1,9 +1,9 @@
 CC ?= gcc
 CFLAGS ?= -std=c17 -O2 -pipe -Wall -Wextra -Wno-deprecated-declarations
-INCDIR ?= $(pkg-config --cflags gtk+-3.0)
-GTK_CFLAGS := $(pkg-config --cflags gtk+-3.0)
-GTK_LIBS   := $(pkg-config --libs gtk+-3.0)
-EPOXY_LIBS := $(pkg-config --libs epoxy)
+INCDIR ?= $(shell pkg-config --cflags gtk+-3.0)
+GTK_CFLAGS := $(shell pkg-config --cflags gtk+-3.0)
+GTK_LIBS   := $(shell pkg-config --libs gtk+-3.0)
+EPOXY_LIBS := $(shell pkg-config --libs epoxy)
```

## 2. Function Signature Updates (Header)

```diff
--- a/src/Utils/UtilsInterface.h
+++ b/src/Utils/UtilsInterface.h
@@ -51,10 +51,12 @@
 void desole();
 void change_insert();
-void new_doc_molpro();
-void new_doc_gauss();
-void new_doc_orca();
-void new_doc_qchem();
+/* GTK3 Migration: Signal handlers must use standard GtkWidget*, gpointer signature */
+void new_doc_molpro(GtkWidget* widget, gpointer data);
+void new_doc_gauss(GtkWidget* widget, gpointer data);
+void new_doc_orca(GtkWidget* widget, gpointer data);
+void new_doc_qchem(GtkWidget* widget, gpointer data);
 void insert_doc_gauss();

@@ -60,7 +62,8 @@
 void show_about();
 void show_version();
-void show_homepage();
+/* GTK3 Migration: Signal handlers must use standard GtkWidget*, gpointer signature */
+void show_homepage(GtkWidget* widget, gpointer data);
```

## 3. G_CONST_RETURN Replacement

```diff
--- a/src/Utils/UtilsInterface.h
+++ b/src/Utils/UtilsInterface.h
@@ -124,7 +127,8 @@
-GtkWidget *add_label_table(GtkWidget *Table, G_CONST_RETURN gchar *label,gushort line,gushort colonne);
+/* GTK3 Migration: G_CONST_RETURN deprecated, use const instead */
+GtkWidget *add_label_table(GtkWidget *Table, const gchar *label,gushort line,gushort colonne);

-void set_last_directory(G_CONST_RETURN gchar* FileName);
+/* GTK3 Migration: G_CONST_RETURN deprecated, use const instead */
+void set_last_directory(const gchar* FileName);
```

## 4. Pixmap to Pixbuf Migration Example

```diff
--- a/src/Utils/UtilsInterface.c
+++ b/src/Utils/UtilsInterface.c
@@ -243,21 +243,26 @@
-/* create_pixmap, convenience function to create a pixmap widget, from data */
+/* GTK3 Migration: GdkPixmap deprecated, use GdkPixbuf instead */
 GtkWidget *create_pixmap(GtkWidget *widget, gchar **data)
 {
-  GtkStyle *style;
-  GdkBitmap *mask;
-  GdkPixmap *gdk_pixmap;
-  GtkWidget *gtk_pixmap;
+  GdkPixbuf *pixbuf;
+  GtkWidget *image;
 
-  style = gtk_widget_get_style(widget);
-  g_assert(style != NULL);
+  (void)widget; /* Unused in GTK3 version */
 
-  gdk_pixmap = gdk_pixmap_create_from_xpm_d(widget->window,&mask, &style->bg[GTK_STATE_NORMAL],data);
-  g_assert(gdk_pixmap != NULL);
-  gtk_pixmap = gtk_image_new_from_pixmap(gdk_pixmap, mask);
+  /* Cast to const char** as required by gdk_pixbuf_new_from_xpm_data */
+  pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)data);
+  g_assert(pixbuf != NULL);
+  
+  image = gtk_image_new_from_pixbuf(pixbuf);
+  g_object_unref(pixbuf);
 
-  g_assert(gtk_pixmap != NULL);
-  gtk_widget_show(gtk_pixmap);
+  g_assert(image != NULL);
+  gtk_widget_show(image);
 
-  return(gtk_pixmap);
+  return(image);
 }
```

## 5. Cairo Integration for Drawing

```diff
--- a/src/Utils/UtilsInterface.c
+++ b/src/Utils/UtilsInterface.c
@@ -265,36 +270,48 @@
-GdkPixmap *get_pixmap(GtkWidget  *widget,gushort red,gushort green,gushort blue)
+/* GTK3 Migration: GdkPixmap, GdkGC, gdk_draw_rectangle removed in GTK3
+ * Use Cairo and GdkPixbuf for drawing
+ */
+GdkPixbuf *get_pixbuf_color(gushort red, gushort green, gushort blue)
 {
-  GdkPixmap *pixmap = NULL;
-  GdkGC *gc = NULL;
   gint width = ScreenHeight/20;
   gint height = ScreenHeight/50;
-  GdkColormap *colormap;
-  GdkColor Color;
-
-  gc = gdk_gc_new(widget->window);
-  Color.red =  red;
-  Color.green =  green;
-  Color.blue =  blue;
-
-  pixmap = gdk_pixmap_new(widget->window,width,height, -1);
-  gdk_draw_rectangle (pixmap,
-                      widget->style->black_gc,
-                      TRUE,
-                      0, 0, width, height);    
-  colormap  = gdk_drawable_get_colormap(widget->window);
-  gdk_colormap_alloc_color(colormap,&Color,FALSE,TRUE);
-  gdk_gc_set_foreground(gc,&Color);
-  gdk_draw_rectangle (pixmap,
-                      gc,
-                      TRUE,
-                      1, 1, width-1, height-1);    
-
-  g_object_unref(gc);
-  return pixmap;
+  cairo_surface_t *surface;
+  cairo_t *cr;
+  GdkPixbuf *pixbuf;
+  
+  /* Create a Cairo surface */
+  surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
+  cr = cairo_create(surface);
+  
+  /* Draw black border */
+  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
+  cairo_rectangle(cr, 0, 0, width, height);
+  cairo_fill(cr);
+  
+  /* Draw colored inner rectangle */
+  cairo_set_source_rgb(cr, red/65535.0, green/65535.0, blue/65535.0);
+  cairo_rectangle(cr, 1, 1, width-1, height-1);
+  cairo_fill(cr);
+  
+  /* Convert Cairo surface to GdkPixbuf */
+  pixbuf = gdk_pixbuf_get_from_surface(surface, 0, 0, width, height);
+  
+  cairo_destroy(cr);
+  cairo_surface_destroy(surface);
+  
+  return pixbuf;
 }
```

## 6. Color Dialog Migration

```diff
--- a/src/Utils/UtilsInterface.c
+++ b/src/Utils/UtilsInterface.c
@@ -3172,58 +3179,74 @@
-static void open_color_dlg_dipole(GtkWidget *button,gpointer data)
+/* GTK3 Migration: GtkColorSelectionDialog replaced with GtkColorChooserDialog */
+static void open_color_dlg_dipole(GtkWidget *button, gpointer data)
 {
-  GtkColorSelectionDialog *colorDlg;
+  (void)data; /* Unused parameter */
+  
+  GtkWidget *colorDlg;
   GtkStyle* style = g_object_get_data(G_OBJECT (button), "Style");
   GtkWidget *win = g_object_get_data(G_OBJECT (button), "Win");
-  GdkColor* color = g_object_get_data(G_OBJECT (button), "Color");;
+  GdkColor* color = g_object_get_data(G_OBJECT (button), "Color");
+  GdkRGBA rgba;
 
-  colorDlg = (GtkColorSelectionDialog *)gtk_color_selection_dialog_new(_("Set Dipole Color"));
-  gtk_color_selection_set_current_color (GTK_COLOR_SELECTION (colorDlg->colorsel), color);
-  gtk_window_set_transient_for(GTK_WINDOW(colorDlg),GTK_WINDOW(win));
-  gtk_window_set_position(GTK_WINDOW(colorDlg),GTK_WIN_POS_CENTER);
-  gtk_window_set_modal (GTK_WINDOW (colorDlg), TRUE);
-  g_signal_connect(G_OBJECT(colorDlg), "delete_event",(GCallback)destroy_button_windows,NULL);
-  g_signal_connect(G_OBJECT(colorDlg), "delete_event",G_CALLBACK(gtk_widget_destroy),NULL);
-
-  g_object_set_data(G_OBJECT (colorDlg->colorsel), "Color", color);
-  gtk_widget_hide(colorDlg->help_button);
-  g_signal_connect_swapped(G_OBJECT(colorDlg->ok_button),"clicked", (GCallback)set_dipole_color,GTK_OBJECT(colorDlg->colorsel));
-
-  g_object_set_data(G_OBJECT (colorDlg->ok_button), "Color", color);
-  g_object_set_data(G_OBJECT (colorDlg->ok_button), "Button", button);
-  g_object_set_data(G_OBJECT (colorDlg->ok_button), "Style", style);
-  g_signal_connect(G_OBJECT(colorDlg->ok_button),"clicked", (GCallback)set_dipole_button_color,NULL);
-
-  g_signal_connect_swapped(G_OBJECT(colorDlg->ok_button), "clicked", (GCallback)destroy_button_windows,GTK_OBJECT(colorDlg));
-  g_signal_connect_swapped(G_OBJECT(colorDlg->ok_button),"clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(colorDlg));
-
-  g_signal_connect_swapped(G_OBJECT(colorDlg->cancel_button), "clicked", (GCallback)destroy_button_windows,GTK_OBJECT(colorDlg));
-  g_signal_connect_swapped(G_OBJECT(colorDlg->cancel_button),"clicked", (GCallback)gtk_widget_destroy,GTK_OBJECT(colorDlg));
-
-  add_button_windows(_(" Set Color "),GTK_WIDGET(colorDlg));
-  gtk_widget_show(GTK_WIDGET(colorDlg));
+  /* Convert GdkColor to GdkRGBA */
+  rgba.red = color->red / 65535.0;
+  rgba.green = color->green / 65535.0;
+  rgba.blue = color->blue / 65535.0;
+  rgba.alpha = 1.0;
+
+  colorDlg = gtk_color_chooser_dialog_new(_("Set Dipole Color"), GTK_WINDOW(win));
+  gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colorDlg), &rgba);
+  gtk_window_set_transient_for(GTK_WINDOW(colorDlg), GTK_WINDOW(win));
+  gtk_window_set_position(GTK_WINDOW(colorDlg), GTK_WIN_POS_CENTER);
+  gtk_window_set_modal(GTK_WINDOW(colorDlg), TRUE);
+
+  /* Store data for callback */
+  g_object_set_data(G_OBJECT(colorDlg), "Color", color);
+  g_object_set_data(G_OBJECT(colorDlg), "Button", button);
+  g_object_set_data(G_OBJECT(colorDlg), "Style", style);
+
+  /* Connect to response signal instead of individual button signals */
+  gint response = gtk_dialog_run(GTK_DIALOG(colorDlg));
+  
+  if(response == GTK_RESPONSE_OK) {
+    /* User clicked OK, update the color */
+    set_dipole_color(GTK_COLOR_CHOOSER(colorDlg), color);
+    set_dipole_button_color(GTK_OBJECT(button), NULL);
+    destroy_button_windows(GTK_WIDGET(colorDlg));
+  }
+  
+  gtk_widget_destroy(GTK_WIDGET(colorDlg));
 }
```

## 7. ComboBox Text Updates

```diff
--- a/src/Utils/UtilsInterface.c
+++ b/src/Utils/UtilsInterface.c
@@ -2127,7 +2150,10 @@
   l = list;
   while(l)
   {
-    gtk_combo_box_append_text (GTK_COMBO_BOX (comboBoxEntry), (gchar*)(l->data));
+    /* GTK3 Migration: Use gtk_combo_box_text_append_text for GtkComboBoxText widgets
+     * See: https://developer.gnome.org/gtk3/stable/GtkComboBoxText.html
+     */
+    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (comboBoxEntry), (gchar*)(l->data));
     l = l->next;
   }
```

