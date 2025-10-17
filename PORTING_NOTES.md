# GabeditXYPlot GTK2 to GTK3 Porting Notes

## Overview
This document tracks the progress of porting GabeditXYPlot from GTK2 to GTK3, focusing on replacing deprecated APIs with Cairo/pango_cairo and GdkPixbuf equivalents.

## Scope
- **Target files**: src/Utils/GabeditXYPlot.c, src/Utils/GabeditXYPlot.h, and helper files
- **Approach**: Small, incremental commits for easier review and testing
- **Goal**: Compile and run GabeditXYPlot with GTK3 APIs

## Completed Changes

### Build System
- [x] Fixed Makefile to properly evaluate pkg-config for GTK3 dependencies

### Header and Includes  
- [x] Added cairo.h, pango/pangocairo.h includes
- [x] Fixed M_PI definition (use G_PI fallback)
- [x] Created GabeditXYPlotCairo helper module with drawing attribute structure

### Widget Accessors
- [x] Replaced all `widget->window` with `gtk_widget_get_window(widget)`
- [x] Replaced `widget->allocation.width/height` with `gtk_widget_get_allocated_width/height()`
- [x] Replaced `GTK_WIDGET_REALIZED(widget)` with `gtk_widget_get_realized(widget)`

### Class Initialization
- [x] Replaced GtkObjectClass with GObjectClass
- [x] Changed `destroy` callback to `finalize`
- [x] Replaced `expose_event` with `draw` signal (via compatibility wrapper)
- [x] Replaced `size_request` with `get_preferred_width/height` handlers
- [x] Updated `size_allocate` to use `gtk_widget_set_allocation()`
- [x] Replaced `style_set` with `style_updated`

### GDK Key Symbols
- [x] Updated GDK key symbols from `GDK_xxx` to `GDK_KEY_xxx` format
  - GDK_Shift_L/R → GDK_KEY_Shift_L/R
  - GDK_Control_L/R → GDK_KEY_Control_L/R
  - GDK_c/C, GDK_v/V, GDK_t/T, GDK_l/L, GDK_i/I, GDK_r/R → GDK_KEY_* versions

### Cairo Drawing Helpers
- [x] Removed GdkColormap usage from Cairo helper functions
- [x] Updated `gabedit_xyplot_cairo_line()` to get color directly from GdkGC foreground
- [x] Updated `gabedit_xyplot_cairo_rectangle()` similarly
- [x] Updated `get_pango_str()` to avoid GdkColormap lookups

## Remaining Work

### High Priority (Required for Compilation)

#### realize() Function (~150 lines)
The `gabedit_xyplot_realize()` function needs major updates:
- [ ] Remove manual window creation (GTK3 handles this)
- [ ] Remove `widget->window =` assignment
- [ ] Remove `widget->style =` assignment
- [ ] Remove `gtk_style_attach()` calls
- [ ] Remove `gtk_style_set_background()` calls
- [ ] Remove `gdk_drawable_get_colormap()` usage
- [ ] Remove `gdk_colormap_alloc_color()` calls
- [ ] Remove `gtk_widget_modify_bg/fg()` calls (use CSS instead)
- [ ] Replace GdkGC creation with XYPlotDrawAttr or store attributes directly
- [ ] Use `gtk_widget_register_window()` if creating custom windows
- [ ] Update to get style context via `gtk_widget_get_style_context()`

#### GdkGC Removal (~81 occurrences)
All GdkGC usage needs replacement:
- [ ] Replace `gdk_gc_new()` / `gdk_gc_new_with_values()` with XYPlotDrawAttr
- [ ] Replace `gdk_gc_set_rgb_fg_color()` with direct color storage
- [ ] Replace `gdk_gc_set_line_attributes()` with Cairo attribute storage
- [ ] Replace `gdk_gc_get_values()` with direct attribute access
- [ ] Update GabeditXYPlot structure to replace GdkGC pointers:
  - `back_gc, fore_gc, data_gc, lines_gc`
  - `hmajor_grid_gc, hminor_grid_gc, vmajor_grid_gc, vminor_grid_gc`

#### gdk_draw_* Replacement (~55 occurrences)
Replace all GDK drawing calls with Cairo equivalents:
- [ ] `gdk_draw_rectangle()` → `cairo_rectangle() + cairo_stroke()/fill()`
- [ ] `gdk_draw_line()` → `cairo_move_to() + cairo_line_to() + cairo_stroke()`
- [ ] `gdk_draw_polygon()` → Cairo path operations
- [ ] `gdk_draw_drawable()` → `cairo_set_source_surface() + cairo_paint()`
- [ ] Create Cairo context with `cairo_create()` from widget window

