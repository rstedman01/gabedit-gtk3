void getLattice(gdouble boxLength[], gdouble maxr, gint g)
{
	gint nTv = 0;
	gdouble Tv[3][3];
	for(gint a=0;a<geometryConvergence.geometries[g].numberOfAtoms;a++)
	{
		sprintf(tmp,"%s", geometryConvergence.geometries[g].listOfAtoms[a].symbol);
		uppercase(tmp);
		if(!strcmp(tmp,"TV")) { 
			for(j=0;j<3;j++) Tv[nTv][j]= geometryConvergence.geometries[g].listOfAtoms[a].C[j];
			nTv++;
		}
	}
	if(nTv<3) 
	{
		for(gint i=0;i<3;i++) boxLength[i] = 2*maxr;
		return;
	}
	for(gint i=0;i<nTv;i++)
	{
		gdouble r = 0
		for(j=0;j<3;j++) r+= Tv[i][j]* Tv[i][j];
		boxLength[i]=sqrt(r);	
	}

}
