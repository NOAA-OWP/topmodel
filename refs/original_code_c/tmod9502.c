/*  TOPMODEL DEMONSTRATION PROGRAM VERSION 95.02

  This C version by Fred Ogden, Sept. 2009.  Converted to ANSI C
  and given somewhat more meaningful variable names.  Compared
  against original FORTRAN version on a simple data set with no
  infiltration excess.
  
  after Keith Beven 1985
  Revised for distribution 1993,1995

****************************************************************
  This program is distributed freely with only two conditions.

  1. In any use for commercial or paid consultancy purposes a 
     suitable royalty agreement must be negotiated with Lancaster 
     University (Contact Keith Beven)

  2. In any publication arising from use for research purposes the
     source of the program should be properly acknowledged and a 
     pre-print of the publication sent to Keith Beven at the address
     below.

  All rights retained 1993, 1995
  Keith Beven
  Centre for Research on Environmental Systems and Statistics
  Institute of Environmental and Biological Sciences
  Lancaster University, Lancaster LA1 4YQ, UK

  Tel: (+44) 1524 593892  Fax: (+44) 1524 593985
  Email:  K.Beven@UK.AC.LANCASTER
  
****************************************************************

  SIMPLE SUBCATCHMENT VERSION OF TOPMODEL

  This program allows single or multiple subcatchment calculations 
  but with single average rainfall and potential evapotranspiration
  inputs to the whole catchment.  Subcatchment discharges are routed
  to the catchment outlet using a linear routing algorithm with
  constant main channel velocity and internal subcatchment 
  routing velocity.  The program requires ln(a/tanB) distributions
  for each subcatchment.  These may be calculated using the
  GRIDATB program which requires raster elevation data as input.
  It is recommended that those data should be 50 m resolution or
  better.

  NOTE that TOPMODEL is not intended to be a traditional model
  package but is more a collection of concepts that can be used
  **** where appropriate ****. It is up to the user to verify that
  the assumptions are appropriate (see discussion in 
  Beven et al.(1994).   This version of the model  will be
  best suited to catchments with shallow soils and moderate
  topography which do not suffer from excessively long dry 
  periods.  Ideally predicted contributing areas should be
  checked against what actually happens in the catchment.

  It includes infiltration excess calculations and parameters
  based on the exponential conductivity Green-Ampt model of 
  Beven (HSJ, 1984) but if infiltration excess does occur it
  does so over whole area of a subcatchment.  Spatial variability
  in conductivities can however be handled by specifying 
  Ko parameter values for different subcatchments, even if they
  have the same ln(a/tanB) and routing parameters, ie. to 
  represent different parts of the area. 

  Note that time step calculations are explicit ie. SBAR
  at start of time step is used to determine contributing area.  
  Thus with long (daily) time steps contributing area depends on 
  initial value together with any volume filling effect of daily 
  inputs.  Also baseflow at start of time step is used to update 
  SBAR at end of time step

  Current program limits are:
          Number of time steps = 2500
          Number of subcatchments = 10
          Number of ln(a/tanB) increments = 30
          Number of subcatchment routing ordinates = 10
          Number of time delay histogram ordinates = 20
          Size of subcatchment pixel maps = 100 x 100

  Limits are mostly set in Common blocks in file TMCOMMON.FOR
*****************************************************************

  This version uses five files as follows:
       Channel 4 "TOPMOD.DAT" contains run and file information
       Channel 7 <INPUTS$> contains rainfall, pe and qobs data
       Channel 8 <SUBCAT$> contains subcatchment data      
       Channel 9 <PARAMS$> contains parameter data
       Channel 10 <OUTPUT$> is output file

  In addition
       Channel 12 <MAPFILE$> is used to read subcatchment ln(a/tanB)
                   maps if IMAP = 1


*****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

/* function/subroutine prototypes */
extern void init(FILE *in_param_fptr,FILE *output_fptr,char *subcat, 
              int num_channels,int num_topodex_values,
              int yes_print_output, double area,double **time_delay_histogram,
              double *cum_dist_area_with_dist,double dt, double *szm, double *t0,
              double tl, double *dist_from_outlet,double *td, double *srmax, 
              double *Q0,double *sr0, int *infex, double *xk0, double *hf, 
              double *dth,double **stor_unsat_zone,double **deficit_local,
              double **deficit_root_zone,double *szq, double *Q,double *sbar,
              int max_atb_increments, int max_time_delay_ordinates,
              double *bal,int *num_time_delay_histo_ords, int *nd);
                 
extern void inputs(FILE *input_fptr, int *nstep, double *dt, double **rain,
                double **pe, double **Qobs, double **Q, double **contrib_area);
                   
extern void topmod(FILE *fptr, int nstep, int num_topodex_values, 
                int yes_print_output, int infex, double dt,double szm,
                double *stor_unsat_zone, double *deficit_root_zone, 
                double *deficit_local, double *pe, double *rain,
                double xk0, double hf, double *dist_area_lnaotb, 
                double tl, double *st, double td,double srmax, double *contrib_area,
                double szq, double *Qout, int num_time_delay_histo_ords, 
                double *Q,double *time_delay_histogram, char *subcat,
                double *bal, double *sbar,int nd);

