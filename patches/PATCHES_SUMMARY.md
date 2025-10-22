# GTK3 Migration Patches Summary

## Available Patches

The following patch files have been generated for review:

### Individual Commit Patches

1. **0001-Initial-analysis-and-Makefile-fix-for-pkg-config.patch** (1.1K)
   - Fixed Makefile to properly use shell invocation for pkg-config
   - Changed `$(pkg-config ...)` to `$(shell pkg-config ...)`

2. **0002-Fix-GTK3-compatibility-function-signatures-deprecate.patch** (31K)
   - Updated function signatures for GTK signal compatibility
   - Replaced deprecated GTK2 APIs with GTK3 equivalents
   - Migrated GdkPixmap to GdkPixbuf
   - Replaced GtkColorSelectionDialog with GtkColorChooserDialog
   - Integrated Cairo for drawing operations
   - Fixed pointer compatibility and type safety

3. **0003-Fix-typo-replace-gtk_pixmap-reference-with-image-var.patch** (1.1K)
   - Fixed typo: replaced leftover `gtk_pixmap` reference with `image` variable

4. **0004-Add-comprehensive-GTK3-migration-documentation.patch** (2.7K)
   - Added UTILS_INTERFACE_GTK3_MIGRATION.md documentation file

### Complete Diff

- **complete-changes.diff** - Contains all changes combined in a single diff file

## How to Apply Patches

You can apply these patches in several ways:

### Apply All Patches in Order
```bash
cd /path/to/gabedit-gtk3
git am /tmp/patches/000*.patch
```

### Apply Individual Patch
```bash
git apply /tmp/patches/0002-Fix-GTK3-compatibility-function-signatures-deprecate.patch
```

### View Patch Contents
```bash
cat /tmp/patches/0002-Fix-GTK3-compatibility-function-signatures-deprecate.patch
```

### Apply Complete Diff
```bash
cd /path/to/gabedit-gtk3
git apply /tmp/patches/complete-changes.diff
```

## Key Changes Summary

### Files Modified
- `Makefile` (8 lines changed)
- `src/Utils/UtilsInterface.h` (18 lines changed)
- `src/Utils/UtilsInterface.c` (576 lines changed)
- `UTILS_INTERFACE_GTK3_MIGRATION.md` (51 lines added - new file)

### Major Changes

1. **Function Signatures (5 functions)**
   - `new_doc_molpro`, `new_doc_gauss`, `new_doc_orca`, `new_doc_qchem`, `show_homepage`
   - Changed from `void func()` to `void func(GtkWidget* widget, gpointer data)`

2. **Deprecated API Replacements**
   - `gtk_about_dialog_set_url_hook` → Removed (auto-handled in GTK3)
   - `gtk_combo_box_append_text` → `gtk_combo_box_text_append_text` (5 instances)
   - `GtkColorSelectionDialog` → `GtkColorChooserDialog`
   - `GdkColor` → `GdkRGBA` (with conversion functions)

3. **Pixmap to Pixbuf Migration (8 functions)**
   - `create_pixmap()`
   - `get_pixmap()` → `get_pixbuf_color()`
   - `create_hbox_pixmap_color()`
   - `create_radio_button_pixmap()`
   - `set_icone()`
   - `create_label_pixmap()`
   - `create_pixmap_label()`
   - `create_button_pixmap()`

4. **Cairo Integration**
   - Replaced `GdkGC`, `gdk_gc_*()` APIs
   - Replaced `gdk_draw_rectangle()` with Cairo drawing
   - New function: `get_pixbuf_color()` using Cairo surfaces

5. **Type Safety**
   - `G_CONST_RETURN` → `const`
   - XPM data cast to `const char**`
   - Fixed widget struct member access (`widget->window`, `widget->style`)

## Statistics
- **Total lines changed:** 594
- **Additions:** 388 lines
- **Deletions:** 265 lines
- **Functions refactored:** 13+
- **Deprecated APIs replaced:** 15+
- **Migration comment blocks:** 15+

