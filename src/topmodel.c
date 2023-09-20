/*  TOPMODEL DEMONSTRATION PROGRAM VERSION 95.02

  In service of the NOAA-NWS Office of Water Prediction, this 
  version has been adapted to support a Basic Model Inferface
  (BMI), 2021.

  after C version by Fred Ogden, Sept. 2009.  Converted to ANSI C
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
#include "../include/topmodel.h"

void shift_Q(double* Q, int num_ordinates){ //NJF Really don't like the +1 size assumption here
  //memmove puts num_ordinates*sizeof(double) bytes starting at Q[1]
  //into Q[0], effectively shifting the array by one
  memmove(&Q[0], &Q[1], num_ordinates*sizeof(*Q));
  //set the last value to 0.0
  Q[num_ordinates] = 0.0;
}

extern void topmod(FILE *output_fptr, int nstep, int num_topodex_values,
                int yes_print_output,int infex, double dt, double szm,
                double *stor_unsat_zone, double *deficit_root_zone,
                double *deficit_local, double *pe, double *rain,double xk0,double hf, 
                double *dist_area_lnaotb, double tl, double *lnaotb, double td,
                double srmax, double *contrib_area, double szq, double *Qout, 
                int num_time_delay_histo_ords,double *Q,
                double *time_delay_histogram,char *subcat,double *bal,
                double *sbar,int num_delay, int current_time_step, int stand_alone,
                double *sump, double *sumae, double *sumq, double *sumrz, double *sumuz,
                double *quz, double *qb, double *qof, double *p, double *ep)
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

/* BMI Adaption: 
  current_time_step, *sump, *sumae, *sumq, stand_alone
  added as function input parameters */
//shift Q array to align current time step
shift_Q(Q, num_time_delay_histo_ords);

double ex[num_topodex_values+1]; //+1 to maintin 1 based array indexing
//NJF TODO consider warning on all program limits here since this is essentially
//"the model" where those assumptions may not be valid...
if(num_topodex_values > WARN_TOPODEX_INCREMENTS){
  printf("WARNING: num_topodex_values, %d, is greater than %d\n",
         num_topodex_values, WARN_TOPODEX_INCREMENTS);
}
int ia,ib,in,irof,it,ir;
double rex,cumf,max_contrib_area,ea,rint,acm,df;
double acf,uz,sae,of;

irof=0;
rex=0.0;
cumf=0.0;
max_contrib_area=0.0;
//sae=0.0;

/* BMI Adaption: START SINGLE TIME STEP ITERATION */
/* Note: original source code starts for-loop over nstep here
   Also recall that bmi adaptation removes submatchment loop
   which lived in topmod9502.c main() function */
if(yes_print_output==TRUE && current_time_step==1)
  {
  fprintf(output_fptr,
 "it      p        ep       q(it)       quz      q       sbar       qof\n");
  }

/* BMI Adaption: Set iteration to bmi's current_time_step (standalone) or 1 (framework)
  Counter++ is handled by bmi's update()*/

if (stand_alone == TRUE)it=current_time_step;
else it=1;  

*qof=0.0;
*quz=0.0;
*ep=pe[it];  
*p=rain[it];
*sump+=*p;  /* BMI Adaption: *sump now as pointer var; incl in model struct */

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
  if(*p>0.0)
    {
    /*  Adjust Rainfall rate from m/time step to m/h */
    rint=*p/dt;
    expinf(irof,it,rint,&df,&cumf,dt,xk0,szm,hf);
    /*  DF is volumetric increment of infiltration and is returned in m/DT */
    rex=*p-df;
    *p-=rex;
    }
  else
    {
    rex=0.0;
    irof=0;
    cumf=0.0;
    }
  }
/******************************************************************/
/* P IS RAINFALL AVAILABLE FOR INFILTRATION AFTER SURFACE CONTROL */
/*   CALCULATION */