extern void tread(FILE *subcat_fptr,FILE *output_fptr,char *subcat, 
               int *num_topodex_values,int *num_channels,double *area,
               double **dist_area_lnaotb, double **st, int yes_print_output,
               double **cum_dist_area_with_dist, double *tl, 
               double **dist_from_outlet, int maxsubcatch,int maxincr);
                  
extern void expinf(int irof, int it, int rint, double df, double cumf,
                   double dt,double xk0, double szm, double hf);                  

extern void results(FILE *output_fptr, FILE *out_hyd_fptr,
                 int ntstep, double *Qobs, double *Q);

extern void itwo_alloc( int ***ptr, int x, int y);
extern void dtwo_alloc( double ***ptr, int x, int y);
extern void d_alloc(double **var,int size);
extern void i_alloc(int **var,int size);



main()
{
FILE *control_fptr;  /* this file must be named topmodel.run */
FILE *input_fptr;   
FILE *subcat_fptr;  
FILE *params_fptr;  
FILE *output_fptr;
FILE *out_hyd_fptr; 

char input_fname[30],subcat_fname[30],params_fname[30],output_fname[30];
char out_hyd_fname[30];

/************* Variable definitions ***********************/
char title[257];         /* the title of the simulation */
char subcat[257];        /* the name of each sub-catchment */
double dt;               /* time step in hours */
int nstep;               /* total number of simulation periods */
int yes_print_output;    /* this is set equal to 1 to print output stuff */
int imap;                /* ordinarily tells code to write map.  NOT IMPLEMENTED HERE */
int num_channels;        /* number of channels */
int num_topodex_values;  /* number of topodex histogram values */
int infex;               /* set to 1 to call subroutine to do infiltration excess calcs. */
                         /* this is not usually appropriate in catchments where TOPMODEL */
                         /* is applicable (shallow highly permeable soils) */

/* model parameters and input scalars */
double szm;   /* this is the famous m parameter */
double t0;    /* this is the areal average of ln(a/tanB)  */
double td;    /* unsaturated zome time delay per unit storage deficit */
double srmax; /* maximum root zone storage deficit */
double Q0;    /* initial subsurface flow per unit area */
double sr0;   /* initial root zone storage deficit */
double xk0;   /* surface soil hydraulic conductivity */
double hf;    /* wetting front suction for G&A soln.  */
double dth;   /* water content change across the wetting front */
double area;  /* catchment area */

/* other variables of note */
int num_delay;         /* number of time steps lag (delay) in channel within catchment to outlet */
int num_time_delay_histo_ords; /* number of time delay histogram ordinates */


double szq;              /* an important parameter, I don't know how to describe it */
double tl;               /* another important parameter that excapes description, time lag?*/
double max_contrib_area; /* the max. contributing area during a simulation */
double bal;              /* the residual of the water balance */
double sbar;             /* the catchment average soil moisture deficit */

/*** THESE POINTERS WILL BE DIMENSIONED DYNAMICALLY TO BECOME 1-D ARRAYS *****/
double *Q=NULL;                 /* simulated discharge */
double *Qobs=NULL;              /* observed discharge */
double *rain=NULL;              /* rainfall rate */
double *pe=NULL;                /* potential evapotranspiration */
double *contrib_area=NULL;      /* contributing area */
double *stor_unsat_zone=NULL;   /* storage in the unsat. zone */
double *deficit_root_zone=NULL; /* root zone storage deficit */
double *deficit_local=NULL;     /* local storage deficit */
double *time_delay_histogram=NULL;  /* time lag of outflows due to channel routing */
double *dist_area_lnaotb=NULL;  /* the distribution of area corresponding to ln(A/tanB) histo.*/
double *lnaotb=NULL;            /* these are the ln(a/tanB) values */
double *cum_dist_area_with_dist=NULL;  /* channel cum. distr. of area with distance */
double *dist_from_outlet=NULL;  /* distance from outlet to point on channel with area known */

/* other internal vars. */
int num_sub_catchments;
int max_atb_increments;
int max_num_subcatchments;
int max_time_delay_ordinates;
int isc;

/* returns */
double Qout;

/* hard coded memory-related parameters */
max_atb_increments=30;
max_num_subcatchments=10;
max_time_delay_ordinates=20;



if((control_fptr=fopen("topmod.run","r"))==NULL)
  {printf("Can't open control file named topmod.run\n");exit(-9);}

fgets(title,256,control_fptr);
fscanf(control_fptr,"%s",input_fname);
fscanf(control_fptr,"%s",subcat_fname);
fscanf(control_fptr,"%s",params_fname);
fscanf(control_fptr,"%s",output_fname);
fscanf(control_fptr,"%s",out_hyd_fname);

if((input_fptr=fopen(input_fname,"r"))==NULL)
  {printf("Can't open input file named %s\n",input_fname);exit(-9);}

if((subcat_fptr=fopen(subcat_fname,"r"))==NULL)
  {printf("Can't open subcat file named %s\n",subcat_fname);exit(-9);}

if((params_fptr=fopen(params_fname,"r"))==NULL)
  {printf("Can't open params file named %s\n",params_fname);exit(-9);}

if((output_fptr=fopen(output_fname,"w"))==NULL)
  {printf("Can't open output file named %s\n",output_fname);exit(-9);}

if((out_hyd_fptr=fopen(out_hyd_fname,"w"))==NULL)
  {printf("Can't open output file named %s\n",out_hyd_fname);exit(-9);}


fprintf(output_fptr,"%s\n",title);
printf("TOPMODEL Version: TMOD95.02\n");
printf("This run: %s\n",title);


/*  READ IN nstep, DT and RAINFALL, PE, QOBS INPUTS */
/*  allocate memory for arrays here too */
inputs(input_fptr, &nstep, &dt, &rain, &pe, &Qobs, &Q, &contrib_area);

/*  READ IN SUBCATCHMENT TOPOGRAPHIC DATA */
fscanf(subcat_fptr,"%d %d %d,",&num_sub_catchments,&imap,&yes_print_output);

/*  START LOOP ON SUBCATCHMENTS */
for(isc=1;isc<=num_sub_catchments;isc++)
  {
  if(yes_print_output==TRUE) fprintf(output_fptr,"Starting Subcatchment %d\n",isc);
  
  /*  INITIALISATION FOR THIS SUBCATCHMENT */
  tread(subcat_fptr,output_fptr,subcat,&num_topodex_values,&num_channels,
        &area,&dist_area_lnaotb,&lnaotb,yes_print_output,
        &cum_dist_area_with_dist,&tl,&dist_from_outlet,
        max_num_subcatchments,max_atb_increments);
  
  init(params_fptr,output_fptr,subcat,num_channels,num_topodex_values,
       yes_print_output,area,&time_delay_histogram,cum_dist_area_with_dist,
       dt,&szm,&t0,tl,dist_from_outlet,&td, &srmax,&Q0,&sr0,&infex,&xk0,&hf,
       &dth,&stor_unsat_zone,&deficit_local,&deficit_root_zone,
       &szq,Q,&sbar,max_atb_increments,max_time_delay_ordinates,
       &bal,&num_time_delay_histo_ords,&num_delay);

  /*  RUN MODEL FOR THIS SUBCATCHMENT INCLUDING LINEAR ROUTING CALCULATIONS */
  topmod(output_fptr,nstep,num_topodex_values,yes_print_output,infex,dt,szm,
         stor_unsat_zone,deficit_root_zone,deficit_local,pe,rain,xk0,hf,
         dist_area_lnaotb,tl,lnaotb,td,srmax,contrib_area,szq,&Qout,
         num_time_delay_histo_ords,Q,time_delay_histogram,subcat,&bal,&sbar,
         num_delay);  

  /*  END LOOP ON SUBCATCHMENTS */
  }

/*  CALL RESULTS ROUTINE:  if IRUN = 0 on return stop */
results(output_fptr,out_hyd_fptr,nstep, Qobs, Q);

fclose(control_fptr);
fclose(input_fptr);
fclose(subcat_fptr);
fclose(params_fptr);
fclose(output_fptr);
fclose(out_hyd_fptr);

return;
}

