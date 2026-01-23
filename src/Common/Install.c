/* Install.c */
/**********************************************************************************************************
Copyright (c) 2002-2021 Abdul-Rahman Allouche. All rights reserved

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the Gabedit), to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions
  of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
************************************************************************************************************/


#include "../../Config.h"
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>

#include "Global.h"
#include "../Utils/UtilsInterface.h"
#include "Preferences.h"
#include "../Utils/Utils.h"
#include "../Common/Install.h"
#include "../Utils/AtomsProp.h"
#include "../Molpro/MolproLib.h"
#include "../Molcas/MolcasLib.h"
#include "../MPQC/MPQCLib.h"
#include "../MolecularMechanics/CreateMolecularMechanicsFile.h"
#include "../MolecularMechanics/CreateDefaultPDBTpl.h"

#include "../../pixmaps/Gabedit.xpm"
#include "../../pixmaps/Book_close.xpm"
#include "../../pixmaps/Page.xpm"

#ifdef G_OS_WIN32
#include <direct.h>
#include <io.h>
#define mkdir(p,m) _mkdir(p)
#ifndef S_ISDIR
#define S_ISDIR(mode) ((mode)&_S_IFDIR)
#endif

#endif /* G_OS_WIN32 */



#define PAGE_STYLE(widget)  gtk_widget_set_style(widget, page_style)
#define TITLE_STYLE(widget) gtk_widget_set_style(widget, title_style) 
#define NUM_PAGES    12
#define EEK_PAGE     (NUM_PAGES - 1)
#define WILBER_WIDTH 62
#define USER_INSTALL ""

enum
{
  LIST_PIXBUF = 0,
  LIST_NAME,
};


void create_color_surfaces_file();
void set_color_surface(gint,gdouble[]);
GtkWidget *create_colorsel_frame(GtkWidget *,gchar*,GtkWidget**);


static GdkPixbuf *bookPixbuf = NULL;
static GdkPixbuf *pagePixbuf = NULL;
static GtkWidget *user_install_dialog = NULL;
static GtkWidget *notebook = NULL;
static GtkWidget *title_pixmap = NULL;
static GtkWidget *title_label  = NULL;
static GtkWidget *footer_label = NULL;
static GtkWidget *log_page        = NULL;
static GtkWidget *prop_page     = NULL;
static GtkWidget *commands_network_page = NULL;
static GtkWidget *fontscolors_page = NULL;
static GtkWidget *colorsurfaces_page = NULL;
static GtkWidget *molpro_basis_page = NULL;
static GtkWidget *molcas_basis_page = NULL;
static GtkWidget *mpqc_basis_page = NULL;
static GtkWidget *mm_file_page = NULL;
static GtkWidget *continue_button = NULL;
static GtkWidget *cancel_button   = NULL;
static GtkWidget *action_area   = NULL;

static GtkStyle *title_style = NULL;
static GtkStyle    *page_style = NULL;
static GdkColormap *colormap   = NULL;
static GdkGC *white_gc = NULL;
static GdkColor black_color;
static GdkColor white_color;
static GdkColor title_color;
static struct
{
  gboolean  directory;
  gchar    *text;
  gchar    *description;
}tree_items[] =
{
  {
    FALSE, "atomsprop",
    N_(
       "The \"atomsprop\" file is used to store atoms properties\n"
       "that affect GABEDIT's default atoms properties behavior.\n"
       "You can configured this file with \n"
       "Setting/Preferences in Gabedit menu."
     )
  },
  {
    FALSE, "commands",
    N_(
    "The \"commands\" file is used to store commands\n"
    "for execute FireFly, Gamess-US, Gaussian, Molcas, Molpro, \nMopac, MPQC, NWChem, Orca, Psicode or Q-Chem program.\n"
    )
  },
  {
    FALSE, "network",
    N_(
    "The \"network\" file is used to store network\n"
    "protocols : ftp, rsh or ssh.\n"
    )
  },
  {
    FALSE, "fonts",
    N_(
    "The \"fonts\" file is used to store fonts style\n"
    )
  },
  {
    FALSE, "listfile",
    N_(
    "GABEDIT save the recent files.\n"
    "We save only 20 recent files by type.\n"
    )
  },
  {
    FALSE, "hosts",
    N_(
    "GABEDIT save the recent remote hosts information.\n"
    "We save only 20 recent remote host.\n"
    )
  },
  {
    FALSE, "colorsurfaces",
    N_(
    "The \"colorsurfaces\" file is used to store color surfaces(Density, Orbitals,...).\n"
    )
  },
  {
    FALSE, "molprobasis",
    N_(
    "GABEDIT uses this file if libmol program is not installed at your local host.\n"
    "Please note that this file contains the list of basis(only the names of basis)\n"
    "supported by molpro2002.6.\n"
    )
  },
  {
    FALSE, "molcasbasis",
    N_(
    "GABEDIT uses this file for obtain the list of basis available in Molcas.\n"
    "Please note that this file contains the list of basis(only the names of basis)\n"
    "supported by Molcas7.\n"
    )
  },
  {
    FALSE, "mpqcBasis",
    N_(
    "GABEDIT uses this file for obtain the list of basis available in MPQC.\n"
    "Please note that this file contains the list of basis(only the names of basis)\n"
    "supported by MPQC2.2.2\n"
    )
  },
  {
    FALSE, "MolecularMechanics.prm",
    N_(
    "GABEDIT uses this file for load Molecular mechanics parameters/\n"
    )
  },
  {
    TRUE, "tmp",
    N_(
    "This subdirectory is used by the GABEDIT to store\n"
    "temporary files."
    )
  }
};

