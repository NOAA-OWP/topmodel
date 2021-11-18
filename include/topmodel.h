#ifndef TOPMODEL_H
#define TOPMODEL_H

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <math.h>
#include <string.h>

#define TRUE  1
#define FALSE 0

// Print to console settings:
// 0: Nothing
// 1: Model info (source code)
// 2: BMI info (e.g.current timestep)
// Note: All errors causing program to exit will print console message
#define TOPMODEL_DEBUG 0

/*** Function/subroutine prototypes ***/
extern void init(FILE *in_param_fptr,FILE *output_fptr,char *subcat, 
                int num_channels,int num_topodex_values,
                int yes_print_output, double area,double **time_delay_histogram,
                double *cum_dist_area_with_dist,double dt, double *szm, double *t0,
                double tl, double *dist_from_outlet,double *td, double *srmax, 
                double *Q0,double *sr0, int *infex, double *xk0, double *hf, 
                double *dth,double **stor_unsat_zone,double **deficit_local,
                double **deficit_root_zone,double *szq, double *Q,double *sbar,
                int max_atb_increments, int max_time_delay_ordinates,
                double *bal,int *num_time_delay_histo_ords, int *num_delay);
                 
extern void inputs(FILE *input_fptr, int *nstep, double *dt, double **rain,
                double **pe, double **Qobs, double **Q, double **contrib_area);
                   
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
                double *quz, double *qb, double *qof, double *p, double *ep);

extern void tread(FILE *subcat_fptr,FILE *output_fptr,char *subcat, 
                int *num_topodex_values,int *num_channels,double *area,
                double **dist_area_lnaotb, double **lnaotb, int yes_print_output,
                double **cum_dist_area_with_dist, double *tl, 
                double **dist_from_outlet, int maxsubcatch, int maxincr);
                  
extern void expinf(int irof, int it, int rint, double *df, double *cumf,
                double dt,double xk0, double szm, double hf);                  

extern void results(FILE *output_fptr, FILE *out_hyd_fptr,
                int nstep, double *Qobs, double *Q, 
                int current_time_step, int yes_print_output);

extern void water_balance(FILE *output_fptr, int yes_print_output,
                char *subcat, double *bal, double *sbar, double *sump,
                double *sumae, double *sumq, double *sumrz, double *sumuz);

extern void itwo_alloc( int ***ptr, int x, int y);
extern void dtwo_alloc( double ***ptr, int x, int y);
extern void d_alloc(double **var,int size);
extern void i_alloc(int **var,int size);


/*** Model structure ***/

// Changed: "struct topmodel_model{" to "struct TopModel_Struct{"
struct TopModel_Struct{

  /******************* I/O files *******************/  
  /*FILE *fptr;*/
  FILE *control_fptr;  
  FILE *input_fptr;   
  FILE *subcat_fptr;  
  FILE *params_fptr;  
  FILE *output_fptr;
  FILE *out_hyd_fptr; 

  /************* Variable definitions **************/
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


  /******* Model parameters and input scalars ******/
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

  /************ Other variables of note ************/
  int num_delay;          /* number of time steps lag (delay) in channel within catchment to outlet */
  int num_time_delay_histo_ords; /* number of time delay histogram ordinates */
  double szq;              /* an important parameter, I don't know how to describe it */
  double tl;               /* another important parameter that excapes description, time lag?*/
  double max_contrib_area; /* the max. contributing area during a simulation */
  double bal;              /* the residual of the water balance */
  double sbar;             /* the catchment average soil moisture deficit */

  // THESE POINTERS WILL BE DIMENSIONED DYNAMICALLY TO BECOME 1-D ARRAYS
  double *Q;                    // simulated discharge
  double *Qobs;                 // observed discharge
  double *rain;                 // rainfall rate
  double *pe;                   // potential evapotranspiration
  double *contrib_area;         // contributing area
  double *stor_unsat_zone;      // storage in the unsat. zone
  double *deficit_root_zone;    // root zone storage deficit
  double *deficit_local;        // local storage deficit
  double *time_delay_histogram; // time lag of outflows due to channel routing
  double *dist_area_lnaotb;     // the distribution of area corresponding to ln(A/tanB) histo.
  double *lnaotb;               // these are the ln(a/tanB) values
  double *cum_dist_area_with_dist;  // channel cum. distr. of area with distance
  double *dist_from_outlet;     // distance from outlet to point on channel with area known

  /************** Other internal vars **************/
  int num_sub_catchments;
  int max_atb_increments;
  int max_num_subcatchments;
  int max_time_delay_ordinates;
  // int isc   //subcat loop no longer handled by topmod()

  /******************** Returns ********************/
  double Qout;

  /******************* BMI vars ********************/ 
  int current_time_step;    /*current time step*/
                            /*for BMI time functions*/

  /***************** State Var Sums ****************/
  double sump;   /* accumulated rainfall */
  double sumae;  /* accumulated evapotranspiration */   
  double sumq;   /* accumulated discharge (Qout) */
  double sumrz;  /* deficit_root_zone over the whole watershed */
  double sumuz;  /* stor_unsat_zone over the whole watershed */

  /************** Additional Output vars **************/ 
  double quz; /* flow from root zone to unsaturated zone*/
  double qb;  /* subsurface flow or baseflow*/
  double qof; /* flow from saturated area and infiltration excess flow*/
  double p;   /* adjusted rain*/
  double ep;  /* adjusted potential evaporation*/

  /************** Framework vars **************/ 
  int stand_alone;
  //double obs_values;

  /************** Test State **************/
  //double dbl_arr_test[3];

};

//-------------------------------------------------
// Define "topmodel_model" as a user-defined type
// using TopModel_Struct above.
//------------------------------------------------------
// "struct topmodel_model" -> "struct TopModel_Struct"
//------------------------------------------------------
typedef struct TopModel_Struct topmodel_model;
extern void alloc_topmodel(topmodel_model *model);
extern void free_topmodel(topmodel_model *model);

#endif