extern void topmod(FILE *output_fptr, int nstep, int num_topodex_values,
                int yes_print_output,int infex, double dt, double szm,
                double *stor_unsat_zone, double *deficit_root_zone,
                double *deficit_local, double *pe, double *rain,double xk0,double hf, 
                double *dist_area_lnaotb, double tl, double *lnaotb, double td,
                double srmax, double *contrib_area, double szq, double *Qout, 
                int num_time_delay_histo_ords,double *Q,
                double *time_delay_histogram,char *subcat,double *bal,
                double *sbar,int num_delay)
{
/*****************************************************************

  THIS ROUTINE RUNS TOPMODEL FOR ONE SUBCATCHMENT, INCLUDING THE
  LINEAR CHANNEL ROUTING CALCULATIONS.

  The calculations are made for areal subdivisions based on the
  num_topodex_values ln(a/tanB) subdivisions.  The saturation deficit for each 
  subdivision is calculated from SBAR at the start of each time
  step.

  Each increment also has a root zone storage (deficit_root_zone) deficit which
  is 0 at 'field capcacity' and becomes more positive as the soil
  dries out; and an unsaturated zone storage (stor_unsat_zone) which is zero at
  field capacity and becomes more positive as storage increases.
  stor_unsat_zone has an upper limit of the local saturation deficit deficit_local.
  The local contributing area is where deficit_local - stor_unsat_zone is less than or
  equal to zero.

  REMEMBER SBAR,deficit_local AND deficit_root_zone VALUES ARE DEFICITS; stor_unsat_zone IS A STORAGE.

******************************************************************/
double ex[31];
int ia,ib,in,irof,it,ir;
double rex,cumf,max_contrib_area,sump,sumae,sumq,qof,quz,ea,ep,p,rint,acm,df;
double acf,uz,sae,qb,of,sumrz,sumuz;

irof=0;
rex=0.0;
cumf=0.0;
max_contrib_area=0.0;
sump=0.0;
sumae=0.0;
sumq=0.0;
sae=0.0;


/*  START LOOP ON TIME STEPS */
if(yes_print_output==TRUE)
  {
  fprintf(output_fptr,
 "it      p        ep       q(it)       quz      q       sbar       qof\n");
  }
for(it=1;it<=nstep;it++)
  {
  qof=0.0;
  quz=0.0;
  ep=pe[it];  
  p=rain[it];
  sump+=p;

  /* SKIP INFILTRATION EXCESS CALCULATIONS IF INFEX = 0 */
  if(infex==1)
    {
    /****************************************************************
      INFILTRATION EXCESS CALCULATIONS USING EXPINF ROUTINE BASED ON
      GREEN-AMPT INFILTRATION IN A SOIL WITH CONDUCTIVITY DECLINING
      EXPONENTIALLY WITH DEPTH (REF. BEVEN, HSJ, 1984)

      NOTE THAT IF INFILTRATION EXCESS DOES OCCUR IT WILL DO SO OVER
      THE WHOLE SUBCATCHMENT BECAUSE OF HOMOGENEOUS SOIL ASSUMPTION

      ALL PARAMETERS AND VARIABLES ON INPUT MUST BE IN M/H

      THIS SECTION CAN BE OMITTED WITHOUT PROBLEM
      *************************************************************/
    if(p>0.0)
      {
      /*  Adjust Rainfall rate from m/time step to m/h */
      rint=p/dt;
      expinf(irof,it,rint,df,cumf,dt,xk0,szm,hf);
      /*  DF is volumetric increment of infiltration and is returned in m/DT */
      rex=p-df;
      p-=rex;
      }
    else
      {
      rex=0.0;
      irof=0;
      cumf=0.0;
      }
    }
  /****************************************************************
  /* P IS RAINFALL AVAILABLE FOR INFILTRATION AFTER SURFACE CONTROL */
  /*   CALCULATION */

  acm=0.0;
  /*  START LOOP ON A/TANB INCREMENTS */
  for(ia=1;ia<=num_topodex_values;ia++)
    {
    acf=0.5*(dist_area_lnaotb[ia]+dist_area_lnaotb[ia+1]);
    uz=0.0;
    ex[ia]=0.0;

    /*  CALCULATE LOCAL STORAGE DEFICIT */
    deficit_local[ia]=(*sbar)+szm*(tl-lnaotb[ia]);
    if(deficit_local[ia]<0.0) deficit_local[ia]=0.0;

    /*  ROOT ZONE CALCULATIONS */
    deficit_root_zone[ia]-=p;
    if(deficit_root_zone[ia]<0.0)
      {
      stor_unsat_zone[ia]-=deficit_root_zone[ia];
      deficit_root_zone[ia]=0.0;
      }

    /*  UZ CALCULATIONS */
    if(stor_unsat_zone[ia]>deficit_local[ia])
      {
      ex[ia]=stor_unsat_zone[ia]-deficit_local[ia];
      stor_unsat_zone[ia]=deficit_local[ia];
      }

    /*  CALCULATE DRAINAGE FROM stor_unsat_zone */
    if(deficit_local[ia]>0.0)
      {
      uz=stor_unsat_zone[ia]/(deficit_local[ia]*td*dt);
      if(uz>stor_unsat_zone[ia]) uz=stor_unsat_zone[ia];
      stor_unsat_zone[ia]-=uz;
      if(stor_unsat_zone[ia]<0.0000001) stor_unsat_zone[ia]=0.0;
      quz+=uz*acf;
      }
      

    /***************************************************************/
    /*  CALCULATE EVAPOTRANSPIRATION FROM ROOT ZONE DEFICIT */

    ea=0.0;
    if(ep>0.0)
      {
      ea=ep*(1.0-deficit_root_zone[ia]/srmax);
      if(ea>(srmax-deficit_root_zone[ia])) ea=srmax-deficit_root_zone[ia];
      deficit_root_zone[ia]+=ea;
      }
    sumae+=ea*acf;
    sae+=ea*acf;
    
    /***************************************************************
      CALCULATION OF FLOW FROM FULLY SATURATED AREA
      This section assumes that a/tanB values are ordered from high to low
      ****************************************************************/
    of=0.0;
    if(ia>1)
      {
      ib=ia-1;
      if(ex[ia]>0.0)
        {
        /*  Both limits are saturated */
        of=dist_area_lnaotb[ia]*(ex[ib]+ex[ia])/2.0;
        acm+=acf;
        }
      else
        {
        /*  Check if lower limit saturated (higher a/tanB value) */
        if(ex[ib]>0.0)
          {
          acf*=ex[ib]/(ex[ib]-ex[ia]);
          of=acf*ex[ib]/2.0;
          acm+=acf;
          }
        }
      }
    qof+=of;

    /*  Set contributing area plotting array */
    contrib_area[it]=acm;
    if(acm>max_contrib_area) max_contrib_area=acm;
    
    /* END OF A/TANB LOOP */
    }

  /*  ADD INFILTRATION EXCESS RUNOFF */
  qof+=rex;

  if(irof==1) max_contrib_area=1.0;

  /*  CALCULATE SATURATED ZONE DRAINAGE */
  qb=szq*exp(-(*sbar)/szm);
  (*sbar)+=(-quz+qb);
  *Qout=qb+qof;
  sumq+=(*Qout);

  /*  CHANNEL ROUTING CALCULATIONS */
  /*  allow for time delay to catchment outlet num_delay as well as  */
  /*  internal routing array */
  for(ir=1;ir<=num_time_delay_histo_ords;ir++)
    {
    in=it+num_delay+ir-1;
    if(in>nstep) break;
    Q[in]+=(*Qout)*time_delay_histogram[ir];
    }

  if(yes_print_output==TRUE && in<=nstep)
    { 
    fprintf(output_fptr,"%d %6.4e %6.4e %6.4e %6.4e %6.4e %6.4e %6.4e\n",
            it, p, ep, Q[it], quz, qb, (*sbar), qof);
    }
  }
/*  END OF TIME STEP LOOP */

/*  CALCULATE BALANCE TERMS */
sumrz=0.0;
sumuz=0.0;
for(ia=1;ia<=num_topodex_values;ia++)
  {
  acf=0.5*(dist_area_lnaotb[ia]+dist_area_lnaotb[ia+1]);
  sumrz+=deficit_root_zone[ia]*acf;
  sumuz+=stor_unsat_zone[ia]*acf;
  }
(*bal)+=(*sbar)+sump-sumae-sumq+sumrz-sumuz;
printf("\nWater Balance for Subcatchment: %s\n",subcat);
printf(
"   SUMP       SUMAE      SUMQ       SUMRZ      SUMUZ      SBAR        BAL\n");
printf("%6.3e  %6.3e  %6.3e  %6.3e  %6.3e  %6.3e  %6.3e\n",
        sump,sumae,sumq,sumrz,sumuz,(*sbar),(*bal));
        
fprintf(output_fptr,"\nWater Balance for Subcatchment: %s\n",subcat);
fprintf(output_fptr,
"   SUMP       SUMAE      SUMQ       SUMRZ      SUMUZ      SBAR        BAL\n");
fprintf(output_fptr,"%6.3e  %6.3e  %6.3e  %6.3e  %6.3e  %6.3e  %6.3e\n",
        sump,sumae,sumq,sumrz,sumuz,(*sbar),(*bal));

if(yes_print_output==TRUE)
  {
  fprintf(output_fptr,"Maximum contributing area %12.5lf\n",max_contrib_area);
  }
return;
}