static gint num_tree_items = sizeof(tree_items) / sizeof(tree_items[0]);
struct stat  stat_buf;

static gboolean user_install_run();
static void     user_install_prop();
static void     user_install_prop_done();
static void     user_install_commands_network();
static void     user_install_commands_network_done();
static void 	user_install_fontscolors();
static void 	user_install_fontscolors_done();

static void user_install_colorsurfaces();
static void user_install_colorsurfaces_done();

static void user_install_molpro_basis();
static void user_install_molpro_basis_done();

static void user_install_molcas_basis();
static void user_install_molcas_basis_done();

static void user_install_mpqc_basis();
static void user_install_mpqc_basis_done();

static void user_install_mm_file();
static void user_install_mm_file_done();
/********************************************************************************/
static gboolean user_install_quit_timeout(gpointer user_data)
{
  (void)user_data;
  gtk_main_quit();
  return G_SOURCE_REMOVE;
}
/********************************************************************************/
static void set_pixbuf()
{
	if(!bookPixbuf) bookPixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) book_close_xpm);
	if(!pagePixbuf) pagePixbuf = gdk_pixbuf_new_from_xpm_data ((const char **) page_xpm);
}
/********************************************************************************/
static void user_install_notebook_set_current_page(GtkNotebook *notebook,
                                           gint         index)
{
  gchar *title;
  gchar *footer;
  GtkWidget *page;

  page = gtk_notebook_get_nth_page(notebook, index);


  title  = g_object_get_data(G_OBJECT(page), "title");
  footer = g_object_get_data(G_OBJECT(page), "footer");

  gtk_label_set_text(GTK_LABEL(title_label), title);
  gtk_label_set_text(GTK_LABEL(footer_label), footer);

  GtkAllocation alloc;
  if (GTK_IS_WIDGET(title_pixmap))
  {
    gtk_widget_get_allocation(title_pixmap, &alloc);
    gtk_widget_set_size_request(title_pixmap, alloc.width, alloc.height);
  }

/*  if (index == EEK_PAGE)
  {
    gtk_widget_set_size_request(title_pixmap, gtk_widget_get_allocated_width(title_pixmap), gtk_widget_get_allocated_height(title_pixmap));
  }
*/
  gtk_notebook_set_current_page(notebook, index);
}
/********************************************************************************/
static GtkWidget *user_install_notebook_append_page(GtkNotebook *notebook,
                                                    gchar       *title,
                                                    gchar       *footer)
{
  GtkWidget *page;

  page = gtk_vbox_new(FALSE, 6);
  g_object_set_data(G_OBJECT(page), "title", title);
  g_object_set_data(G_OBJECT(page), "footer", footer);
  gtk_notebook_append_page(notebook, page, NULL);
  gtk_widget_show(page);

  return page;
}
/********************************************************************************/
static void add_label(GtkBox *box, gchar *text)
{
  GtkWidget *label;

  label = gtk_label_new(text);
  PAGE_STYLE(label);
  gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);

  gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
  gtk_box_pack_start(box, label, FALSE, FALSE, 0);

  gtk_widget_show(label);
}
/********************************************************************************/
static void user_install_treeview_select_row(GtkTreeSelection *selection, gpointer data)
{
    GtkNotebook *notebook = GTK_NOTEBOOK(data);
    GtkTreeIter iter;
    GtkTreeModel *model;
    
    if (!gtk_tree_selection_get_selected(selection, &model, &iter))
        return;
    
    if (!GTK_IS_TREE_MODEL(model)) {
        g_warning("Invalid tree model in selection callback");
        return;
    }
    
    GtkTreePath *path = gtk_tree_model_get_path(model, &iter);
    if (!path) return;
    
    gint *indices = gtk_tree_path_get_indices(path);
    if (indices) {
        gint depth = gtk_tree_path_get_depth(path);
        gint page_num = (depth > 1) ? indices[1] + 1 : 0;
        gtk_notebook_set_current_page(notebook, page_num);
    }
    
    gtk_tree_path_free(path);
}
/********************************************************************************/
static void user_install_cancel_callback(GtkWidget *widget,
                                         gpointer   data)
{
  static gint timeout = 0;

  if (timeout) gtk_main_quit ();

  gtk_widget_destroy(continue_button);
  user_install_notebook_set_current_page(GTK_NOTEBOOK(notebook), EEK_PAGE);
  timeout = g_timeout_add(1024, user_install_quit_timeout, NULL);
}
/********************************************************************************/
static gint user_install_corner_expose(GtkWidget      *widget,
                                       cairo_t *cr,
                                       gpointer        data)
{
  GtkAllocation alloc;
  GtkStyleContext *ctx;
  GdkRGBA color;
  gint corner = GPOINTER_TO_INT(data);

  if (!GTK_IS_WIDGET(widget)) return FALSE;

  gtk_widget_get_allocation(widget, &alloc);
  ctx = gtk_widget_get_style_context(widget);

  gtk_style_context_save(ctx);
  gtk_style_context_add_class(ctx, "title");
  gtk_style_context_get_background_color(ctx, GTK_STATE_FLAG_NORMAL, &color);

  gtk_style_context_restore(ctx);
  gdk_cairo_set_source_rgba(cr, &color);

  switch (corner)
  {
    case GTK_CORNER_TOP_LEFT:
            cairo_arc(cr, 16, 16, 16, G_PI, 1.5*G_PI);
            cairo_line_to(cr, 0, 0);
            cairo_line_to(cr, 0, 16);
      break;

    case GTK_CORNER_BOTTOM_LEFT:
            cairo_arc(cr, 16, 0, 16, 0.5*G_PI, G_PI);
            cairo_line_to(cr, 0, 0);
            cairo_line_to(cr, 0, 16);
      break;

    case GTK_CORNER_TOP_RIGHT:
            cairo_arc(cr, 16, 16, 16, G_PI, 1.5*G_PI);
            cairo_line_to(cr, 0, 0);
            cairo_line_to(cr, 0, 16);
      break;

    case GTK_CORNER_BOTTOM_RIGHT:
            cairo_arc(cr, 16, 0, 16, 0.5*G_PI, G_PI);
            cairo_line_to(cr, 0, 0);
            cairo_line_to(cr, 0, 16);
      break;
    default:
      return FALSE;
  }
  cairo_fill(cr);
  return TRUE;
}
/********************************************************************************/
static void user_install_continue_callback(GtkWidget *widget,
                                           gpointer   data)
{
	static gint notebook_index = 0;
	UserInstallCallback callback;

	callback = (UserInstallCallback)data;

	switch (notebook_index)
	{
		case 0:
		break;

		case 1:
      /*  Creating the directories can take some time on NFS, so inform
       *  the user and set the buttons insensitive
       */
			gtk_widget_set_sensitive(continue_button, FALSE);
			gtk_widget_set_sensitive(cancel_button, FALSE);
			gtk_label_set_text(GTK_LABEL(footer_label),
							 _("Please wait while your personal\n"
							 "GABEDIT directory is being created..."));

			while (gtk_events_pending())
					gtk_main_iteration();

			if (user_install_run())
				gtk_widget_set_sensitive(continue_button, TRUE);

			gtk_widget_set_sensitive(cancel_button, TRUE);
			break;

		case 2:
			user_install_prop();
			break;

		case 3:
			user_install_prop_done();
			user_install_commands_network();
			break;

		case 4:
			user_install_commands_network_done();
			user_install_fontscolors();
			break;

		case 5:
			user_install_fontscolors_done();
			user_install_colorsurfaces();
			break;
		case 6:
			user_install_colorsurfaces_done();
			user_install_molpro_basis();
			break;

		case 7:
			user_install_molpro_basis_done();
			user_install_molcas_basis();
			break;
		case 8:
			user_install_molcas_basis_done();
			user_install_mpqc_basis();
			break;
		case 9:
			user_install_mpqc_basis_done();
			user_install_mm_file();
			break;
		case 10:
			user_install_mm_file_done();
			gtk_widget_destroy(user_install_dialog);
			g_object_unref(white_gc);
			g_object_unref(title_style);
			g_object_unref(page_style);

			(* callback)();
			return;
			break;

		case EEK_PAGE:
		default:
			g_assert_not_reached();
	}

	if (notebook_index < NUM_PAGES - 1)
		user_install_notebook_set_current_page(GTK_NOTEBOOK(notebook), ++notebook_index);
}
/********************************************************************************/
void create_buttons_dialog(GtkWidget* dialog,UserInstallCallback callback)
{
  GtkWidget* button;



  button = create_button(dialog,_("Cancel"));
  gtk_box_pack_end (GTK_BOX(action_area), button, FALSE, TRUE, 0);
  g_signal_connect_swapped(GTK_OBJECT(button), "clicked", G_CALLBACK(user_install_cancel_callback),GTK_OBJECT(dialog));

  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  cancel_button = button;
  gtk_widget_show_all (button);

  button = create_button(dialog,_(" Continue "));
  gtk_box_pack_end (GTK_BOX(action_area), button, FALSE, TRUE, 0);  
  GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(button);
  continue_button = button;
  g_signal_connect(G_OBJECT(button), "clicked", (GCallback)user_install_continue_callback,callback);
  gtk_widget_show_all (button);

}
/********************************************************************************/
void user_install_dialog_create(UserInstallCallback callback)
{
  GtkWidget *dialog;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *ebox;
  GtkWidget *table;
  GtkWidget *darea;
  GtkWidget *page;
  GtkWidget *sep;
  PangoFontDescription *large_font_desc;
  gchar* Version_S = g_strdup_printf("%d.%d.%d",MAJOR_VERSION,MINOR_VERSION,MICRO_VERSION);
  gchar* temp = NULL;

  dialog = user_install_dialog = gtk_dialog_new ();
  gtk_window_set_type_hint(GTK_WINDOW(dialog), GDK_WINDOW_TYPE_HINT_NORMAL);
  gtk_window_set_position(GTK_WINDOW(dialog),GTK_WIN_POS_CENTER);
  gtk_window_set_title(GTK_WINDOW(dialog),_("Gabedit User Installation"));

  gtk_widget_realize (dialog);
  set_icone(GTK_WIDGET(dialog));

  action_area = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_box_set_homogeneous(GTK_BOX(action_area), FALSE);

  /*  B/W Style for the page contents  */
  GtkCssProvider *css_provider = gtk_css_provider_new();
  const gchar *css_data = 
    "* { "
    "  color: black; "
    "  background-color: white; "
    "}"
    ". title-area { "
    "  background-color: royalblue; "
    "  font-size: 20pt; "
    "  font-weight: bold; "
    "}";

  gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
  GtkStyleContext *context = gtk_widget_get_style_context(dialog);
  gtk_style_context_add_provider(context, 
                                  GTK_STYLE_PROVIDER(css_provider),
                                  GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

  footer_label = gtk_label_new(NULL);
  /* Apply CSS class for styling */
  gtk_style_context_add_class(gtk_widget_get_style_context(footer_label), "page-style");
  gtk_label_set_justify(GTK_LABEL(footer_label), GTK_JUSTIFY_RIGHT);
  gtk_box_pack_start(GTK_BOX(action_area), footer_label, FALSE, FALSE, 8);
  gtk_widget_show(footer_label);

  vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0); // Changed from gtk_vbox_new
  GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  gtk_container_add(GTK_CONTAINER(content), vbox); // Fixed: add to content area

  ebox = gtk_event_box_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(ebox), "title-area");
  gtk_widget_set_events(ebox, GDK_EXPOSURE_MASK);
  gtk_widget_set_size_request(ebox, WILBER_WIDTH + 16, -1);
  gtk_box_pack_start(GTK_BOX(vbox), ebox, FALSE, FALSE, 0);
  gtk_widget_show(ebox);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8); // Changed from gtk_hbox_new
  gtk_container_set_border_width(GTK_CONTAINER(hbox), 8);
  gtk_container_add(GTK_CONTAINER(ebox), hbox);
  gtk_widget_show(hbox);

  title_pixmap = create_pixmap(dialog, gabedit_xpm);
  gtk_box_pack_start(GTK_BOX(hbox), title_pixmap, FALSE, FALSE, 8);
  gtk_widget_show(title_pixmap);

  title_label = gtk_label_new(NULL);
  gtk_style_context_add_class(gtk_widget_get_style_context(title_label), "title-area");
  gtk_label_set_justify(GTK_LABEL(title_label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start(GTK_BOX(hbox), title_label, FALSE, FALSE, 0);
  gtk_widget_show(title_label);

  hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  gtk_box_pack_start(GTK_BOX(vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show(hbox);

  ebox = gtk_event_box_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(ebox), "title-area");
  gtk_widget_set_size_request(ebox, 16, -1);
  gtk_box_pack_start(GTK_BOX(hbox), ebox, FALSE, FALSE, 0);
  gtk_widget_show(ebox);

  ebox = gtk_event_box_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(ebox), "page-style");
  gtk_box_pack_start(GTK_BOX(hbox), ebox, TRUE, TRUE, 0);
  gtk_widget_show(ebox);

  /* GTK3: Use GtkGrid instead of GtkTable */
  table = gtk_grid_new();
  gtk_grid_set_column_spacing(GTK_GRID(table), 8);
  gtk_container_add(GTK_CONTAINER(ebox), table);
  gtk_widget_show(table);

  /* Drawing area with draw signal (not expose-event) */
  darea = gtk_drawing_area_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(darea), "title-area");
  gtk_widget_set_size_request(GTK_WIDGET(darea), 16, 16);
  g_signal_connect(G_OBJECT(darea), "draw",
                   G_CALLBACK(user_install_corner_expose),
                   GINT_TO_POINTER(GTK_CORNER_TOP_LEFT));
  gtk_grid_attach(GTK_GRID(table), darea, 0, 0, 1, 1);
  gtk_widget_show(darea);

  darea = gtk_drawing_area_new();
  gtk_style_context_add_class(gtk_widget_get_style_context(darea), "title-area");
  gtk_widget_set_size_request(GTK_WIDGET(darea), 16, 16);
  g_signal_connect(G_OBJECT(darea), "draw",
                   G_CALLBACK(user_install_corner_expose),
                   GINT_TO_POINTER(GTK_CORNER_BOTTOM_LEFT));
  gtk_grid_attach(GTK_GRID(table), darea, 0, 2, 1, 1);
  gtk_widget_show(darea);

  notebook = gtk_notebook_new();
  gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), FALSE);
  gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), FALSE);
  gtk_grid_attach(GTK_GRID(table), notebook, 1, 1, 1, 1);
  gtk_widget_set_hexpand(notebook, TRUE);
  gtk_widget_set_vexpand(notebook, TRUE);
  gtk_widget_show(notebook);

  gtk_widget_show(vbox);
  /*  Page 1  */
  temp = g_strdup_printf("Welcome to  The GABEDIT %s  User Installation", Version_S);
  g_free(Version_S);
  page = user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                temp,
                                _("Click \"Continue\" to enter the GABEDIT user installation."));

  add_label(GTK_BOX(page),
            _("Gabedit is a Graphical User Interface to FireFly, Gamess-US, Gaussian, Molcas, Molpro, \nMopac, MPQC , NWChem, Orca, Psicode and Q-Chem\n"
            "computational chemistry packages.\n"
            "It can display a variety of calculation results including support for most major molecular file formats.\n"
            "The advanced 'Molecule Builder' allows one to rapidly sketch in molecules and examine them in 3D\n"
            "Graphics can be exported to various formats, including animations\n"
            "\n"
            "Gabedit can creates input file for the computational chemistry packages(CCP) cited above.\n"
            "Gabedit can graphically display a variety of the CCP calculation results\n"
            "Gabedit can display UV-Vis, IR and Raman computed spectra.\n"
            "Gabedit can generate a povray file for geometry, surfaces, contours, planes colorcoded.\n"
            "Gabedit can save picture in BMP, JPEG, PNG, PPM, PDF and PS format.\n"
            "It can generate automatically a seriess of pictures for animation(vibration, geometry convergence, . ...).\n"
            )
            );

  sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL); // Changed from gtk_hseparator_new
  gtk_box_pack_start(GTK_BOX(page), sep, FALSE, FALSE, 2);
  gtk_widget_show(sep);

  add_label(GTK_BOX(page),
            _("Copyright (c) 2002-2021 Abdul-Rahman Allouche.\n"
            "All rights reserved.\n"
            "\nGabedit is free.\n"
            )); 

  /*  Page 2  */
  {
    GtkWidget *hbox;
    GtkWidget *vbox;
    GtkWidget *notebook2;
    GtkWidget *page2;
    GtkWidget *label;
    GtkTreeIter main_node;
    GtkTreeIter sub_node;
    gchar     *str;

    GtkTreeStore *store;
    GtkTreeModel *model;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkWidget* treeView;
    GtkTreeSelection *select;

    gint i;

    gchar *node[1];
    set_pixbuf();

    page = user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
		    			    _("Personal GABEDIT Directory"),
					    _(
                                             "Click \"Continue\" to create "
                                             "your personal GABEDIT directory.")
					    );

    hbox = gtk_hbox_new(FALSE, 8);
    gtk_box_pack_start(GTK_BOX(page), hbox, FALSE, FALSE, 0);
    gtk_widget_show(hbox);

    store = gtk_tree_store_new (2, GDK_TYPE_PIXBUF, G_TYPE_STRING);
    model = GTK_TREE_MODEL (store);

    treeView = gtk_tree_view_new_with_model (model);
    gtk_tree_view_set_rules_hint (GTK_TREE_VIEW (treeView), FALSE);
    gtk_tree_view_set_headers_visible (GTK_TREE_VIEW (treeView), FALSE);
    PAGE_STYLE(treeView);

    column = gtk_tree_view_column_new ();
    gtk_tree_view_column_set_title (column, " ");
    gtk_tree_view_column_set_reorderable(column, FALSE);

    renderer = gtk_cell_renderer_pixbuf_new ();
    gtk_tree_view_column_pack_start (column, renderer, FALSE);
    gtk_tree_view_column_add_attribute (column, renderer, "pixbuf", LIST_PIXBUF);

    renderer = gtk_cell_renderer_text_new ();
    gtk_tree_view_column_pack_start (column, renderer, TRUE);
    gtk_tree_view_column_add_attribute (column, renderer, "text", LIST_NAME);

    gtk_tree_view_append_column (GTK_TREE_VIEW (treeView), column);
    gtk_box_pack_start(GTK_BOX(hbox), treeView, FALSE, FALSE, 0);
    gtk_widget_show(treeView);

    vbox = gtk_vbox_new(FALSE, 6);
    gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 0);
    gtk_widget_show(vbox);

    str = g_strdup_printf(_("For a proper GABEDIT installation, a subdirectory named\n"
                            "%s needs to be created."), gabedit_directory());
    add_label(GTK_BOX(vbox), str);
    g_free(str);

    add_label(GTK_BOX(vbox),
		    _(
              "This subdirectory will contain a number of important files.\n"
                "Click on one of the files or subdirectories in the tree\n"
                "to get more information about the selected item."
		    )
	);

    notebook2 = gtk_notebook_new();
    gtk_container_set_border_width(GTK_CONTAINER(notebook2), 8);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook2), FALSE);
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook2), FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), notebook2, TRUE, TRUE, 0);
    gtk_widget_show(notebook2);

    /*  empty page  */
    page2 = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(page2);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), page2, NULL);

    node[0] = (gchar *)gabedit_directory();

    gtk_tree_store_append(store, &main_node, NULL);
    gtk_tree_store_set (store, &main_node, LIST_NAME, node[0], -1);
    gtk_tree_store_set (store, &main_node, LIST_PIXBUF, bookPixbuf, -1);


    select = gtk_tree_view_get_selection (GTK_TREE_VIEW (treeView));
    gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
    g_signal_connect (G_OBJECT (select), "changed", G_CALLBACK (user_install_treeview_select_row), notebook2);

    for (i = 0; i < num_tree_items; i++)
    {
      node[0] = tree_items[i].text;

      if (tree_items[i].directory)
      {
    		gtk_tree_store_append(store, &sub_node, &main_node);
    		gtk_tree_store_set (store, &sub_node, LIST_NAME, node[0], -1);
    		gtk_tree_store_set (store, &sub_node, LIST_PIXBUF, bookPixbuf, -1);
      } else
      {
    		gtk_tree_store_append(store, &sub_node, &main_node);
    		gtk_tree_store_set (store, &sub_node, LIST_NAME, node[0], -1);
    		gtk_tree_store_set (store, &sub_node, LIST_PIXBUF, pagePixbuf, -1);
      }

      page2 = gtk_vbox_new(FALSE, 0);
      label = gtk_label_new(tree_items[i].description);
      PAGE_STYLE(label);
      PAGE_STYLE(label);
      gtk_label_set_justify(GTK_LABEL(label), GTK_JUSTIFY_LEFT);
      gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
      gtk_box_pack_start(GTK_BOX(page2), label, TRUE, TRUE, 0);
      gtk_widget_show(label);
      gtk_widget_show(page2);

      gtk_notebook_append_page(GTK_NOTEBOOK(notebook2), page2, NULL);
    }
    {
  	GtkTreePath *path = gtk_tree_path_new_from_string  ("0");
  	gtk_tree_view_expand_to_path(GTK_TREE_VIEW(treeView), path);
  	gtk_tree_path_free(path);
    }

  }

  /*  Page 3  */
  page = log_page = user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                             _("Creation of Directories"),
                                             NULL);

  /*  Page 4  */
  page = prop_page = user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                             _("GABEDIT Atoms Properties"),
                                             _("Click \"Continue\" to accept the settings above."));

  add_label(GTK_BOX(page),_("Setting for atoms properties."));

  sep = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_box_pack_start(GTK_BOX(page), sep, FALSE, FALSE, 2);
  gtk_widget_show(sep);

  /*  Page 5  */
  page = commands_network_page = user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                             _("GABEDIT Commands/Network"),
                                             _("Click \"Continue\" to accept the settings above."));

  add_label(GTK_BOX(commands_network_page),
	    _(
            "To execute FireFly, Gamess-US, Gaussian, Molcas, Molpro, \nMopac, MPQC, NWChem, Orca, Psicode and Q-Chem program, "
            "GABEDIT needs to know commands system.\n"
            "GABEDIT needs to know network protocols.\n"
  	    "You can change them after installation (Settings/Preferences)."
	    )
	    );

  sep = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(page), sep, FALSE, FALSE, 2);
  gtk_widget_show (sep);

  /*  Page 6  */
  page = fontscolors_page =
           user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                             _("GABEDIT Fonts/Colors setting"),
                                             _("Click \"Continue\" to accept the settings above."));

  add_label(GTK_BOX(fontscolors_page),
	    _(
            "Setting for Text Font and Text Colors, "
            "GABEDIT needs to know the defaults Fonts/Colors for Data and Result editors."
	    )
	    );

  sep = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(page), sep, FALSE, FALSE, 2);
  gtk_widget_show (sep);

  /*  Page 7  */
  page = colorsurfaces_page =
           user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                             _("GABEDIT Color Surfaces setting"),
                                             _("Click \"Continue\" to accept the settings above."));

  add_label(GTK_BOX(colorsurfaces_page),
            _("Setting for color surfaces, "
            "GABEDIT needs to know the defaults Color for surfaces(density,Orbitals,....)."
	    )
	    );

  sep = gtk_hseparator_new();
  gtk_box_pack_start(GTK_BOX(page), sep, FALSE, FALSE, 2);
  gtk_widget_show (sep);

  /*  Page 8  */
  page = molpro_basis_page =
           user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                             _("GABEDIT creation of molpro basis list file"),
                                             _("Click \"Continue\" for next page."));
  add_label(GTK_BOX(molpro_basis_page),
		  _(
            "If the libmol program (delivered with molpro) is not installed on your local host,\n"
            "GABEDIT uses this file for get the list of basis(only the names of basis)supported by molpro.\n"
            "Please note that this file contains the list of basis supported by molpro2002.6."
	    )
	  );

  /*  Page 9  */
  page = molcas_basis_page =
           user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                             _("GABEDIT creation of molcas basis list file"),
                                             _("Click \"Continue\" for next page."));
  add_label(GTK_BOX(molcas_basis_page),
		  _(
            "GABEDIT uses this file for get the list of basis(only the names of basis)supported by molcas.\n"
            "Please note that this file contains the list of basis supported by Molcas7."
	    )
		  );

  /*  Page 10  */
  page = mpqc_basis_page =
           user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                             _("GABEDIT creation of mpqc basis list file"),
                                             _("Click \"Continue\" for next page."));
  add_label(GTK_BOX(mpqc_basis_page),
		  _(
            "GABEDIT uses this file for get the list of basis(only the names of basis)supported by MPQC.\n"
            "Please note that this file contains the list of basis supported by MPQC2.2.2."
	    )
	 );


    /*  Page 11  */
  page = mm_file_page =
  user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                     _("GABEDIT creation of Molecular Mechanics file"),
                                     _("Click \"Continue\" to start GABEDIT."));
  add_label(GTK_BOX(mm_file_page),
            _("Gabedit use this file for load molecular mechanics parameters "));

  /*  EEK page  */
  page = user_install_notebook_append_page(GTK_NOTEBOOK(notebook),
                                           _("Aborting Installation..."),
                                           NULL);

  user_install_notebook_set_current_page(GTK_NOTEBOOK(notebook), 0);

  create_buttons_dialog(dialog,callback);
  gtk_widget_show_all(dialog);
}
/********************************************************************************/
void user_install_verify(UserInstallCallback user_install_callback)
{
  gboolean     properly_installed = TRUE;
  const gchar *filename;
  struct stat  stat_buf;

 filename = gabedit_directory();

  if (stat(filename, &stat_buf) != 0)
    properly_installed = FALSE;

  /*  If there is already a proper installation, invoke the callback  */
  if (properly_installed)
    (* user_install_callback)();
  else
    user_install_dialog_create(user_install_callback);
}

