# GabeditXYPlot GTK3 Port

This directory contains work-in-progress porting GabeditXYPlot from GTK2 to GTK3.

## Current Status

**Foundation Complete**: ~60% done with full documentation for completion

### ✅ What's Done
- Build system configured for GTK3
- All widget accessor patterns updated (window, allocation, realized)
- Class initialization modernized (GObject, draw, size handlers)
- GDK key symbols updated
- GdkColormap removed from Cairo helpers
- Helper module created for drawing attributes

### 📋 What Remains
- realize() function refactoring
- ~55 gdk_draw_* → Cairo conversions
- ~81 GdkGC references to remove
- GdkPixmap → cairo_surface_t conversions
- ~5 gtk_paint_* updates
- widget->style reference updates

### 📚 Documentation
All remaining work is fully documented with conversion patterns:

1. **PORTING_NOTES.md** - Comprehensive status tracking
   - Line-by-line breakdown of all changes needed
   - File status and compilation errors
   - Prioritized task list

2. **CONVERSION_PATTERNS.md** - Code conversion cookbook
   - Before/after examples for every pattern
   - Drawing with Cairo instead of GdkGC
   - Style context usage
   - realize() function template
   - Testing checklist

## Quick Start

### Build
```bash
make clean
make
```

### Check Status
```bash
# Count remaining GTK2 patterns
grep -c "gdk_draw_" src/Utils/GabeditXYPlot.c
grep -c "gdk_gc_" src/Utils/GabeditXYPlot.c
grep -c "widget->style" src/Utils/GabeditXYPlot.c
```

### Apply Patterns
See CONVERSION_PATTERNS.md for specific examples. The general workflow:

1. Find a GTK2 pattern (e.g., `gdk_draw_line`)
2. Look up the pattern in CONVERSION_PATTERNS.md
3. Apply the Cairo equivalent
4. Compile and test
5. Commit with clear message

## Files Modified

### Core Files
- `src/Utils/GabeditXYPlot.c` - Main widget implementation
- `src/Utils/GabeditXYPlot.h` - Widget header
- `Makefile` - Build configuration

### New Helper Files
- `src/Utils/GabeditXYPlotCairo.c` - Drawing attribute helpers
- `src/Utils/GabeditXYPlotCairo.h` - Header for helpers

### Documentation
- `PORTING_NOTES.md` - Detailed status and roadmap
- `CONVERSION_PATTERNS.md` - Conversion examples
- `README-PORTING.md` - This file

## Commit History

The port is done in small, reviewable commits:

1. **Initial analysis** - Fixed Makefile, identified scope
2. **Cairo helpers** - Added drawing attribute support, fixed GdkColormap usage
3. **Widget accessors** - Replaced widget->window/allocation throughout
4. **Class init** - Updated to GObject, added size/draw handlers
5. **Documentation** - Added comprehensive porting notes and patterns

## Testing Strategy

After each conversion:
1. Compile without warnings
2. Verify rendering
3. Test mouse interactions
4. Check memory usage
5. Test export functions

## Architecture Notes

### Drawing Flow
- GTK2: GdkGC stores attributes → gdk_draw_* uses GC → draws to GdkPixmap/GdkWindow
- GTK3: Attributes in structures → Cairo context created → draws to cairo_surface_t

### Event Handling
- GTK2: expose_event with GdkEventExpose
- GTK3: draw signal with cairo_t passed in

### Realization
- GTK2: Widget creates its own window, manages style
- GTK3: GTK creates window, widget uses style context

## Dependencies

- GTK+ 3.x
- Cairo
- Pango/PangoCairo
- GdkPixbuf

## Resources

- [GTK 3 Migration Guide](https://docs.gtk.org/gtk3/migrating.html)
- [Cairo Tutorial](https://www.cairographics.org/tutorial/)
- [Pango Documentation](https://docs.gtk.org/Pango/)

## Contributing

When adding more conversions:

1. Use patterns from CONVERSION_PATTERNS.md
2. Make small, focused commits
3. Update PORTING_NOTES.md status
4. Test thoroughly
5. Document any new patterns discovered

## Future Work

Once compilation succeeds:

1. Runtime testing of all features
2. Performance profiling
3. Memory leak detection
4. Visual comparison with GTK2 version
5. Export format validation
6. Theme compatibility testing

## Contact

For questions about this port, refer to the commit messages and documentation files. Each commit explains its specific changes and rationale.

---
Last Updated: 2025-10-17
Status: Foundation complete, systematic conversion ready to proceed