#### gtk_paint_* Replacement (~5 occurrences)
- [ ] `gtk_paint_layout()` → `pango_cairo_show_layout()`
- [ ] `gtk_paint_flat_box()` → `cairo_rectangle() + cairo_fill()`
- [ ] Get GtkStyleContext for colors: `gtk_style_context_get_color()`

#### GdkPixmap Replacement (~10+ occurrences)
- [ ] Replace `plotting_area` (GdkPixmap) with cairo_surface_t
- [ ] Replace `old_area` (GdkPixmap) with cairo_surface_t
- [ ] Replace `gdk_pixmap_new()` with `cairo_image_surface_create()`
- [ ] Replace `gdk_pixmap_copy()` with Cairo surface operations
- [ ] Update `gdk_pixbuf_get_from_drawable()` to `gdk_pixbuf_get_from_window()`
- [ ] Handle GdkPixbuf alpha with `gdk_pixbuf_add_alpha()`

#### widget->style Removal (~10 occurrences)
- [ ] Replace `widget->style` with `gtk_widget_get_style_context()`
- [ ] Replace `widget->style->font_desc` with PangoContext queries
- [ ] Remove gtk_style_attach() usage

#### Miscellaneous GTK2 API
- [ ] Fix `widget->parent` → `gtk_widget_get_parent()`
- [ ] Fix `widget->allocation.x/y` accessors
- [ ] Fix `GTK_REALIZED` macro usage (one missed occurrence)
- [ ] Update GdkImage usage (removed in GTK3)
- [ ] Fix incomplete GdkVisual/GdkColormap operations

### Medium Priority (Code Quality)

#### Drawing Architecture
- [ ] Consider refactoring to use single Cairo context per draw
- [ ] Consolidate repeated Cairo setup code into helpers
- [ ] Review XYPlotDrawAttr usage and expand if helpful

#### Style and Theming
- [ ] Add CSS provider for widget styling instead of widget_modify_*
- [ ] Update color handling to use GdkRGBA throughout
- [ ] Handle theme changes via style_updated properly

### Low Priority (Polish)

#### Code Cleanup
- [ ] Remove `(void)widget` unused parameter markers where Cairo is used
- [ ] Review G_CONST_RETURN usage (deprecated)
- [ ] Update any remaining GTK2 idioms

#### Testing
- [ ] Create test cases for drawing operations
- [ ] Verify all mouse interactions work
- [ ] Test zoom, distance, and selection features
- [ ] Verify export functionality (PDF, PS, SVG, PNG)

## File Status

### Modified Files
- `src/Utils/GabeditXYPlot.c` - Partially ported, ~60% complete
- `src/Utils/GabeditXYPlot.h` - Needs GdkGC → XYPlotDrawAttr update
- `Makefile` - Fixed for GTK3

### New Files
- `src/Utils/GabeditXYPlotCairo.c` - Cairo drawing attribute helpers
- `src/Utils/GabeditXYPlotCairo.h` - Header for drawing helpers

## Compilation Status

Current compilation shows ~70+ errors, primarily in:
1. `gabedit_xyplot_realize()` - Widget/window creation
2. Various drawing functions - gdk_draw_* calls
3. Style context access - widget->style usage
4. GdkPixmap/GdkImage operations

## Next Steps

1. **Complete realize() function refactoring** - This is the largest blocker
2. **Replace gdk_draw_* systematically** - Create Cairo drawing wrappers
3. **Update GabeditXYPlot structure** - Remove GdkGC members
4. **Test incrementally** - Verify each change doesn't break functionality

## Resources

- GTK3 Migration Guide: https://docs.gtk.org/gtk3/migrating.html
- Cairo Tutorial: https://www.cairographics.org/tutorial/
- Pango Cairo Integration: https://docs.gtk.org/PangoCairo/

## Notes

- The current draw handler is a compatibility wrapper around the old expose logic
- GdkGC is used via compatibility typedef but should be fully removed
- Many functions still use GdkColor; consider migrating to GdkRGBA
- The realize() function needs the most work as it directly manipulates widget internals

Last Updated: 2025-10-17