/********************************************************************************/
/*  Local functions  */
static gboolean user_install_run()
{
#ifndef G_OS_WIN32
	FILE *pfp;
#endif
	gchar *str;
	gchar *t1;
	gchar *t2;
	gint i;
	GtkWidget *table;
	GtkWidget *log_text;

#ifdef G_OS_WIN32
	gint Ok = 1;

	t1 = g_strdup_printf("%s",gabedit_directory());
	str = g_strdup_printf("%s %s","mkdir", gabedit_directory());
	Ok  = 1;
	if (mkdir (t1, 0755) < 0) 
		Ok = 0;
	if(Ok)
	for(i=0;i<num_tree_items;i++)
	{
		if(tree_items[i].directory)
		{
			if(t1)
				g_free(t1);
			t1 =  g_strdup_printf("mkdir %s%s%s",gabedit_directory(), G_DIR_SEPARATOR_S,tree_items[i].text);
			t2 = str;
			str =  g_strdup_printf("%s\n%s",str,t1);
			g_free(t1);
			g_free(t2);

			t1 =  g_strdup_printf("%s%s%s",gabedit_directory(), G_DIR_SEPARATOR_S,tree_items[i].text);
			if (mkdir (t1, 0755) < 0) 
			{
				Ok = 0;
				break;
			}
		}
	}
	if(Ok)
	{
		GtkTextBuffer *buffer;

		table = gtk_table_new(1, 2, FALSE);
		gtk_table_set_col_spacing(GTK_TABLE(table), 0, 2);
		gtk_box_pack_start(GTK_BOX(log_page), table, TRUE, TRUE, 0);

		log_text = gtk_text_view_new();
		gtk_table_attach(GTK_TABLE(table), log_text, 0,1, 0,1,
                   GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                   GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                   0, 0);

		gtk_widget_show(log_text);
		gtk_widget_show(table);

		add_label(GTK_BOX(log_page),
		_(
            "Did you notice any error messages in the lines above?\n"
            "If not, installation was successful!,  "
            "Otherwise, quit and investigate the possible reason..."
	    )
	    );

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (log_text));
		gtk_text_buffer_set_text (buffer, str, -1);

		g_object_set_data(G_OBJECT(log_page), "footer",
                     _("Click \"Continue\" to complete GABEDIT installation."));
		if(str);
		g_free(str);

		return TRUE;
	}
	else
	{
		add_label(GTK_BOX(log_page),
              _("Installation failed.  Contact system administrator."));
		return FALSE;
	}
