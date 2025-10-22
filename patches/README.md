# GTK3 Migration Patches

This directory contains patch files for the GTK3 migration of UtilsInterface.c and UtilsInterface.h.

## Files

### Patch Files (Git Format)
- `0001-Initial-analysis-and-Makefile-fix-for-pkg-config.patch` - Makefile pkg-config fix
- `0002-Fix-GTK3-compatibility-function-signatures-deprecate.patch` - Main GTK3 migration changes
- `0003-Fix-typo-replace-gtk_pixmap-reference-with-image-var.patch` - Typo fix
- `0004-Add-comprehensive-GTK3-migration-documentation.patch` - Documentation

### Complete Diff
- `complete-changes.diff` - All changes in a single unified diff

### Documentation
- `PATCHES_SUMMARY.md` - Overview and instructions for applying patches
- `KEY_CHANGES.md` - Detailed view of key changes with diff examples

## Viewing the Patches

### View Individual Patch
```bash
cat patches/0002-Fix-GTK3-compatibility-function-signatures-deprecate.patch
```

### View Complete Diff
```bash
cat patches/complete-changes.diff
```

### View Summary
```bash
cat patches/PATCHES_SUMMARY.md
```

### View Key Changes
```bash
cat patches/KEY_CHANGES.md
```

## Applying the Patches

### Apply All Patches (in order)
```bash
git am patches/000*.patch
```

### Apply Individual Patch
```bash
git apply patches/0002-Fix-GTK3-compatibility-function-signatures-deprecate.patch
```

### Apply Complete Diff
```bash
git apply patches/complete-changes.diff
```

## What Changed

- **Makefile**: Fixed pkg-config shell invocation
- **UtilsInterface.h**: Updated function signatures and type declarations
- **UtilsInterface.c**: Complete GTK3 migration with:
  - Signal handler signatures updated
  - Deprecated GTK2 APIs replaced
  - GdkPixmap → GdkPixbuf migration
  - GtkColorSelectionDialog → GtkColorChooserDialog
  - Cairo integration for drawing
  - Type safety improvements
- **UTILS_INTERFACE_GTK3_MIGRATION.md**: Migration documentation

## Statistics

- 4 commits
- 594 lines changed (333 additions, 261 deletions)
- 13+ functions refactored
- 15+ deprecated API calls replaced
- 15+ migration comment blocks added

## Build Status

✅ Compiles successfully with GCC and GTK 3.24.41
✅ Zero compilation errors in UtilsInterface.c and UtilsInterface.h
