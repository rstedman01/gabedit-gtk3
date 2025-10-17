# GTK2 to GTK3 Conversion Patterns for GabeditXYPlot

This document provides quick reference patterns for completing the GTK3 port.

## Widget Accessors (DONE)

```c
// GTK2 → GTK3
widget->window           → gtk_widget_get_window(widget)
widget->allocation.width → gtk_widget_get_allocated_width(widget)
widget->allocation.height → gtk_widget_get_allocated_height(widget)
widget->parent           → gtk_widget_get_parent(widget)
GTK_WIDGET_REALIZED(w)   → gtk_widget_get_realized(w)
```

## Drawing with GdkGC → Cairo

### Pattern 1: Simple Line
```c
// GTK2
gdk_draw_line(window, gc, x1, y1, x2, y2);

// GTK3
cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
/* Set color from GdkColor */
cairo_set_source_rgb(cr, color.red/65535.0, color.green/65535.0, color.blue/65535.0);
cairo_set_line_width(cr, line_width);
cairo_move_to(cr, x1, y1);
cairo_line_to(cr, x2, y2);
cairo_stroke(cr);
cairo_destroy(cr);
```

### Pattern 2: Rectangle
```c
// GTK2
gdk_draw_rectangle(window, gc, filled, x, y, width, height);

// GTK3
cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
cairo_set_source_rgb(cr, color.red/65535.0, color.green/65535.0, color.blue/65535.0);
cairo_rectangle(cr, x, y, width, height);
if (filled)
    cairo_fill(cr);
else {
    cairo_set_line_width(cr, line_width);
    cairo_stroke(cr);
}
cairo_destroy(cr);
```

### Pattern 3: Polygon
```c
// GTK2
gdk_draw_polygon(window, gc, filled, points, npoints);

// GTK3
cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
cairo_set_source_rgb(cr, color.red/65535.0, color.green/65535.0, color.blue/65535.0);
cairo_move_to(cr, points[0].x, points[0].y);
for (int i = 1; i < npoints; i++)
    cairo_line_to(cr, points[i].x, points[i].y);
cairo_close_path(cr);
if (filled)
    cairo_fill(cr);
else {
    cairo_set_line_width(cr, line_width);
    cairo_stroke(cr);
}
cairo_destroy(cr);
```

## Text Drawing

```c
// GTK2
gtk_paint_layout(style, window, state, use_text, area, widget, detail, x, y, layout);

// GTK3
cairo_t *cr = gdk_cairo_create(gtk_widget_get_window(widget));
GtkStyleContext *context = gtk_widget_get_style_context(widget);
GdkRGBA color;
gtk_style_context_get_color(context, gtk_widget_get_state_flags(widget), &color);
gdk_cairo_set_source_rgba(cr, &color);
cairo_move_to(cr, x, y);
pango_cairo_show_layout(cr, layout);
cairo_destroy(cr);
```

## GdkPixmap → cairo_surface_t

```c
// GTK2
GdkPixmap *pixmap = gdk_pixmap_new(window, width, height, -1);
cairo_t *cr = gdk_cairo_create(pixmap);
// ... draw to cr ...
cairo_destroy(cr);
// Copy pixmap to window
gdk_draw_drawable(window, gc, pixmap, 0, 0, 0, 0, width, height);

// GTK3
cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
cairo_t *cr = cairo_create(surface);
// ... draw to cr ...
cairo_destroy(cr);
// Copy surface to window
cairo_t *cr_win = gdk_cairo_create(gtk_widget_get_window(widget));
cairo_set_source_surface(cr_win, surface, 0, 0);
cairo_paint(cr_win);
cairo_destroy(cr_win);
cairo_surface_destroy(surface);
```

## Style Context

```c
// GTK2
GtkStyle *style = widget->style;
GdkColor *fg = &style->fg[GTK_STATE_NORMAL];
PangoFontDescription *font = style->font_desc;
gtk_widget_modify_bg(widget, GTK_STATE_NORMAL, &color);

// GTK3
GtkStyleContext *context = gtk_widget_get_style_context(widget);
GdkRGBA color;
gtk_style_context_get_color(context, GTK_STATE_FLAG_NORMAL, &color);
PangoFontDescription *font;
gtk_style_context_get(context, GTK_STATE_FLAG_NORMAL, "font", &font, NULL);

/* For setting colors, use CSS */
GtkCssProvider *provider = gtk_css_provider_new();
gtk_css_provider_load_from_data(provider, 
    "* { background-color: #ffffff; color: #000000; }", -1, NULL);
gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
    GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
g_object_unref(provider);
```

## realize() Function Pattern