#else /* G_OS_32 */
	str = g_strdup_printf("%s %s","mkdir", gabedit_directory());
	for(i=0;i<num_tree_items;i++)
	{
		if(tree_items[i].directory)
		{
			t1 =  g_strdup_printf("mkdir %s%s%s",gabedit_directory(), G_DIR_SEPARATOR_S,tree_items[i].text);
			t2 = g_strdup(str);
			str =  g_strdup_printf("%s\n%s",t2,t1);
			g_free(t1);
			g_free(t2);
		}
	}

	if ((pfp = popen(str, "r")) != NULL)  
	{
		GtkTextBuffer *buffer;

		table = gtk_table_new(1, 2, FALSE);
		gtk_table_set_col_spacing(GTK_TABLE(table), 0, 2);
		gtk_box_pack_start(GTK_BOX(log_page), table, TRUE, TRUE, 0);

		log_text = gtk_text_view_new();
		gtk_table_attach(GTK_TABLE(table), log_text, 0,1, 0,1,
                   GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                   GTK_EXPAND | GTK_SHRINK | GTK_FILL,
                   0, 0);

		gtk_widget_show(log_text);
		gtk_widget_show(table);

		add_label(GTK_BOX(log_page),
		_(
            "Did you notice any error messages in the lines above?\n"
            "If not, installation was successful!,  "
            "Otherwise, quit and investigate the possible reason..."
	    )
		);

		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (log_text));
		gtk_text_buffer_set_text (buffer, str, -1);

		g_object_set_data(G_OBJECT(log_page), "footer",
                     _("Click \"Continue\" to complete GABEDIT installation."));
		g_free(str);

		return TRUE;
	}
	else
	{
		add_label(GTK_BOX(log_page),
              _("Installation failed.  Contact system administrator."));
		return FALSE;
	}
