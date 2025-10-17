# GTK3 Helper Functions for GabeditXYPlot

This directory contains helper functions for the GTK3 port of GabeditXYPlot-related components.

## Files

- `gabedit_gdk3_helpers.h` - Header file with function declarations
- `gabedit_gdk3_helpers.c` - Implementation file

## Functions

### capture_widget_to_pixbuf()

```c
GdkPixbuf* capture_widget_to_pixbuf(GtkWidget *widget);
```

Captures the rendered contents of a GTK widget into a GdkPixbuf.

**Parameters:**
- `widget`: The GtkWidget to capture (must be realized and have a GdkWindow)

**Returns:**
- A new GdkPixbuf containing the widget's content, or NULL on error
- The caller must free the returned pixbuf with `g_object_unref()`

**Usage example:**
```c
GdkPixbuf *pixbuf = capture_widget_to_pixbuf(widget);
if (pixbuf) {
    // Use the pixbuf
    gdk_pixbuf_save(pixbuf, "screenshot.png", "png", NULL, NULL);
    g_object_unref(pixbuf);
}
```

### draw_pango_layout_with_cairo()

```c
void draw_pango_layout_with_cairo(cairo_t *cr, PangoLayout *layout, 
                                  int x, int y, 
                                  gboolean centerX, gboolean centerY, 
                                  double angle);
```

Draws a PangoLayout on a Cairo context with optional centering and rotation.

**Parameters:**
- `cr`: Cairo context to draw on
- `layout`: PangoLayout to render
- `x`: X coordinate for the layout position
- `y`: Y coordinate for the layout position
- `centerX`: If TRUE, center the layout horizontally at x
- `centerY`: If TRUE, center the layout vertically at y
- `angle`: Rotation angle in radians

**Behavior:**
- The function saves and restores the Cairo context state
- If centering is enabled, the centering calculation accounts for rotation
- The rotation is applied around the specified (x, y) point

**Usage example:**
```c
cairo_t *cr = ...;
PangoLayout *layout = pango_cairo_create_layout(cr);
pango_layout_set_text(layout, "Hello World", -1);

// Draw text centered at (100, 100) with 45-degree rotation
draw_pango_layout_with_cairo(cr, layout, 100, 100, TRUE, TRUE, M_PI / 4);

g_object_unref(layout);
```

## Implementation Notes

### Widget Capture
The `capture_widget_to_pixbuf()` function uses GTK3's `gdk_pixbuf_get_from_window()` API, which is the recommended way to capture widget content in GTK3. This replaces the deprecated GTK2 `gdk_pixbuf_get_from_drawable()` function.

### Text Rendering with Rotation
The `draw_pango_layout_with_cairo()` function implements a specific coordinate transformation order:

1. Calculate centering offsets that account for the upcoming rotation
2. Move to the adjusted position with `cairo_move_to()`
3. Apply rotation with `cairo_rotate()`
4. Draw the layout with `pango_cairo_show_layout()`

This approach ensures that when both centering and rotation are enabled, the center of the rotated text appears at the specified (x, y) coordinate.

The centering calculation uses trigonometry to pre-compute the offset:
- For X centering: `x -= (w/2)*cos(angle) - (h/2)*sin(angle)`
- For Y centering: `y -= (w/2)*sin(angle) + (h/2)*cos(angle)`

Where `w` and `h` are the layout width and height in pixels.

## Compatibility

These helper functions are designed for GTK3 and use the following APIs:
- `gtk_widget_get_window()`
- `gtk_widget_get_allocated_width()` / `gtk_widget_get_allocated_height()`
- `gdk_pixbuf_get_from_window()`
- `pango_cairo_show_layout()`

They are meant to replace GTK2 patterns found in the legacy GabeditXYPlot code.