extern void inputs(FILE *input_fptr, int *nstep, double *dt, double **r,
                   double **pe, double **Qobs, double **Q, double **contrib_area)
{
/***************************************************************
*
      SUBROUTINE INPUTS
*
*  This subroutine must read in rainfall, pe and observed 
*  discharges for T = 1,NSTEP with time step DT hours
****************************************************************/
int i;
fscanf(input_fptr,"%d %lf",nstep,dt);

/* allocate memory for arrays */
d_alloc(r,(*nstep));
d_alloc(pe,(*nstep));
d_alloc(Qobs,(*nstep));
d_alloc(Q,(*nstep));
d_alloc(contrib_area,(*nstep));

for(i=1;i<=(*nstep);i++)
  {
  fscanf(input_fptr,"%lf %lf %lf",&(*r)[i],&(*pe)[i],&(*Qobs)[i]);
  (*Q)[i]=0.0;
  }
return;
}

extern void tread(FILE *subcat_fptr,FILE *output_fptr,char *subcat, 
               int *num_topodex_values,int *num_channels,double *area,
               double **dist_area_lnaotb,double **lnaotb, int yes_print_output,
               double **cum_dist_area_with_dist,double *tl,
               double **dist_from_outlet, int max_num_subcatchments,
               int max_num_increments)
{
/**************************************************************

      SUBROUTINE TREAD

     READS TOPOGRAPHIC INDEX DIST. AND CHANNEL INFO
***************************************************************/ 
double tarea,sumac;
int j;

if((*cum_dist_area_with_dist)==NULL)
  {
  d_alloc(cum_dist_area_with_dist,max_num_subcatchments+1);
  d_alloc(dist_from_outlet,max_num_subcatchments+1);
  }
if((*dist_area_lnaotb)==NULL)
  {
  d_alloc(dist_area_lnaotb,max_num_increments+1);
  d_alloc(lnaotb,max_num_increments+1);
  }

fgets(subcat,256,subcat_fptr); /* do twice to read in line-feed */
fgets(subcat,256,subcat_fptr);
fprintf(output_fptr,"Subcatchment : %s\n",subcat);
fscanf(subcat_fptr,"%d %lf",num_topodex_values,area);

if((*num_topodex_values)>max_num_increments)
  {
  printf("Error, you have too many increments in your ln(a/tanB) file.\n");
  printf("  The maximum is: %d\n",max_num_increments);
  printf("program stopped.\n");
  exit(-9);
  }

/*  num_topodex_values IS NUMBER OF A/TANB ORDINATES */
/*  AREA IS SUBCATCHMENT AREA AS PROPORTION OF TOTAL CATCHMENT  */
for(j=1;j<=(*num_topodex_values);j++)
  {
  fscanf(subcat_fptr,"%lf %lf",&(*dist_area_lnaotb)[j],&(*lnaotb)[j]);
  }
/*  dist_area_lnaotb IS DISTRIBUTION OF AREA WITH LN(A/TANB) */
/*  lnaotb IS LN(A/TANB) VALUE */

tarea = (*dist_area_lnaotb)[1];
for(j=2;j<=(*num_topodex_values);j++)
  {
  tarea+=(*dist_area_lnaotb)[j];
  }

/*  CALCULATE AREAL INTEGRAL OF LN(A/TANB)
*  NB.  a/tanB values should be ordered from high to low with lnaotb[1]
*  as an upper limit such that dist_area_lnaotb[1] should be zero, with dist_area_lnaotb(2) representing
*  the area between lnaotb[1] and lnaotb[2] */

*tl=0.0;
(*dist_area_lnaotb)[1]/=tarea;
sumac=(*dist_area_lnaotb)[1];
for(j=2;j<=(*num_topodex_values);j++)
  {
  (*dist_area_lnaotb)[j]/=tarea;
  sumac+=(*dist_area_lnaotb)[j];
  (*tl)+=(*dist_area_lnaotb)[j]*((*lnaotb)[j]+(*lnaotb)[j-1])/2.0;
  }
(*dist_area_lnaotb)[(*num_topodex_values)+1]=0.0;

/*  READ CHANNEL NETWORK DATA */
fscanf(subcat_fptr,"%d",num_channels);
for(j=1;j<=(*num_channels);j++)
  {
  fscanf(subcat_fptr,"%lf %lf",
                 &(*cum_dist_area_with_dist)[j],&(*dist_from_outlet)[j]);
  }
/*  cum_dist_area_with_dist IS CUMULATIVE DISTRIBUTION OF AREA WITH dist_from_outlet */
/*  dist_from_outlet[1] is distance from subcatchment outlet */
/*  cum_dist_area_with_dist[1] = 0. */

if(yes_print_output==TRUE)
  {
  fprintf(output_fptr,"TL = %8.2lf\n",(*tl));
  fprintf(output_fptr,"SUMAC = %8.2lf\n",sumac);
  }
return;
}