#endif /* G_OS_32 */

     
}
/********************************************************************************/
static void user_install_prop()
{
  GtkWidget *hbox;

  hbox = gtk_hbox_new(TRUE, 8);
  gtk_box_pack_start(GTK_BOX(prop_page), hbox, TRUE, TRUE, 0);
  gtk_widget_show(hbox); 
  create_table_prop_in_vbox(user_install_dialog,hbox);

}
/********************************************************************************/
static void user_install_prop_done()
{
 save_atoms_prop();
}
/********************************************************************************/
static void user_install_commands_network()
{
/*
  GtkWidget *hbox;

  hbox = gtk_hbox_new(FALSE, 8);
  gtk_box_pack_start(GTK_BOX(commands_network_page), hbox, FALSE, TRUE, 0);
  gtk_widget_show(hbox); 
  create_execucte_commands(user_install_dialog,hbox,TRUE);
#ifdef G_OS_WIN32
  hbox = gtk_hbox_new(FALSE, 8);
  gtk_box_pack_start(GTK_BOX(commands_network_page), hbox, FALSE, TRUE, 0);
  gtk_widget_show(hbox); 
  create_pscpplink_directory(user_install_dialog,hbox,TRUE);
  hbox = gtk_hbox_new(FALSE, 8);
  gtk_box_pack_start(GTK_BOX(commands_network_page), hbox, FALSE, TRUE, 0);
  gtk_widget_show(hbox); 
  create_gamess_directory(user_install_dialog,hbox,TRUE);
#endif
  hbox = gtk_hbox_new(FALSE, 8);
  gtk_box_pack_start(GTK_BOX(commands_network_page), hbox, FALSE, TRUE, 0);
  gtk_widget_show(hbox); 
  create_network_protocols(user_install_dialog,hbox,TRUE);
*/
}
/********************************************************************************/
static void user_install_commands_network_done()
{
/*
  modify_gaussian_command();
  modify_molpro_command();
*/
  create_commands_file();
  create_network_file();
}
/********************************************************************************/
static void user_install_fontscolors()
{
  GtkWidget *hbox;

  hbox = gtk_hbox_new(FALSE, 8);
  gtk_box_pack_start(GTK_BOX(fontscolors_page), hbox, FALSE, FALSE, 0);
  gtk_widget_show(hbox); 
  create_font_color_in_box(user_install_dialog,hbox);
}

