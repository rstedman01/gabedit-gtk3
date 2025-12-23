
#ifndef __GABEDIT_PREVIEWGEOM_H__
#define __GABEDIT_PREVIEWGEOM_H__

GtkWidget* add_preview_geom(GtkWidget* box);
void add_frag_to_preview_geom(GtkWidget* drawingArea, Fragment* frag);

typedef struct _PreviewData 
{
    cairo_surface_t *offscreen;
    int width;
    int height;
} PreviewData;

#endif 