extern void init(FILE *in_param_fptr,FILE *output_fptr,char *subcat, 
              int num_channels,int num_topodex_values,int yes_print_output,
              double area,double **time_delay_histogram,
              double *cum_dist_area_with_dist,double dt, double *szm, double *t0, 
              double tl, double *dist_from_outlet,double *td, double *srmax, 
              double *Q0,double *sr0, int *infex, double *xk0, double *hf, 
              double *dth,double **stor_unsat_zone,double **deficit_local,
              double **deficit_root_zone,double *szq, double *Q,
              double *sbar,int max_atb_increments, int max_time_delay_ordinates,
              double *bal,int *num_time_delay_histo_ords,int *num_delay)
{


/***************************************************************
      SUBROUTINE INIT

   READS PARAMETER DATA
******************************************/
double rv;   /* internal overland flow routing velocity */
double chv;  /* average channel flow velocity */
double tch[11],rvdt,chvdt,t0dt,time,a1,sumar;
double sum,ar2,a2;
int i,j,ia,in,ir;

if((*stor_unsat_zone)==NULL)
  {
  d_alloc(stor_unsat_zone,max_atb_increments);
  d_alloc(deficit_root_zone,max_atb_increments);
  d_alloc(deficit_local,max_atb_increments);
  }
if((*time_delay_histogram)==NULL)
  {
  d_alloc(time_delay_histogram,max_time_delay_ordinates);
  }

/* read in run parameters  */
fgets(subcat,256,in_param_fptr);
fscanf(in_param_fptr,"%lf %lf %lf %lf %lf %lf %lf %lf %d %lf %lf %lf",
       szm,t0,td,&chv,&rv,srmax,Q0,sr0,infex,xk0,hf,dth);

/*  Convert parameters to m/time step DT
*  with exception of XK0 which must stay in m/h
*                    Q0 is already in m/time step
                     T0 is input as Ln(To) */
rvdt=rv*dt;
chvdt=chv*dt;
t0dt=(*t0)+log(dt);  /* was ALOG */

/*  Calculate SZQ parameter */
(*szq)=exp(t0dt-tl);

/*  CONVERT DISTANCE/AREA FORM TO TIME DELAY HISTOGRAM ORDINATES */

tch[1]=dist_from_outlet[1]/chvdt;
for(j=2;j<=num_channels;j++)
  {
  tch[j]=tch[1]+(dist_from_outlet[j]-dist_from_outlet[1])/rvdt;
  }
(*num_time_delay_histo_ords)=(int)tch[num_channels];
if((double)(*num_time_delay_histo_ords)<tch[num_channels]) 
   {
   (*num_time_delay_histo_ords)++;
   }
(*num_delay)=(int)tch[1];
(*num_time_delay_histo_ords)-=(*num_delay);
for(ir=1;ir<=(*num_time_delay_histo_ords);ir++)
  {
  time=(double)(*num_delay)+(double)ir;
  if(time>tch[num_channels])
    {
    (*time_delay_histogram)[ir]=1.0;
    }
  else
    {
    for(j=2;j<=num_channels;j++)
      {
      if(time<=tch[j])
        {
        (*time_delay_histogram)[ir]=
             cum_dist_area_with_dist[j-1]+
                (cum_dist_area_with_dist[j]-cum_dist_area_with_dist[j-1])*
                              (time-tch[j-1])/(tch[j]-tch[j-1]);
        break;  /* exits this for loop */
        }
      }
    }
  }
a1=(*time_delay_histogram)[1];
sumar=(*time_delay_histogram)[1];
(*time_delay_histogram)[1]*=area;
if((*num_time_delay_histo_ords)>1)
  {
  for(ir=2;ir<=(*num_time_delay_histo_ords);ir++)
    {
    a2=(*time_delay_histogram)[ir];
    (*time_delay_histogram)[ir]=a2-a1;
    a1=a2;
    sumar+=(*time_delay_histogram)[ir];
    (*time_delay_histogram)[ir]*=area;
    }
  }
if(yes_print_output==TRUE)
  {
  fprintf(output_fptr,"SZQ =  %12.5lf\n",(*szq));
  fprintf(output_fptr,"Subcatchment routing data:\n");
  fprintf(output_fptr,"Maximum Routing Delay  %12.5lf\n",tch[num_channels]);
  fprintf(output_fptr,"Sum of Histogram ordinates: %10.4lf\n",sumar);
  for(ir=1;ir<=(*num_time_delay_histo_ords);ir++)
    {
    fprintf(output_fptr,"%12.5lf ",(*time_delay_histogram)[ir]);
    }
  fprintf(output_fptr,"\n");
  }

/*  INITIALISE deficit_root_zone AND Q0 VALUES HERE
 *  SR0 IS INITIAL ROOT ZONE STORAGE DEFICIT BELOW FIELD CAPACITY
 *  Q0 IS THE INITIAL DISCHARGE FOR THIS SUBCATCHMENT  
 *
 *  INITIALISE STORES */
for(ia=1;ia<=num_topodex_values;ia++)
  {
  (*stor_unsat_zone)[ia]=0.0;
  (*deficit_root_zone)[ia]=(*sr0);
  }
(*sbar)=-(*szm)*log((*Q0)/(*szq));

/*   Reinitialise discharge array */
sum=0.0;
for(i=1;i<=(*num_delay);i++)
  {
  Q[i]+=(*Q0)*area;
  }
for(i=1;i<=(*num_time_delay_histo_ords);i++)
  {
  sum+=(*time_delay_histogram)[i];
  in=(*num_delay)+i;
  Q[in]+=(*Q0)*(area-sum);
  }

/*  Initialise water balance.  BAL is positive for storage */
(*bal)=-(*sbar)-(*sr0);
if(yes_print_output==TRUE)
  {
  fprintf(output_fptr,"Initial Balance BAL %12.5f\n",(*bal));
  fprintf(output_fptr,"Initial SBAR        %12.5f\n",(*sbar));
  fprintf(output_fptr,"Initial SR0         %12.5f\n",(*sr0));
  }
return;
}