acm=0.0;
/*  START LOOP ON A/TANB INCREMENTS */
//---------------------------------------
// Note: Loop count starts at 1, not 0!
//---------------------------------------
for(ia=1;ia<=num_topodex_values;ia++)
  {
  acf=0.5*(dist_area_lnaotb[ia]+dist_area_lnaotb[ia+1]);
  uz=0.0;
  ex[ia]=0.0;
  
  /*  CALCULATE LOCAL STORAGE DEFICIT */
  deficit_local[ia]=(*sbar)+szm*(tl-lnaotb[ia]);
  
  if(deficit_local[ia]<0.0) deficit_local[ia]=0.0;

  /*  ROOT ZONE CALCULATIONS */
  deficit_root_zone[ia]-=*p;
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
    *quz+=uz*acf;
    }
    
  /***************************************************************/
  /*  CALCULATE EVAPOTRANSPIRATION FROM ROOT ZONE DEFICIT */

  ea=0.0;
  if(*ep>0.0)
    {
    ea=*ep*(1.0-deficit_root_zone[ia]/srmax);
    if(ea>(srmax-deficit_root_zone[ia])) ea=srmax-deficit_root_zone[ia];
    deficit_root_zone[ia]+=ea;
    }
  *sumae+=ea*acf; /* BMI Adaption: *sumae now as pointer var; incl in model struct */
  //sae+=ea*acf;

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
  (*qof)+=of;

  /*  Set contributing area plotting array */
  contrib_area[it]=acm;
  if(acm>max_contrib_area) max_contrib_area=acm;
  
  /* END OF A/TANB LOOP */
  }

/*  ADD INFILTRATION EXCESS RUNOFF */
(*qof)+=rex;

if(irof==1) max_contrib_area=1.0;

/*  CALCULATE SATURATED ZONE DRAINAGE */
(*qb)=szq*exp(-(*sbar)/szm);
(*sbar)+=(-(*quz)+(*qb));
*Qout=(*qb)+(*qof);


/*  CHANNEL ROUTING CALCULATIONS */
/*  allow for time delay to catchment outlet num_delay as well as  */
/*  internal routing array */
//---------------------------------------
// Note: Loop count starts at 1, not 0!
//---------------------------------------
for(ir=1;ir<=num_time_delay_histo_ords;ir++)
  {
  in=it+num_delay+ir-1;
  if(in>current_time_step) break;
  //Accumulate previous time dealyed flow with current
  Q[in]+=(*Qout)*time_delay_histogram[ir];
  }
  //Add current time flow to mass balance variable
  *sumq += Q[it];
/* BMI Adaption: replace nstep with current_time_step */
if(yes_print_output==TRUE && in<=current_time_step)
  { 
  fprintf(output_fptr,"%d %6.4e %6.4e %6.4e %6.4e %6.4e %6.4e %6.4e\n",
          current_time_step, (*p), (*ep), Q[it], (*quz), (*qb), (*sbar), (*qof));
  }

/*  BMI Adaption: END SINGLE TIME STEP ITERATION 
    Note: original source code time-loop ends here*/
  
/*  CALCULATE BALANCE TERMS */
//---------------------------------------
// Note: Loop count starts at 1, not 0!
//---------------------------------------
*sumrz=0.0;
*sumuz=0.0;
for(ia=1;ia<=num_topodex_values;ia++)
  {

  acf=0.5*(dist_area_lnaotb[ia]+dist_area_lnaotb[ia+1]);
  (*sumrz)+=deficit_root_zone[ia]*acf;
  (*sumuz)+=stor_unsat_zone[ia]*acf;

  }

return;

}