/********************************************************************************/
static void user_install_fontscolors_done()
{
 create_fonts_file();
}
/********************************************************************************/
static void user_install_colorsurfaces()
{
  GtkWidget *hbox;
  GtkWidget **selectors = g_malloc(3*sizeof(GtkWidget*));

  hbox = gtk_hbox_new(FALSE, 8);
  gtk_box_pack_start(GTK_BOX(colorsurfaces_page), hbox, FALSE, FALSE, 0);
  gtk_widget_show(hbox); 
  create_colorsel_frame(hbox,NULL,selectors);
  g_object_set_data(G_OBJECT (colorsurfaces_page), "Selectors",selectors);
}

/********************************************************************************/
static void user_install_colorsurfaces_done()
{
	gdouble Col[4];
	GdkColor color;
  	GtkWidget**selectors =(GtkWidget**) g_object_get_data(G_OBJECT (colorsurfaces_page), "Selectors");

	/* positive value */
	gtk_color_button_get_color ((GtkColorButton*)selectors[0], &color);
	Col[0] = color.red/65535.0;
	Col[1] = color.green/65535.0;
	Col[2] = color.blue/65535.0;
	set_color_surface(0,Col);

	/* negative value */
	gtk_color_button_get_color ((GtkColorButton*)selectors[1], &color);
	Col[0] = color.red/65535.0;
	Col[1] = color.green/65535.0;
	Col[2] = color.blue/65535.0;
	set_color_surface(1,Col);
	/* density surface */
	gtk_color_button_get_color ((GtkColorButton*)selectors[2], &color);
	Col[0] = color.red/65535.0;
	Col[1] = color.green/65535.0;
	Col[2] = color.blue/65535.0;
	set_color_surface(2,Col);
	g_free(selectors);

	create_color_surfaces_file();
}
/********************************************************************************/
static void user_install_molpro_basis()
{
}
/********************************************************************************/
static void user_install_molpro_basis_done()
{
 create_molpro_basis_file();
}
/********************************************************************************/
static void user_install_molcas_basis()
{
}
/********************************************************************************/
static void user_install_molcas_basis_done()
{
 create_molcas_basis_file();
}
/********************************************************************************/
static void user_install_mpqc_basis()
{
}
/********************************************************************************/
static void user_install_mpqc_basis_done()
{
 create_mpqc_basis_file();
}
/********************************************************************************/
static void user_install_mm_file()
{
}
/********************************************************************************/
static void user_install_mm_file_done()
{
	createMMFile();
	CreateDefaultPDBTpl();
}