extern void expinf(int irof, int it, int rint, double df, double cumf,
                   double dt,double xk0, double szm, double hf)
{
/**************************************************************

      SUBROUTINE EXPINF(IROF,IT,RINT,DF,CUMF)
   
  SUBROUTINE TO CALCULATE INFILTRATION EXCESS RUNOFF USING THE 
  EXPONENTIAL DECREASE IN KSAT WITH DEPTH GREEN-AMPT MODEL. 

**************************************************************/
double sum,fc,func,cd,xke,e,tp,r2,f1;
double dfunc,fx,add,fac,constant,f,f2,xkf,szf,dth;
int i,j;

e=0.00001;

/*  Note that HF and DTH only appear in product CD */
cd=hf*dth;
szf=1.0/szm;
xkf=xk0;

if(irof!=1)
  {
  /* ponding has not yet occured */   
  if(cumf>0.0)
    {
    /*  FIRST TIME STEP, OVERFLOW IF CUMF=0, GO DIRECT TO F2 CALCULATION */
    /*  INITIAL ESTIMATE OF TIME TO PONDING */
    f1=cumf;
    r2=-xkf*szf*(cd+f1)/(1.0-exp(szf*f1));
    if(r2<rint)
      {
      /*  PONDING STARTS AT BEGINNING OF TIME STEP */
      tp=((double)it-1.0)*dt;
      irof=1;
      f=cumf;
      }
    }
  if(irof!=1)
    {
    f2=cumf+dt*rint;
    r2=-xkf*szf*(cd+f2)/(1.0-exp(szf*f2));
    if(fabs(f2)<1.0e-09 || r2>rint )
      {
      irof=0;
      df=rint*dt;
      cumf+=df;
      return;  /* no ponding during time step */
      }
    f=cumf+r2*dt;
    for(i=1;i<=20;i++)
      {    
      r2=-xkf*szf*(cd+f)/(1.0-exp(szf*f));
      if(r2>rint)
        {
        f1=f;
        f=(f2+f)*0.5;
        if(fabs(f-f1)<e) break;
        }
      else
        {
        f2=f;
        f=(f1+f)*0.5;
        if(fabs(f-f2)<e) break;
        }
      if(i==20)
        {
        printf("Max number of iterations exceeded\n");
        exit(-9);  /* stop the program */
        }
      }
    tp=((double)it-1.0)*dt+(f-cumf)/rint;
    }
  if(tp<=(double)it*dt)
    {
    /* IF(TP.GT.IT*DT)GO TO 20 */
    /*  SET UP DEFINITE INTEGRAL CONSTANT USING FPC */
    constant=0.0;
    fac=1.0;
    fc=(f+cd);
    for(j=1;j<=10;j++)
      {
      fac=fac*(double)j;
      add=pow((fx*szf),(double)j)/((double)j*fac);
      constant+=add;
      }
    constant=log(fc)-(log(fc)+constant)/exp(szf*cd);
    irof=1;
    f+=0.5*rint*((double)it*dt-tp);
    }
  }
/*  EXCESS CALCULATION */
/*  NEWTON-RAPHSON SOLUTION FOR F(T) */
if(tp<=(double)it*dt)
  {
  for(i=1;i<=20;i++)
    {
    /*  CALCULATE SUM OF SERIES TERMS */
    fc=(f+cd);
    sum=0.0;
    fac=1.0;
    for(j=1;j<=10;j++)
      {
      fac=fac*(double)j;
      add=pow((fc*szf),(double)j)/((double)j*fac);
      sum+=add;
      }
    func=-(log(fc)-(log(fc)+sum)/exp(szf*cd)-constant)/(xkf*szf)-
                                                        ((double)it*dt-tp);
    dfunc=(exp(szf*f)-1.0)/(xkf*szf*fc);
    df=-func/dfunc;
    f+=df;
    if(fabs(df)<e) break;
    if(i==20)
      {
      printf("Max number of iterations exceeded\n");
      exit(-9);  /* stop the program */
      }
    }
  if(f<(cumf+rint))
    {
    df=f-cumf;
    cumf=f;
    /*  SET UP INITIAL ESTIMATE FOR NEXT TIME STEP */
    f+=df;
    return;
    }
  }
return;
}