```c
// GTK2 (typical pattern)
static void my_widget_realize(GtkWidget *widget)
{
    GdkWindowAttr attributes;
    gint attributes_mask;
    
    GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);
    
    attributes.x = widget->allocation.x;
    attributes.y = widget->allocation.y;
    attributes.width = widget->allocation.width;
    attributes.height = widget->allocation.height;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual(widget);
    attributes.colormap = gtk_widget_get_colormap(widget);
    attributes.event_mask = gtk_widget_get_events(widget) | ...;
    
    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL | GDK_WA_COLORMAP;
    
    widget->window = gdk_window_new(gtk_widget_get_parent_window(widget),
                                     &attributes, attributes_mask);
    gdk_window_set_user_data(widget->window, widget);
    
    widget->style = gtk_style_attach(widget->style, widget->window);
    gtk_style_set_background(widget->style, widget->window, GTK_STATE_ACTIVE);
    
    /* Create GCs */
    my_widget->gc = gdk_gc_new(widget->window);
}

// GTK3
static void my_widget_realize(GtkWidget *widget)
{
    GtkAllocation allocation;
    GdkWindow *window;
    GdkWindowAttr attributes;
    gint attributes_mask;
    
    gtk_widget_set_realized(widget, TRUE);
    
    gtk_widget_get_allocation(widget, &allocation);
    
    attributes.x = allocation.x;
    attributes.y = allocation.y;
    attributes.width = allocation.width;
    attributes.height = allocation.height;
    attributes.window_type = GDK_WINDOW_CHILD;
    attributes.wclass = GDK_INPUT_OUTPUT;
    attributes.visual = gtk_widget_get_visual(widget);
    attributes.event_mask = gtk_widget_get_events(widget) | ...;
    
    attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
    
    window = gdk_window_new(gtk_widget_get_parent_window(widget),
                            &attributes, attributes_mask);
    gtk_widget_set_window(widget, window);
    gtk_widget_register_window(widget, window);
    
    /* Don't create GCs - store drawing attributes directly */
    /* Initialize cairo surfaces if needed */
    MyWidget *my = MY_WIDGET(widget);
    my->surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 
                                               allocation.width, 
                                               allocation.height);
}
```

## GdkGC Replacement Strategy

Instead of storing GdkGC pointers, store drawing attributes:

```c
// Define structure
typedef struct {
    GdkRGBA color;
    double line_width;
    cairo_line_cap_t cap_style;
    cairo_line_join_t join_style;
    GdkLineStyle dash_style;
} DrawingAttrs;

// Use in widget
struct _MyWidget {
    GtkWidget parent;
    DrawingAttrs foreground;
    DrawingAttrs background;
    // ... other members
};

// Apply to Cairo context
void apply_attrs(cairo_t *cr, DrawingAttrs *attrs)
{
    gdk_cairo_set_source_rgba(cr, &attrs->color);
    cairo_set_line_width(cr, attrs->line_width);
    cairo_set_line_cap(cr, attrs->cap_style);
    cairo_set_line_join(cr, attrs->join_style);
    
    if (attrs->dash_style == GDK_LINE_ON_OFF_DASH) {
        double dashes[] = {5.0, 5.0};
        cairo_set_dash(cr, dashes, 2, 0);
    } else {
        cairo_set_dash(cr, NULL, 0, 0);
    }
}
```

## Converting GdkColor to GdkRGBA

```c
// GdkColor (16-bit per channel)
GdkColor color = { .red = 32767, .green = 32767, .blue = 32767 };

// To GdkRGBA (0.0-1.0 per channel)
GdkRGBA rgba;
rgba.red = color.red / 65535.0;
rgba.green = color.green / 65535.0;
rgba.blue = color.blue / 65535.0;
rgba.alpha = 1.0;
```

## Image Capture

```c
// GTK2
GdkPixbuf *pixbuf = gdk_pixbuf_get_from_drawable(NULL, widget->window, NULL,
                                                  0, 0, 0, 0, width, height);

// GTK3
GdkWindow *window = gtk_widget_get_window(widget);
GdkPixbuf *pixbuf = gdk_pixbuf_get_from_window(window, 0, 0, width, height);
```

## Helpful Macros (Already Available)

From gabeditGTK3compat.h:
```c
#define SCALE(i) (i / 65535.)      // Convert 16-bit color to 0.0-1.0
#define SCALE2(i) (i * 65535.)     // Convert 0.0-1.0 to 16-bit color
```

## Common Pitfalls

1. **Don't forget to destroy Cairo contexts**: Always `cairo_destroy(cr)` when done
2. **Cairo coordinates are doubles**: Cast ints if needed for precision
3. **Surface format matters**: Use CAIRO_FORMAT_ARGB32 for full color
4. **Style context state**: Always pass correct state flags
5. **Window realization**: Check `gtk_widget_get_realized()` before using window
6. **Line dash patterns**: Set before stroking, reset after if needed
7. **Cairo transform state**: Use `cairo_save()`/`cairo_restore()` around transforms

## Testing Checklist

After each conversion:
- [ ] Code compiles without warnings
- [ ] Widget renders correctly
- [ ] Colors match expected values
- [ ] Line styles (solid/dashed) work
- [ ] Mouse interactions still function
- [ ] No memory leaks (check with valgrind)
- [ ] Export functions produce correct output

## Quick Reference Commands

```bash
# Compile single file
gcc -std=c17 -O2 $(pkg-config --cflags gtk+-3.0) -c src/Utils/GabeditXYPlot.c

# Find remaining GTK2 patterns
grep -n "gdk_draw_\|gdk_gc_\|widget->style\|widget->window\|GTK_WIDGET_SET_FLAGS" src/Utils/GabeditXYPlot.c

# Count occurrences
grep -c "gdk_draw_" src/Utils/GabeditXYPlot.c
```
