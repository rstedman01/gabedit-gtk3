#!/usr/bin/env bash
set -euo pipefail

# repo-root assumed current directory
echo "Scanning for GTK2 patterns in src/... (dry run)"

# Patterns to search
PATTERNS=(
  "GTK_DIALOG(.*)->vbox"
  "GTK_DIALOG(.*)->action_area"
  "GTK_BIN(.*)->child"
  "GTK_TOGGLE_BUTTON.*->active"
  "GTK_OBJECT_FLAGS"
  "GTK_IN_DESTRUCTION"
  "gtk_combo_box_entry_new_text\\s*\\("
  "*->style"
  "*->window"
)

for p in "${PATTERNS[@]}"; do
  echo
  echo "Matches for pattern: $p"
  grep -RIn --color=always -E "$p" $1 || true
done

echo
echo "If you want to auto-replace common patterns, run with APPLY=1."

if [ "${APPLY:-0}" -eq 1 ]; then
  echo "Applying automated safe replacements..."
  # GTK_BIN(...)->child -> gtk_bin_get_child(...)
  perl -0777 -i -pe 's/GTK_BIN\s*\(\s*([^)]+)\s*\)->child/gtk_bin_get_child(GTK_BIN($1))/g' $(git ls-files src | grep -E '\.c$|\.h$')

  # gtk_combo_box_entry_new_text() -> gtk_combo_box_text_new_with_entry()
  perl -0777 -i -pe 's/\bgtk_combo_box_entry_new_text\s*\(\s*\)/gtk_combo_box_text_new_with_entry()/g' $(git ls-files src | grep -E '\.c$|\.h$')

  # GTK_TOGGLE_BUTTON(x)->active -> gtk_toggle_button_get_active...
  perl -0777 -i -pe 's/GTK_TOGGLE_BUTTON\s*\(\s*([^)]+)\s*\)->active/gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON($1))/g' $(git ls-files src | grep -E '\.c$|\.h$')

  # GTK_DIALOG(...)->vbox / ->action_area -> getters
  perl -0777 -i -pe 's/GTK_DIALOG\s*\(\s*([^)]+)\s*\)->vbox/gtk_dialog_get_content_area(GTK_DIALOG($1))/g' $(git ls-files src | grep -E '\.c$|\.h$')
  perl -0777 -i -pe 's/GTK_DIALOG\s*\(\s*([^)]+)\s*\)->action_area/gtk_dialog_get_action_area(GTK_DIALOG($1))/g' $(git ls-files src | grep -E '\.c$|\.h$')

  # GTK_OBJECT_FLAGS(...) & GTK_IN_DESTRUCTION -> gtk_widget_in_destruction
  perl -0777 -i -pe 's/\(\s*GTK_OBJECT_FLAGS\s*\(\s*\(\s*GtkObject\*\)\s*([^)]+)\s*\)\s*&\s*GTK_IN_DESTRUCTION\s*\)/gtk_widget_in_destruction(GTK_WIDGET($1))/g' $(git ls-files src | grep -E '\.c$|\.h$')

  echo "Automatic replacements applied. Please review changes with 'git diff'."
else
  echo "Dry run complete. No files changed."
fi