extern void results(FILE *output_fptr, FILE *out_hyd_fptr,
                 int nstep, double *Qobs, double *Q)
{
/***************************************************************
      SUBROUTINE RESULTS
      
         PRINTS RESULTS
  AND DOES OBJECTIVE FUNCTION CALCULATIONS 
  *************************************************************/
  
double f1,f2,sumq,ssq,vare,varq,qbar,nse;
int it;

f1=0.0;
f2=0.0;
sumq=0.0;
ssq=0.0;
for(it=1;it<=nstep;it++)
  {
  sumq+=Qobs[it];
  ssq+=Qobs[it]*Qobs[it];
  f1+=pow((Q[it]-Qobs[it]),2.0);
  f2=f2 + fabs(Q[it]-Qobs[it]);
  fprintf(out_hyd_fptr,"%d %lf %lf\n",it,Qobs[it],Q[it]);
  }
qbar=sumq/(double)nstep;
varq=(ssq/(double)nstep -qbar*qbar);
vare=f1/(double)nstep;
nse=1.0-vare/varq;

printf("Objective function values:\n");
printf("SSE %e    NSE %7.5lf   F2 %e\n",f1,nse,f2);
printf("Mean Obs Q %e   Variance Obs Q %e\n",qbar,varq);
printf("    Error Variance %e\n",vare);

fprintf(output_fptr,"Objective function values:\n");
fprintf(output_fptr,"F1 %e    NSE %7.5lf   F2 %e\n",f1,nse,f2);
fprintf(output_fptr,"Mean Obs Q %e   Variance Obs Q %e\n",qbar,varq);
fprintf(output_fptr,"    Error Variance %e\n",vare);
return;
}

