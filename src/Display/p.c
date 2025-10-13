#include <glib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../../Config.h"
#include "../Common/GabeditType.h"
#include "AnimationGeomConv.h"
#include "../Utils/Utils.h"

void getLattice(gdouble boxLength[], gdouble maxr, gint g)
{
	gint nTv = 0;
	gdouble Tv[3][3] = { {0.0,0.0,0.0}, {0.0,0.0,0.0}, {0.0,0.0,0.0} };
	gchar tmp[8];
	for(gint a = 0; a < geometryConvergence.geometries[g].numberOfAtoms; a++)
	{
		sprintf(tmp,"%s", geometryConvergence.geometries[g].listOfAtoms[a].symbol);
		uppercase(tmp);
		if(!strcmp(tmp,"TV")) { 
			for(gint j = 0; j < 3; j++) Tv[nTv][j]= geometryConvergence.geometries[g].listOfAtoms[a].C[j];
			nTv++;
		}
	}
	if(nTv<3) 
	{
		for(gint i = 0; i < 3; i++) boxLength[i] = 2 * maxr;
		return;
	}
	for(gint i = 0; i < nTv; i++)
	{
		gdouble r = 0;
		for(gint j = 0;j < 3;j++) r += Tv[i][j] * Tv[i][j];
		boxLength[i] = sqrt(r);	
	}

}
