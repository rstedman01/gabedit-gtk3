#include "../../Config.h"
#include <glib.h>
#include <math.h>
#include <GL/gl.h>
#include "GlobalOrb.h"
#include "Vibration.h"
#include "PrincipalAxisGL.h"

#define HEV 27.21138469
#define KEV 8.61734355e-05
#define UMASI 1.66053873e-27
#define EVSI 1.60217646e-19
#define HPLANK 6.62606957e-34
#define ATMSI 101325.33538686
#define EVCAL 23.06054819e3
#define CM1K 1.43877505

#ifndef M_PI
#ifdef G_PI
#define M_PI G_PI
#else 
#define M_PI 3.14159265358979323846
#endif
#endif

#if !defined(__STDC_VERSION__ ) || (__STDC_VERSION__ < 199901L)
#include <stdlib.h>
#endif

/********************************************************************************/
static inline void getRotConstants(const gdouble I[3], gdouble TRot[3])
{
        const gdouble x = 6.62606867 / 8.0 / M_PI / M_PI / 2.99792458 / 1.66053873 / 0.5291772083 / 0.5291772083 * 1.0e3;
        gint i;
        for (i = 0; i < 3; i++)
        {
                TRot[i] = 0.0;
                if (fabs(I[i]) > 1e-10) TRot[i] = (gdouble)(x / I[i]) * CM1K;
        }
}
/********************************************************************************/
static void computeEnergyCorrections(double E[], double T, double TRot[])
{
        // 0 = translation
        double kT=KEV*T; // in eV
        double k=KEV; // in eV*K^-1
        E[0] = 1.5*kT;
        // 1 = electronic
        E[1] = 0.0;
        // 2 = rotational
        E[2] = 1.5*kT;
        for(int i=0;i<3;i++) if(TRot[i]<=1e-12)  E[2] = kT;
        // 3 = vibrational
        E[3] = 0;
        for(int i=0;i<vibration.numberOfFrequencies;i++) 
	{
		gdouble TVib = vibration.modes[i].frequence*CM1K;
		if(TVib>0) E[3] += k*TVib*(0.5+1/(exp(TVib/T)-1));
	}
        // 4 = All
        E[4] = 0 ;
        for(int i=0;i<=3;i++) E[4]+= E[i];
        /*
        cout<<"Etransi(H)="<<E[0]/HEV<<endl;
        cout<<"Ee(H)="<<E[1]/HEV<<endl;
        cout<<"Erot(H)="<<E[2]/HEV<<endl;
        cout<<"Evib(H)="<<E[3]/HEV<<endl;
        cout<<"Etot(H)="<<E[4]/HEV<<endl;
        */
}
/**********************************************************************/
static void computeEntropies(double S[], double T, double TRot[], double m, double P, double multiplicity)
{
        double kT=KEV*T; // in eV
        double h = HPLANK; //m2 kg / s
        double p = P*ATMSI; // N m^-2
        double k = KEV; // in eV*K^-1

        gdouble m_si = m * UMASI; //MKSA
        gdouble kT_si = kT * EVSI; // MKSA

        //cout<<"TRot="<<TRot[0]<<" "<<TRot[1]<<" "<<TRot[2]<<endl;
        // 0 = translation
        S[0] = k * (1.5 * log(2 * M_PI * m * kT_si / (h * h)) + log(kT_si / p ) + 2.5);

        // 1 = electronic
        S[1] = 0.0;
        if(multiplicity>0) S[1] += k * log(multiplicity * 1.0);
        // 2 = rotational
        {
                gint i;
                gint n = 0;
                for(int i = 0; i < 3; i++) if(TRot[i] > 1e-12)  n++;
                gdouble tmp = 1.0;
                for(int i = 0; i < 3; i++) if(TRot[i] > 1e-12) tmp *= sqrt((M_PI * T / TRot[i]));
                tmp /= M_PI;
                tmp /= (gdouble)((int)((gdouble)(n) + 0.6));
                S[2] = k * (log(tmp) + (gdouble)n * 1.0);
        }
        // 3 = vibrational
        S[3] = 0.0;
        {
                gint i;
                for (i = 0; i < vibration.numberOfFrequencies; i++)
                {
                        gdouble vib = vibration.modes[i].frequence * CM1K;
                        if (vib > 0.0)
                        {
                                S[3] += vib / T / (exp(vib / T) - 1.0) - log(1.0 - exp(-vib / T));
                        }
                }
                S[3] *= k;
        }
        S[4] = 0.0;
        {
                gint i;
                for (i = 0; i <= 3; i++) S[4] += S[i];
        }
}
/************************************************************************************************************/
static void computeThermo(double T, double P, double Eelec, double multiplicity)
{
	double I[3];
	double TRot[3];
	double S[4];
	double E[4];
	double H;
	double G;
        double kT = KEV * T; // in eV

	compute_the_principal_axis();
	getPrincipalAxisInertias(I);
	
        getRotConstants(I, TRot);

	computeEnergyCorrections(E,  T,  TRot);
        {	
                gdouble m_total = 0.0;
                gint i;
                for (i = 0; i < nCenters; i++)
                {
                        m_total += fabs(GeomOrb[i].Prop.masse);
                }

                computeEntropies(S, T, TRot, m_total, P, multiplicity);
        }

        H = Eelec + E[4] + kT;
        G = H - T*S[4];
        (void)H;
        (void)G;
}