/****************************************/
/*  MEMORY HANDLING ROUTINES BY FLO     */
/****************************************/

/*#############*/
void itwo_alloc(int ***array,int rows, int cols)
{
/*sub allocates memory for a two-dimensional integer array */
int  i,frows,fcols, numgood=0;
int error=0;

if ((rows==0)||(cols==0))
  {
  printf("Error: Attempting to allocate array of size 0\n");
  exit(-9);
  }

frows=rows+1;  /* added one for FORTRAN numbering */
fcols=cols+1;  /* added one for FORTRAN numbering */

*array=(int **)malloc(frows*sizeof(int *));
if (*array) 
  {
  memset((*array), 0, frows*sizeof(int*));
  for (i=0; i<frows; i++)
    {
    (*array)[i] =(int *)malloc(fcols*sizeof(int ));
    if ((*array)[i] == NULL)
      {
      error = 1;
      numgood = i;
      i = frows;
      }
     else memset((*array)[i], 0, fcols*sizeof(int )); 
     }
   }
return;
}


/*#############*/
void dtwo_alloc(double ***array,int rows, int cols)
{
/*sub allocates memory for a two-dimensional double-precision array */
int  i,frows,fcols, numgood=0;
int error=0;

if ((rows==0)||(cols==0))
  {
  printf("Error: Attempting to allocate array of size 0\n");
  exit(-9);
  }

frows=rows+1;  /* added one for FORTRAN numbering */
fcols=cols+1;  /* added one for FORTRAN numbering */

*array=(double **)malloc(frows*sizeof(double *));
if (*array) 
  {
  memset((*array), 0, frows*sizeof(double *));
  for (i=0; i<frows; i++)
    {
    (*array)[i] =(double *)malloc(fcols*sizeof(double ));
    if ((*array)[i] == NULL)
      {
      error = 1;
      numgood = i;
      i = frows;
      }
     else memset((*array)[i], 0, fcols*sizeof(double )); 
     }
   }
return;
}


/*#############*/
void d_alloc(double **var,int size)
{
/*sub allocates memory for a one-dimensional double precision array */
  size++;  /* just for safety */

   *var = (double *)malloc(size * sizeof(double));
   if (*var == NULL)
      {
      printf("Problem allocating memory for array in d_alloc.\n");
      return;
      }
   else memset(*var,0,size*sizeof(double));
   return;
}

/*#############*/
void i_alloc(int **var,int size)
{
/*sub allocates memory for a one-dimensional integer array */

   size++;  /* just for safety */

   *var = (int *)malloc(size * sizeof(int));
   if (*var == NULL)
      {
      printf("Problem allocating memory in i_alloc\n");
      return; 
      }
   else memset(*var,0,size*sizeof(int));
   return;
}