extern void water_balance(FILE *output_fptr, int yes_print_output, 
                char *subcat,double *bal, double *sbar, double *sump, 
                double *sumae, double *sumq, double *sumrz, double *sumuz)
{

(*bal)+=(*sbar)+(*sump)-(*sumae)-(*sumq)+(*sumrz)-(*sumuz);

#if TOPMODEL_DEBUG >=1  
  printf("\nWater Balance for Subcatchment: %s\n",subcat);
  printf(
  "   SUMP       SUMAE      SUMQ       SUMRZ      SUMUZ      SBAR        BAL\n");
  printf("%6.3e  %6.3e  %6.3e  %6.3e  %6.3e  %6.3e  %6.3e\n",
          (*sump),(*sumae),(*sumq),(*sumrz),(*sumuz),(*sbar),(*bal));
#endif

  if (yes_print_output==TRUE)
    {
    fprintf(output_fptr,"\nWater Balance for Subcatchment: %s\n",subcat);
    fprintf(output_fptr,
    "   SUMP       SUMAE      SUMQ       SUMRZ      SUMUZ      SBAR        BAL\n");
    fprintf(output_fptr,"%6.3e  %6.3e  %6.3e  %6.3e  %6.3e  %6.3e  %6.3e\n",
           (*sump),(*sumae),(*sumq),(*sumrz),(*sumuz),(*sbar),(*bal));
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

/* TODO: When running on large scales, modify to allocate size via 'num_time_delay_histo_ords'
Include this modification in a 'Macro' (#ifdef) allowing the user to choose 
  original author (Fred Ogden via Keith Beven), i.e. 'nstep', or 
  alternative for computational efficiency, i.e. 'num_time_delay_histo_ords'  
This change should be make by the AGU 2021 CONUS scale demonstration*/
d_alloc(r,(*nstep));
d_alloc(pe,(*nstep));
d_alloc(Qobs,(*nstep));
//NJF This is dangerous, there is no validation between nstep and
//number of historgram ordinates, which is used to iterate Q in later steps
//so this could easily overflow if nstep is smaller than historgram ords
d_alloc(Q,(*nstep)); //NJF TODO validate that this works correctly
                     //When used in "stand alone" mode
d_alloc(contrib_area,(*nstep));

//---------------------------------------
// Note: Loop count starts at 1, not 0!
//---------------------------------------
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
               double **dist_from_outlet)
{
/**************************************************************

      SUBROUTINE TREAD

     READS TOPOGRAPHIC INDEX DIST. AND CHANNEL INFO
***************************************************************/ 
double tarea,sumac;
int j;

// NJF This won't work unless the first line of subcat file has already been consumed
// Should probably document this behavior for this function...
fgets(subcat,256,subcat_fptr); /* do twice to read in line-feed */
fgets(subcat,256,subcat_fptr);

if (yes_print_output == TRUE) 
  {
    fprintf(output_fptr,"Subcatchment : %s\n",subcat);
  }  
fscanf(subcat_fptr,"%d %lf",num_topodex_values,area);

//Setup the topoindex arrays
if(*num_topodex_values > WARN_TOPODEX_INCREMENTS){
  printf("WARNING: Number of ln(a/tanB) increments,%d, is > than %d\n",
          *num_topodex_values,
          WARN_TOPODEX_INCREMENTS);
}
if((*dist_area_lnaotb)==NULL)
{
  //Need one extra value (0.0) at the end of this array to do
  //sum in topmod, e.g. dist_area_lnaotb[i] + dist_area_lnaotb[i+1]
  //for each increment
  d_alloc(dist_area_lnaotb, *num_topodex_values+1);
}
if((*lnaotb) == NULL){
  d_alloc(lnaotb, *num_topodex_values); //NJF why +1 in old code?
}
//NJF TODO if not NULL should probably free and then allocate?
//I don't see any reasonable expectation that these arrays should persist
//across calls to tread...

/*  num_topodex_values IS NUMBER OF A/TANB ORDINATES */
/*  AREA IS SUBCATCHMENT AREA AS PROPORTION OF TOTAL CATCHMENT  */
tarea = 0; //Accumulate area as it is read
for(j=1;j<=(*num_topodex_values);j++)
  {
  fscanf(subcat_fptr,"%lf %lf",&(*dist_area_lnaotb)[j],&(*lnaotb)[j]);
  tarea += (*dist_area_lnaotb)[j];
  }
/*  dist_area_lnaotb IS DISTRIBUTION OF AREA WITH LN(A/TANB) */
/*  lnaotb IS LN(A/TANB) VALUE */

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
//init last value to additive identity
(*dist_area_lnaotb)[(*num_topodex_values)+1]=0.0;

/*  READ CHANNEL NETWORK DATA */
fscanf(subcat_fptr,"%d",num_channels);

if(*num_channels > WARN_NUM_SUBCATCHMENTS){
  printf("WARNING: Number of channels, %d, is greater than %d\n",
         *num_channels,
         WARN_NUM_SUBCATCHMENTS);
}

if((*cum_dist_area_with_dist)==NULL){
  d_alloc(cum_dist_area_with_dist, *num_channels); //NJF why +1 in old code?
}
if((*dist_from_outlet) == NULL){
  d_alloc(dist_from_outlet, *num_channels); //NJF why +1 in old code?
}
//NJF TODO if not NULL should probably free and then allocate?
//I don't see any reasonable expectation that these arrays should persist
//calls to tread...

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


/**
 * Define functions to be used in init() and in Set_value in bmi_topmodel.c
 * to enable calibratable parameters to be updated.
 * BChoat 2023/08/29
 */


/** 
 * Function to convert distance/area form to time delay histogram ordinates
 * 
 * Converts parameters to m/time step DT
 * Based on the calculation of values going into output tch, it seems TOPMODEL assumes 
 * 	one main channel with up to 9 areas of overland flow contributing to the 
 * 	one main channel. tch[1] is the main channel, and tch[>1] are the areas of 
 * 	overland flow (BChoat).
 *
 * There is one assumed velocity for the main channel and one assumed velocity applied
 * 	to all overland flow areas. (BChoat)
 *
 * It is not clear why tch is hardcoded as an array of a fixed length. This is also done
 * 	in the original fortran code on which Fred Ogden based this C-code. In Fortran
 * 	it is coded as a 10-dimensional array and tch(10), and here as tch[11]. 
 * 	
 * 	TCH(10) in Fortran gives you a ten element array indexed from 1..10. When this was ported, 
 * 	the 1-based indexing was directly ported as well, so the tch buffer was over allocated by one, 
 * 	giving an array indexed 0..10, but index 0 is ignored/unsued. (BChoat)
 *
 * NJF Refactored to dynamically allocate tch based on the number of channels provided
 *
 * @params[in] dist_from_outlet, pointer to array of length num_channels of type double,
 * 	distance from outlet to point on channel with area known (i.e., to a channel; BChoat)
 * @params[in] num_channels, integer, how many channels are present (m/hr)
 * @params[in] CALIBRATABLE chv, pointer of type double and length one, the average channel velocity  (m/hr)
 * 	for the main channel (BChoat) 
 * @params[in] CALIBRATABLE rv, pointer of type double and length one, average internal overland flow routing velocity
 * @params[in] dt, double of length one, timestep in hours

 * @params[out] tch, double* of length num_channels, holds histogram ordinates for each channel (BChoat)
 * 	tch is used as input in subsequent functions
 * 	Note, position 0 is ignored. It was 
 * 	written this way when translated from the original code.
 */

extern void convert_dist_to_histords(const double * const dist_from_outlet, const int num_channels,
					const double * const chv, const double * const rv, const double dt, double* const tch)
{    
    //This function ASSUMES tch is overallocated by 1 and is indexable from (1, num_channels)
    //validate invariants
    if(num_channels < 1){
      printf("ERROR: convert_dist_to_histords, num channels < 1, must have at least one channel\n");
      exit(-1); //TODO return int error code and handle error externally
    }
    // declare local variables 
    double chvdt, rvdt;
    int j;
    
    chvdt = *chv*dt; // distance water travels in one timestep within channel
    rvdt = *rv*dt; //distance water travels as overland flow in one timestep

    tch[1]=dist_from_outlet[1]/chvdt;
    for(j=2;j<=num_channels;j++)
     {
     
      tch[j]=tch[1]+(dist_from_outlet[j]-dist_from_outlet[1])/rvdt;
     } 

    return;

}



/**
 * Function to calculate the time delay histogram
 *
 * @params[in] num_channels, int, defined in subcat.dat file
 * @params[in] area, double between 0 and 1 defining catchment area as ratio of 
 * 	entire catchment
 * @params[in] tch, double* of length num_channels, holds histogram ordinates for each channel
 * 	output from conver_dist_to_histords()
 * 	Note, position 0 is ignored. It was 
 * 	written this way when translated from the original code.
 * @params[in], cum_dist_area_with_dist, pointer of length num_channels-1 and type double,
 * 	cumulative distribution of area with dist_from_outlet. 

 * @params[out] num_time_delay_histo_ords, pointer of type int, number of time delay histogram ordinates
 * @params[out] num_delay, pointer of type int, number of time steps lag (delay) in channel within 
 * 	catchment to outlet.
 * @parms[out], time_delay_histogram, pointer of size and length max_time_delay_ordinates and type double, 
 * 	time lag of outflows due to channel routing
 */ 

extern void calc_time_delay_histogram(int num_channels, double area,
				double* tch, double *cum_dist_area_with_dist,
				int *num_time_delay_histo_ords, int *num_delay,	double **time_delay_histogram) 

{
    // declare local variables
    double time, a1, sumar, a2;
    int j, ir;

    // casting tch[num_channels] to int truncates tch[num_channels] 
    // (e.g., 7.9 becomes 7)
    //Determine how many ROUTING ORDINATES 
    //computed, essentially, by the `dist_from_outlet` of the FARTHEST channel segment
    (*num_time_delay_histo_ords)=(int)tch[num_channels];
    
    // this is here to round up. Since casting tch as int effectively rounds down, 
    // we add a value of 1 effectively rounding up.
    if((double)(*num_time_delay_histo_ords)<tch[num_channels]) 
       {
       (*num_time_delay_histo_ords)++;
       }
    //Determine the distance from outlet for first channel ordinate???
    (*num_delay)=(int)tch[1]; 
    (*num_time_delay_histo_ords)-=(*num_delay);

    if(*num_time_delay_histo_ords > WARN_HISTOGRAM_ORDINATES){
      printf("WARNING: number of time delay hisogram ordinates, %d, is greater than %d\n", 
            *num_time_delay_histo_ords,
            WARN_HISTOGRAM_ORDINATES);
    }

    if((*time_delay_histogram) == NULL){
      d_alloc(time_delay_histogram, *num_time_delay_histo_ords);
    } //FIXME always free/realloc
      //If not, the caller must ensure the correct size of time_delay_histogram prior to calling
  
    //NJF so we build histogram with ordinates between 1 and "distance" between first and last channel
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

    sumar = 0;
    //Convert ordinates to cummulative area, should sum to 1
    for(int i =  *num_time_delay_histo_ords; i > 1; i--){
      (*time_delay_histogram)[i] -= (*time_delay_histogram)[i-1]*area;
      (*time_delay_histogram)[i] *= area;
      sumar += (*time_delay_histogram)[i];
    }
    sumar += (*time_delay_histogram)[1];
    if(sumar < 0.99999 || sumar > 1.00001){
      printf("ERROR: Histogram oridnates do not sum to 1.\n");
      exit(-1); //FIXME this fuction should probably return an error code
                //and the error be handled elsewhere, not just an exit here...
    }

    return;
} 



/** 
 * Function to (re)initialize discharge array
 * @params[in] num_delay, pointer of type int, number of time steps lag (delay) in channel within 
 * 	catchment to outlet. Output from calc_time_delay_histogram
 * @params[in] Q0, pointer of type double, initial subsurface flow per unit area
 * @params[in] area, pointer of type double between 0 and 1 defining catchment area as ratio of
 * 	entire catchment
 * @params[in] num_time_delay_histo_ords, pointer of type int, number of time delay histogram ordinates,
 * 	output from calc_time_delay_histogram
 * @parms[in], time_delay_histogram, double pointer of type double, time lag of outflows due to channel routing,
 * 	output from calc_time_delay_histogram
 *
 * @params[out] Q, pointer of type double and length num_delay+num_time_delay_histo_ords, simulated discharge
 */

extern void init_discharge_array(int *num_delay, double *Q0, double area, 
			int *num_time_delay_histo_ords, double **time_delay_histogram,
                        double **Q)
{
    //if Q is already allocated, need to free and re-compute the required size
    if(*Q != NULL){
      free(*Q);
      *Q = NULL;
    }
    //*Q = calloc(*num_delay + *num_time_delay_histo_ords + 1, sizeof(double));
    d_alloc(Q, *num_delay + *num_time_delay_histo_ords);

    // declare local variables
    double sum;
    int i, in;
 
    sum=0.0;

    for(i=1;i<=(*num_delay);i++)
    {
      (*Q)[i]+=(*Q0)*area;
    }
          
    for(i=1;i<=(*num_time_delay_histo_ords);i++)
    {
      sum+=(*time_delay_histogram)[i];
      in=(*num_delay)+i;
      (*Q)[in]+=(*Q0)*(area-sum);
    };
    return;
}


/** 
 * Function to initialize unsaturated zone storage and deficit
 *
 * @params[in] num_topodex_values, int, number of topodex histogram values 
 * 	(i.e., number of A/TANB ordinates), defines the size of stor_unsat_zone, deficit_root_zone, and deficit_local
 * @params[in] dt, double, timestep in hours
 * @params[in] CALIBRATABLE sr0, pointer of type double, initial root zone storage deficit
 * @params[in] CALIBRATABLE szm, pointer of type double, exponential scaling parameter for decline
 * 	of transmissivity with increase in storage deficit
 * @params[in] Q0, pointer of type double, initial subsurface flow per unit area
 * @params[in] CALIBRATABLE t0, pointer of type double, downslope transmissivity when soil is just saturated
 * 	to the surface. 'input as LN(t0)
 * @params[in] tl, double, not well defined, but related to time lag

 * @params[out] stor_unsat_zone, double pointer of type double of size num_topodex_values,
 * 	storage in the unsaturated zone
 * @params[out] szq, pointer of type double, not well defined, but related to rate at which moisture
 * 	is lost from the subsruface.
 * @params[out] deficit_local, double pointer of type double of size num_topodex_values, local storage (or saturation) 
 * 	deficit
 * @params[out] deficit_root_zone, double pointer of type  double of size num_topodex_values, root zone storage deficit 
 * @params[out] sbar, pointer of type double, catchment average soil mositure deficit
 * 	edited dynamically as model steps through time.
 * @params[out] bal, pointer of type double, residual of water balance 
 */
 
extern void init_water_balance( 
				int num_topodex_values, double dt, double *sr0, 
				double *szm, double *Q0, double *t0, double tl,
				double **stor_unsat_zone, double *szq,
				double **deficit_local, 
        double **deficit_root_zone, double *sbar, 
				double *bal)
{
    // declare local variables
    double t0dt;
    int ia;

    if((*stor_unsat_zone) == NULL){
      d_alloc(stor_unsat_zone, num_topodex_values);
    }
    if((*deficit_root_zone) == NULL){
      d_alloc(deficit_root_zone, num_topodex_values);
    }
    if((*deficit_local) == NULL){
      d_alloc(deficit_local, num_topodex_values);
    }
    //FIXME realloc if any of these were NULL so they are guaranteed the correct size? Or carefully
    //document the assumption and the requirement for caller to size these arrays to num_topodex_values

    t0dt=(*t0)+log(dt);  /* was ALOG - specific log function in fortran*/

    /*  Calculate SZQ parameter */
    (*szq)=exp(t0dt-tl);

    for(ia=1;ia<=num_topodex_values;ia++)
      {
      (*stor_unsat_zone)[ia]=0.0;
      (*deficit_root_zone)[ia]=(*sr0);
      }

    (*sbar)=-(*szm)*log((*Q0)/(*szq));
    
    //  Initialise water balance.  BAL is positive for storage
    (*bal)=-(*sbar)-(*sr0);

    return;
}


/**
 * Main initialize function. Calls convert_dist_to_histords(), calc_time_delay_histogram(), 
 * 	init_water_balance(), and init_discharge_array()
 * 	
 * @params[in] in_param_fptr, FILE pointer, file with parameters (e.g., params.dat)
 * @params[in] output_fptr, FILE pointer, file to which output will be written (e.g., topmod-cat.out)
 * @params[in] subcat, pointer of type char, name of subcatchment, read in from in_param_fptr file
 * @params[in] num_channels, int, defined in subcat.dat file
 * @params[in] num_topodex_values, int, number of topodex histogram values 
 * 	(i.e., number of A/TANB ordinates)
 * @params[in] yes_print_output, int (boolean), 1 = TRUE; write output files, 0 = FALSE; do not write
 * @params[in] area, double between 0 and 1 defining catchment area as ratio of
 * 	entire catchment
 * @parms[in], time_delay_histogram, double pointer of type double, time lag of outflows due to channel routing,
 * 	output from calc_time_delay_histogram
 * @params[in], cum_dist_area_with_dist, pointer of type double, cumulative distribution of area with
 * 	dist_from_outlet
 * @params[in] dt, double, timestep in hours
 * @params[in] tl, double, not well defined, but related to time lag
 * @params[in] dist_from_outlet, pointer to array of length num_channels of type double,
 * 	distance from outlet to point on channel with area known (i.e., to a channel; BChoat)
 * @params[out] num_time_delay_histo_ords, pointer of type int, number of time delay histogram ordinates,
 * 	output from calc_time_delay_histogram
 * @params[out] num_delay, pointer of type int, number of time steps lag (delay) in channel within 
 * 	catchment to outlet. Output from calc_time_delay_histogram

 * The following 12 variables are read in from the params.dat file using fscanf
 * -----------------
 * @params[out] CALIBRATABLE szm, pointer of type double, exponential scaling parameter for decline
 * 	of transmissivity with increase in storage deficit
 * @params[out] CALIBRATABLE t0, pointer of type double, downslope transmissivity when soil is just saturated
 * 	to the surface. 'input as LN(t0)
 * @params[out] CALIBRATABLE td, pointer of type double, unsaturated zone time delay for recharge to the 
 * 	saturated zone per unit of deficit, provided in params.dat
 * 	Only read in within init(), not used.
 * @params[out] CALIBRATABLE chv, double of length one, the average channel velocity  (m/hr)
 * 	for main channel (BChoat) 
 * @params[out] CALIBRATABLE rv, double of length one, average internal overland flow routing velocity
 * @params[out] CALIBRATABLE srmax, pointer of type double, maximum root zone storage deficit provided in params.dat
 * 	Only read in within init(), not used.
 * @params[out] Q0, pointer of type double, initial subsurface flow per unit area
 * @params[out] CALIBRATABLE sr0, pointer of type double, initial root zone storage deficit
 * @params[out] infex, pointer of type int (boolean), 1 = TRUE; call subroutine to do infiltration excess calcs,
 * 	Not typically appropiate in catchments where TOPMODEL is applicable (i.e., shallow highly
 * 	permeable  soils). 0 = FALSE (default), 
 * 	Only read in within init(), not used. 
 * @params[out] xk0, pointer of type double, surface soil hydraulic conductivity.
 * 	Only read in within init(), not used.
 * @params[out] hf, pointer of type double, wetting from suction for G&A soln.
 * 	Only read in within init(), not used.
 * @params[out] dth, pointer of type double, Water content change across the wetting front
 * 	Only read in within init(), not used.
 * ----------------- 	

 * @params[out] stor_unsat_zone, double pointer of type double of size num_topodex_values,
 * 	storage in the unsaturated zone
 * @params[out] deficit_local, double pointer of type double of size num_topodex_values, local storage (or saturation) 
 * 	deficit
 * @params[out] deficit_root_zone, double pointer of type double of size num_topodex_valuesnum_topodex_values,
 * 	 root zone storage deficit 
 * @params[out] szq, pointer of type double, not well defined, but related to rate at which moisture
 * 	is lost from the subsurface.
 * @params[out] Q, pointer of type double of length num_delay, simulated discharge
 * @params[out] sbar, pointer of type double, catchment average soil mositure deficit
 * 	edited dynamically as model steps through time.
 * @params[out] bal, pointer of type double, residual of water balance 
 * 
 */
extern void init(FILE *in_param_fptr, FILE *output_fptr, char *subcat,
	      int num_channels, int num_topodex_values, int yes_print_output,
	      double area, double **time_delay_histogram,
	      double *cum_dist_area_with_dist, double dt, 
        double tl, double *dist_from_outlet,
        int *num_time_delay_histo_ords,int *num_delay,
	      double *szm, double *t0, double *chv, double *rv, double *td, double *srmax, 
        double *Q0,double *sr0, int *infex, double *xk0, double *hf, double *dth,
	      double **stor_unsat_zone, double **deficit_local,
        double **deficit_root_zone,double *szq, double **Q,
        double *sbar, double *bal)
{

/***************************************************************
      SUBROUTINE INIT

   READS PARAMETER DATA
******************************************/
double tch[num_channels+1]; //+1 to maintain 1 based indexing used by other routines
double sumar;
int ir;



/* read in run parameters  */
fgets(subcat,256,in_param_fptr);

printf("subcat: %s\n", subcat);

fscanf(in_param_fptr,"%lf %lf %lf %lf %lf %lf %lf %lf %d %lf %lf %lf",
       szm,t0,td,chv,rv,srmax,Q0,sr0,infex,xk0,hf,dth);


printf("\n\nCalibratable parameters from params*.dat:\n");

printf("\nET and recharge:\n");
printf("srmax = %f\n", *srmax);
printf("td = %f\n", *td);

printf("\nDischarge:\n");
printf("chv = %f\n", *chv);
printf("rv = %f\n", *rv);

printf("\nWater balance:\n");
printf("szm = %f\n", *szm);
printf("sr0 = %f\n", *sr0);
printf("t0 = %f\n", *t0);

//NJF num_channels is the value provided (SHOULD COME FROM TREAD)
//Convert distance/area form to time delay histogram ordinates
convert_dist_to_histords(dist_from_outlet, num_channels, chv, rv, dt, tch);

// calculate the time_delay_histogram
calc_time_delay_histogram(num_channels, area, tch, 
			  cum_dist_area_with_dist, num_time_delay_histo_ords,
			  num_delay, time_delay_histogram);


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


// Reinitialise discharge array
init_discharge_array(num_delay, Q0, area, 
			num_time_delay_histo_ords, time_delay_histogram, 
			Q);


// Initialize water balance and unsatrutaed storage and deficits
init_water_balance(num_topodex_values, 
				dt, sr0, szm, Q0, t0, tl,
				stor_unsat_zone, szq, 
				deficit_local, deficit_root_zone, sbar, bal);


if(yes_print_output==TRUE)
  {
  fprintf(output_fptr,"Initial BAL         %12.5f\n",(*bal));
  fprintf(output_fptr,"Initial SBAR        %12.5f\n",(*sbar));
  fprintf(output_fptr,"Initial SR0         %12.5f\n",(*sr0));
  }

return;
}


extern void expinf(int irof, int it, int rint, double* df, double* cumf,
                   double dt, double xk0, double szm, double hf)
{
/**************************************************************

      SUBROUTINE EXPINF(IROF,IT,RINT,DF,CUMF)
   
  SUBROUTINE TO CALCULATE INFILTRATION EXCESS RUNOFF USING THE 
  EXPONENTIAL DECREASE IN KSAT WITH DEPTH GREEN-AMPT MODEL. 

**************************************************************/

// BMI Adaption: df and cumf are now passed as pointers


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
  if(*cumf>0.0)
    {
    /*  FIRST TIME STEP, OVERFLOW IF CUMF=0, GO DIRECT TO F2 CALCULATION */
    /*  INITIAL ESTIMATE OF TIME TO PONDING */
    f1=*cumf;
    r2=-xkf*szf*(cd+f1)/(1.0-exp(szf*f1));
    if(r2<rint)
      {
      /*  PONDING STARTS AT BEGINNING OF TIME STEP */
      tp=((double)it-1.0)*dt;
      irof=1;
      f=*cumf;
      }
    }
  if(irof!=1)
    {
    f2=*cumf+dt*rint;
    r2=-xkf*szf*(cd+f2)/(1.0-exp(szf*f2));
    //--------------------------------------------------- 
    // Bug fix: "fabs(f2<1.0e-09) -> "fabs(f2)<1.0e-09"
    //---------------------------------------------------
    if( fabs(f2)<1.0e-09 || r2>rint )
      {
      irof=0;
      *df=rint*dt;
      *cumf+=*df;
      return;  /* no ponding during time step */
      }
    f=*cumf+r2*dt;
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
    tp=((double)it-1.0)*dt+(f-*cumf)/rint;
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
    *df=-func/dfunc;
    f+=*df;
    if(fabs(*df)<e) break;
    if(i==20)
      {
      printf("Max number of iterations exceeded\n");
      exit(-9);  /* stop the program */
      }
    }
  if(f<(*cumf+rint))
    {
    *df=f-*cumf;
    *cumf=f;
    /*  SET UP INITIAL ESTIMATE FOR NEXT TIME STEP */
    f+=*df;
    return;
    }
  }
return;
}

extern void results(FILE *output_fptr, FILE *out_hyd_fptr,
                 int nstep, double *Qobs, double *Q, 
                 int yes_print_output)
{
/***************************************************************
      SUBROUTINE RESULTS
      
         PRINTS RESULTS
  AND DOES OBJECTIVE FUNCTION CALCULATIONS 
  *************************************************************/

/* BMI Adaption: include additional function input parameters:
    yes_print_output
*/
  
  double f1,f2,sumq,ssq,vare,varq,qbar,nse;
  int it;
  f1=0.0;
  f2=0.0;
  sumq=0.0;
  ssq=0.0;

  for(it=1;it<=nstep;it++){
    sumq+=Qobs[it];
    ssq+=Qobs[it]*Qobs[it];
    f1+=pow((Q[it]-Qobs[it]),2.0);
    f2=f2 + fabs(Q[it]-Qobs[it]);
    if (yes_print_output == TRUE){
      fprintf(out_hyd_fptr,"%d %lf %lf\n",it,Qobs[it],Q[it]);
    }
  }
  qbar=sumq/(double)nstep;
  varq=(ssq/(double)nstep -qbar*qbar);
  vare=f1/(double)nstep;
  nse=1.0-vare/varq;

/* BMI Adaption: Console prints based on macro setting*/
#if TOPMODEL_DEBUG >=1  
  printf("Objective function values:\n");
  printf("SSE %e    NSE %7.5lf   F2 %e\n",f1,nse,f2);
  printf("Mean Obs Q %e   Variance Obs Q %e\n",qbar,varq);
  printf("    Error Variance %e\n",vare);
#endif

  /* BMI Adaption: All file prints based on yes_print_output*/
  if (yes_print_output == TRUE){
    fprintf(output_fptr,"Objective function values:\n");
    fprintf(output_fptr,"F1 %e    NSE %7.5lf   F2 %e\n",f1,nse,f2);
    fprintf(output_fptr,"Mean Obs Q %e   Variance Obs Q %e\n",qbar,varq);
    fprintf(output_fptr,"    Error Variance %e\n",vare);
  }

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
void d_alloc(double **var, int size)
{
/*sub allocates memory for a one-dimensional double precision array */

   // Note: Be aware of extra size when serializing.
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
void i_alloc(int **var, int size)
{
/*sub allocates memory for a one-dimensional integer array */

   // Note: Be aware of extra size when serializing.
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

