
/*****************************************************************
    topmodel.c   (Oct. 2002)

    Conversion of Keith Beven's Fortran version of TopModel
    called TMOD9502.FOR to C by Scott Peckham.  Many changes
    were made (e.g. variable names, logic, comments, optional
    plots) so that this version is much easier to read and
    understand.  In particular, the routine Call_ExpInf2 was
    completely rewritten starting from the Beven (1984) paper
    to simplify the logic and avoid all use of gotos, etc.
    See notes for Call_ExpInf2.  There is a separate subroutine
    for reading each of the four input files, which have been
    modified to include descriptive headers.  This version has
    not been extensively tested, however.

    Conversion from IDL can be somewhat automated by using a
    word processor to do global search and replacements:
       {endfor, endwhile, endif, endelse} -> "}"
       {do begin, then begin} -> "{"
       for comments: ";" -> "//"
       {lt, le, gt, ge, eq, ne} -> {<, <=, >, >=, ==, !=}
       {AND, OR, NOT} -> {&&, ||, !}
       {pro, function, END;} -> {void, void, "}"}
       single quotes -> double quotes
       {alog, alog10, abs} -> {log, log10, fabs}
       {fix, float, double} -> {(int), (float), (double)} 
    It also helps to use Kernighan & Ritchie-style function
    definitions.

    Functions in this file are:
 
    Read_Input_Vars,
    Read_TI_Histogram,
    Read_Area_Distance_Curve,
    Read_Rainfall_Etc,

    Init_Vars,
    Call_ExpInf2,
    Check_Results

    main (TopModel),     ;(Keep testing.)

*****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Notice string terminator, \0, at end. */
// #define directory "F:\\TOP_MODEL\\Input_Files\\\0"
#define directory "\\home\\jess\\Downloads\\Topmodel\\TopModel\\__input_files\0"

/*#define format  "%s 14.3f"  /* generic output format */

/*****************************************************************/
void Read_Input_Vars(in_file, SZM, LN_T0, TD, CHV, RV,
                     SRMAX, Q0, SR0, INFILTRATE, K0, d_psi,
                     d_theta)

char   in_file[100];
int    *INFILTRATE;
double *SZM, LN_T0, TD, CHV, RV, SRMAX, Q0, SR0, K0, d_psi, d_theta;
{

/*---------------------------------------*\
  Notes:  T0  -> LN_T0  (less confusing)
\*---------------------------------------*/

/*----------*\
  Local vars
\*----------*/
int   k;
char  line[100];
FILE  *fp;

/*--------------------*\
  Open the input file
\*--------------------*/
fp = fopen(in_file, "r");

/*-----------------*\
  Strip off header
\*-----------------*/
for (k=1; k<=4; k++) fscanf(fp, "%s", line);

/*--------------------*\
  Read the parameters
\*--------------------*/
fscanf(fp, "%f", &SZM);   /* controls rate of decline of T(z)) */
fscanf(fp, "%f", &LN_T0);
fscanf(fp, "%f", &TD);    /* time delay const for routing unsat. flow */
fscanf(fp, "%f", &CHV);
fscanf(fp, "%f", &RV);    /* constant stream velocity */
fscanf(fp, "%f", &SRMAX);
fscanf(fp, "%f", &Q0);
fscanf(fp, "%f", &SR0);
fscanf(fp, "%d", &INFILTRATE);  /* flag whether to do infiltration */
fscanf(fp, "%f", &K0);
fscanf(fp, "%f", &d_psi);     /* HF */
fscanf(fp, "%f", &d_theta);   /* DTH */

/*---------------------*\
  Close the input file
\*---------------------*/
fclose(fp);
 
}  /* Read_Input_Vars */
/*****************************************************************/
void Read_TI_Histogram(TI_file, n_bins, AC, ST, SUMAC, TL,
                       basin_area)

char   TI_file[100];
int    *n_bins;
double *AC, *ST, *SUMAC, *TL, *basin_area;

{
/*---------------------------------------------------------------*\
  Notes:  TI_file should be a text file that has two numbers
          per line.  The first number should be the percentage
          of basin area (or percentage of pixels in a DEM) with
          values in a TI bin, and the second number should be a
          TI value for the right side of a bin. The TI
          values should be ordered from highest (first line) to
          lowest (last line), with ST[0] as an upper limit and
          AC[0]=0.0.  AC[k] should be the area of pixels with
          TI values between ST[k-1] and ST[k].     
\*---------------------------------------------------------------*/
double *fractions;   /*************/
int k, n;
const char format[] = "%s %14.3f\n";
char line[100];
FILE *fp;

/*-------------------------*\
  Open the TI input file
  and strip off the header
\*-------------------------*/
fp = fopen(TI_file, "r");
for (k=1; k<=6; k++) fscanf(fp, "%s", line);

/*------------------------*\
  Read the number of bins
\*------------------------*/
fscanf(fp, "%d", &n_bins);
ST = (double *)malloc(n_bins * sizeof(double));
AC = (double *)malloc(n_bins * sizeof(double));
fractions = (double *)malloc(n_bins * sizeof(double));

/*------------------------*\
  Read TI and area values
\*------------------------*/
basin_area = 0.0;
for (n=0L; n < n_bins; n++) {
    fscanf(fp, "%f%f", &(AC[n]), &(ST[n]));
    basin_area = basin_area + AC[n];   /* should add to 1.0 */
}

/*---------------------*\
  Close the input file
\*---------------------*/
fclose(fp);

printf(format, "Basin area sum = ", basin_area);

/*--------------------------*\
  Compute the area integral
\*--------------------------*/
TL = 0.0;
fractions[0] = AC[0] / basin_area;
SUMAC = fractions[0];
for (k=1; k < n_bins; k++) {
    fractions[k] = (AC[k] / basin_area);
    SUMAC = SUMAC + fractions[k];
    TL = TL + AC[k]*(ST[k] + ST[k-1])/2.0;
}

}  /* Read_TI_Histogram */
/*****************************************************************/
void Read_Area_Distance_Curve(AD_file, n_bins, D, ACH)

char   AD_file[100];
int    *n_bins;
double *D, *ACH;
{

/*----------------------------------------------------*\
  Notes:  ACH is cumulative distribution of area with
          distance D from basin outlet.

          D[0] = distance from subbasin outlet to the
                 main basin outlet
          ACH[0]   = 0.0
          ACH[n-1] = subbasin area
\*----------------------------------------------------*/

int  k, n;
char line[100]; 
FILE *fp;

/*-----------------------*\
  Open the AD input file
\*-----------------------*/
fp = fopen(AD_file, "r");

/*---------------------*\
  Strip off the header
\*---------------------*/
for (k=1; k <= 5; k++) fscanf(fp, "%s", line);

/*--------------------*\
  Read number of bins
\*--------------------*/
fscanf(fp, "%d", &n_bins);
D   = (double *)malloc(n_bins * sizeof(double));
ACH = (double *)malloc(n_bins * sizeof(double));

/*----------------------*\
  Read D and ACH values
\*----------------------*/
for (n=0; n < n_bins; n++)
    fscanf(fp, "%f%f", &(ACH[n]), &(D[n]));

/*---------------------*\
  Close the input file
\*---------------------*/
fclose(fp);

}  /* Read_Area_Distance_Curve */
/*****************************************************************/
void Read_Rainfall_Etc(rain_file, n_steps, dt, R, PE, Q_obs)

char   rain_file[100];
long   *n_steps;
double *dt, *R, *PE, *Q_obs;
{

/*----------*\
  Local vars
\*----------*/
int k, count;
long n;
const char format[] = "%s %14.3f\n";
char line[100];
FILE *fp;

/*-------------------------*\
  Open the rain input file
\*-------------------------*/
fp = fopen(rain_file, "r");

/*---------------------*\
  Strip off the header       (line is an array; no &)
\*---------------------*/
for (k=1; k<=6; k++) fscanf(fp, "%s", line);  /* USE fgets */

/*------------------------------*\
  Read number of timesteps & dt
\*------------------------------*/
fscanf(fp, "%d%f", &n_steps, &dt);

printf(format, "n_steps = ", *n_steps);
printf(format, "dt      = ", *dt);
printf("\n");

/*-------------------------*\
 Allocate memory for arrays
\*-------------------------*/
R     = (double *)malloc(*n_steps * sizeof(double));
PE    = (double *)malloc(*n_steps * sizeof(double));
Q_obs = (double *)malloc(*n_steps * sizeof(double));

/*-----------------------------*\
  Read R, PE, and Q_obs values
\*-----------------------------*/
for (n=0; n < n_steps; n++) {
    count = fscanf(fp, "%f%f%f\n", &(R[n]), &(PE[n]), &(Q_obs[n]));
    if (count < 3) Q_obs[n]=-9999.0;
}

/*---------------------*\
  Close the input file
\*---------------------*/
fclose(fp);

}  /* Read_Rainfall_Etc */
/*****************************************************************/
void Init_Vars(LN_T0, TL, dt, D, n_AD, area, n_bins, SR0,
               SZM, Q0, Q, n_steps, RV, CHV, ACH, RV_dt,
               CHV_dt, SZQ, TCH, AR, NR, ND, SUZ, SRZ, Sbar,
               balance)

/*-------------------------*\
  Input vars; some modified
\*-------------------------*/
int    *n_bins, *n_AD;
long   *n_steps;
double RV, CHV;
double *LN_T0, *TL, *dt, *D, *area, *SR0, *SZM, *Q0,
       *Q, *ACH;
/*-------------*\
  Returned vars
\*-------------*/
int    *NR, *ND;
double *RV_dt, *CHV_dt, *SZQ, *TCH, *AR, *SUZ, *SRZ,
       *Sbar, *balance;

{

/*----------*\
  Local vars
\*----------*/
int i, j, k, index;
double T0, frac, SUMAR, A1, A2, time, sum;
const char format[] = "%s 14.3f\n";

/*-----------------------------------------------*\
  Convert parameters to [m/timestep (dt)]
  with exception of XK0 which must stay in [m/h]
  Q0 is already in [m/timestep]
  LN_T0 = Ln(T0)
\*-----------------------------------------------*/
*RV_dt  = RV  * dt;
*CHV_dt = CHV * dt;
T0      = exp(LN_T0);
*SZQ    = T0 * dt * exp(-TL);
for (i=0; i < NR; i++) AR[i]=0.0;

/*-------------------------------*\
  Convert distance/area form to
  time delay histogram ordinates
\*-------------------------------*/
TCH[0] = (D[0] / *CHV_dt);
for (j=1; j < n_AD; j++)
   TCH[j] = TCH[0] + (D[j] - D[0]) / *RV_dt;

/* NR = ceil(TCH[n_AD - 1]); */
NR = (int)(TCH[n_AD - 1]);
if ((float)NR < TCH[n_AD - 1]) NR=(NR + 1);
ND = (int)TCH[0];
NR = NR - ND;

/*-----------------------*\
  Compute AR from TCH ??
\*-----------------------*/
for (i=0; i < NR; i++) {
    time = (double)(ND + i + 1);              /**************/
    if (time > TCH[n_AD - 1]) AR[i] = 1.0;
    else {
        for (j=1; j < n_AD; j++) {
            if (time <= TCH[j]) {
                frac  = (time - TCH[j-1]) / (TCH[j] - TCH[j-1]);
                AR[i] = ACH[j-1] + (ACH[j] - ACH[j-1]) * frac;
                break;
                /* j = n_AD;  break out of inner loop */
            }
        }
    }
}

A1    = AR[0];
SUMAR = AR[0];
AR[0] = AR[0] * area;
if (NR > 1) {
    for (k=1; k < NR; k++) {
        A2 = AR[k];
        AR[k] = (A2 - A1);
        A1 = A2;
        SUMAR = SUMAR + AR[k];
        AR[k] = AR[k] * area;
    }
}

printf(format, " SZQ                        = ", SZQ);
printf(format, " Maximum routing delay      = ", TCH[n_AD - 1]);
printf(format, " Sum of histogram ordinates = ", SUMAR);
printf(format, "AR  = ", AR);  /************/

/*---------------------------*\
  Initialize SRZ, SUZ & Sbar
\*---------------------------*/
for (k=0; k < n_bins; k++) SRZ[k] = SR0;
Sbar = -1.0 * SZM * log(Q0 / SZQ);

/*------------------------------*\
  Reinitialize the discharge, Q
\*------------------------------*/
printf(format, "NR = ", NR);
printf(format, "ND = ", ND);

for (k=0; k <= ND; k++)
    Q[k] = Q[k] + (Q0 * area);
sum = 0.0;
for (i=0; i < NR; i++) {
    sum   = sum + AR[i];
    index = (ND + i + 1);
    if (index < n_steps)
        Q[index] = Q[index] + Q0*(area - sum);
}

/*--------------------------------*\
  Initialize the water balance
  Balance is positive for storage
\*--------------------------------*/
balance = -1.0 * (Sbar + SR0);
printf(format, " Initial balance = ", balance);
printf(format, " Initial Sbar    = ", Sbar);
printf(format, " Initial SR0     = ", SR0);

}  /* Init_Vars */
/*****************************************************************/
void Call_ExpInf2(n, dt, rainrate, dI, I,
                  d_psi, d_theta, K0, fs, IROF)

int    *IROF;
long   n;
double dt, rainrate, K0, d_psi, d_theta, fs;
double *dI, *I;    /* These get updated. */
{

/*--------------------------------------------------------------*\
  Notes:  Calculate infiltration excess runoff using Beven's
          version of the Green-Ampt model in which the
          saturated hydraulic conductivity decreases as an
          exponential function of depth and the storage-suction
          factor is a constant.  This version of Green-Ampt
          is documented in the paper:

          Beven, K. (1984) Infiltration into a class of
          vertically non-uniform soils, Hydrol. Sci. J.,
          29(4), 425-434.

          Variable names closely follow those in the above
          reference:
          infil = infiltration rate [m/s] = dI/dt
          I     = cumulative infiltration [meters]
                  at end of a timestep
          tp    = ponding time [hr or s]
          Ip    = I at ponding time, tp
          C     = d_psi * d_theta   (assumed constant)
          K0    = hydraulic conductivity at surface (z=0)
          f     = parameter for how K decreases with z
          fs    = -(f / d_theta)
          n     = timestep number that starts at 0
          dt    = timestep length in hours

  NB!     Should rainrate be divided by dt prior to call ??
  --------------------------------------------------------------
  Table from Beven paper with values for sand layers

   Layer     Ks        d_psi     d_theta   C
   [m]       [m/h]     [m]                 [m]
  ------------------------------------------------
  0.0-0.3    13.2      0.060     0.35      0.021
  0.3-0.6     7.5      0.080     0.355     0.028
  0.6-0.9     4.2      0.100     0.36      0.036
  0.9-1.2     2.9      0.125     0.36      0.045
  1.2-1.5     1.7      0.159     0.365     0.058
  1.5-1.8     0.5      0.178     0.37      0.066

  (WHAT IS A TYPICAL VALUE FOR f OR fs ???)
   fs = -(f / d_theta);   (passed from caller) 
\*--------------------------------------------------------------*/

/*----------*\
  Local vars
\*----------*/
int    j, k, n_tries;
int    NO_PONDING, CONVERGED;
double C, t_start, t_end, tp, Ip, Ip_hi, Ip_lo, Ir;
double rhs, lhs, h, dh_dI, del_I, dI1, dI2;
double infil, sum, term, lambda, x;
long   factorial;
const  char format[] = "%s 14.3f\n";
const  ERR = 0.00001;     /* error tolerance */
const  N_MAX = 20;        /* max number of iterations */

/* printf("%s", "Calling ExpInf2 routine..."); */

C       = (d_psi * d_theta);
t_start = n * dt;            /* time at start of timestep */
t_end   = (n + 1) * dt;      /* time at end of timestep */

/*------------------------------------*\
  Initial guess for Ip, but note that
  Ip could be much bigger than Ip_hi.
\*------------------------------------*/
Ip_hi = *I + (rainrate * dt);   /* upper bound */
Ip_lo = *I;                     /* lower bound */
Ip    = Ip_hi;
/*------------------*/
/*
Ip   = Ip_hi * 10.0;
Ip   = (Ip_lo + Ip_hi) / 2.0;      (midpoint)
Ip   = Ip_lo;     (Don't use this, it may be 0.)
*/

/*-----------------------------------------*\
 Iterate eqn (9) in Beven paper to get Ip
 and then compute time to ponding, tp.
 -----------------------------------------------
 This seems to assume that infil(Ip) is an
 increasing function, but it isn't for small Ip.
 Maybe okay if we start high and slide down.
 --------------------------------------------------
 NB!  Ip may not be initialized to zero since that
 would cause the expression for infil to blow up.
\*--------------------------------------------------*/
n_tries   = 0;
CONVERGED = (Ip != 0.0);
while ((n_tries <= N_MAX) && !CONVERGED) {
    infil = -K0 * fs * (C + Ip)/(1.0 - exp(fs * Ip)); 
    if (infil > rainrate) {
        /*------------------------------*\
          Lower the upper bound and
          reduce Ip toward lower bound
        \*------------------------------*/
        Ip_hi = Ip;
        Ip    = (Ip + Ip_lo) / 2.0;
        CONVERGED = (fabs(Ip - Ip_lo) < ERR);
    } else {
        /*-------------------------------*\
          Raise the lower bound and
          increase Ip toward upper bound
        \*-------------------------------*/
        Ip_lo = Ip;
        Ip    = (Ip + Ip_hi) / 2.0;
        CONVERGED = (fabs(Ip - Ip_hi) < ERR);
    }
    n_tries = n_tries + 1;
}

if (!CONVERGED && (Ip != 0.0)) {
    printf("ERROR:  Max number of iterations exceeded\n");
    printf("while computing time to ponding.\n");
    printf("\n");
    return;
    /* return(); */
}

/*--------------------------------------*\
  Use Ip to check if ponding will occur
  in this timestep
\*--------------------------------------*/
NO_PONDING = ((Ip - *I) > (rainrate * dt)) || (rainrate == 0);
              /* || (rainrate < K0) ********/

/*-----------------------------------------*\
  Case where there is ponding already
  by the first timestep.  Is this needed ?
\*-----------------------------------------*/
/*
if ((*I == 0.0) && (n == 1)) {
    NO_PONDING = 0;
    tp = t_start;
}
*/

/*----------------------------*\
  Compute I for this timestep
\*----------------------------*/
if (NO_PONDING) {
    *IROF = 0;  /**********/

    /*----------------------------*\
      All of the rain infiltrates
    \*----------------------------*/
    *dI = rainrate * dt;
    *I  = *I + *dI;
} else {
    *IROF = 1;  /*********/

    /*------------------------------------*\
      Ponding occurs in this timestep, but
      may have started in a previous step.
      Compute time to ponding, tp from Ip
      ------------------------------------
      All rain infiltrates up until t=tp,
      then there is ponding for remainder.
      ------------------------------------
      Note: (rainrate ne 0) to get here.
      Note: tp may be greater than t_end.
      Note: If (Ip < I) then (dI < 0).
    \*------------------------------------*/
    *dI = (Ip - *I);
    tp = t_start + (*dI / rainrate);
    /* if (tp < t_start) tp = t_start;  */

    /*-------------------------------------*\
      Compute 10 terms in a Taylor series
      that depends on Ip, to compute lambda
    \*-------------------------------------*/
    sum       = 0.0;
    factorial = 1;
    x         = (Ip + C);
    for (j=1; j <= 10; j++) {
        factorial = factorial * j;
        term      = power(fs*x, j) /(j * factorial);
        sum       = sum + term; 
    }

    /*-----------------------------------------*\
      Compute the integration constant, lambda
    \*-----------------------------------------*/
    lambda = log(x) - (log(x) + sum)/exp(fs * C);

    /*---------------------------------------*\
      Initial guess for Newton-Raphson is Ip
      plus half of potential contribution
    \*---------------------------------------*/
    Ir = Ip + (0.5 * rainrate * (t_end - tp));

    /*----------------------------------*\
      Iterate eqn (8) in Beven paper to
      directly compute new I = I(t_end)
    \*----------------------------------*/
    n_tries = 0;
    CONVERGED = 0;
    while ((n_tries < N_MAX) && !CONVERGED) {
        /*-------------------------------------*\
          Compute 10 terms in a Taylor series
          that depends on I
        \*-------------------------------------*/
        sum       = 0.0;
        factorial = 1;
        x         = (Ir + C);
        for (j=1; j <= 10; j++) {
            factorial = factorial * j;
            term      = power(fs*x, j) /(j * factorial);
            sum       = sum + term; 
        }

        /*------------------------------*\
          Use the Newton-Raphson method
          -------------------------------------------
          Recall from Beven paper that:
          (1 - exp(fs*I))/(I+C) * dI = -K0 * fs * dt
          is what was integrated for I = Ip to I_end
          and t = tp to t_end to get eqn (8).
        \*-------------------------------------------*/
        term  = (log(x) + sum) / exp(fs * C);
        rhs   = (log(x)- term - lambda);
        lhs   = (t_end - tp) * (-1.0 * K0 * fs);
        h     = (rhs - lhs);
        dh_dI = (1.0 - exp(fs * Ir)) / (Ir + C);
        del_I = -1.0 * (h / dh_dI);
        Ir    = Ir + del_I;
        CONVERGED = (fabs(del_I) <= ERR);
        n_tries = n_tries + 1;
    }

    if (!CONVERGED) {
        printf("Max number of iterations exceeded\n");
        printf("while computing cumulative infiltration.\n");
        printf("\n");
        return;
    }

    /*------------------------------------*\
      How much was contributed in this
      timestep, before and after tp ?
    \*------------------------------------*/
    dI1 = (Ip - *I);
    if (dI1 < 0.0) dI1 = 0.0;
    dI2 = (Ir - Ip);

    printf(format, "(Ip - I)  = ", dI1);
    printf(format, "(Ir - Ip) = ", dI2); 
    printf("\n");

    /*---------------------------*\
      Replace old I with new one
    \*---------------------------*/
    *I = Ir;
}   /* else, if PONDING */

}  /* Call_ExpInf2 */
/*****************************************************************/
void Check_Results(Q, Q_obs, n_steps, dt, out_file)

double *Q, *Q_obs, dt;
long   n_steps;
char   out_file[100];
{
 
/*----------*\
  Local vars
\*----------*/
double Qsum, ssQ, f1, f2, Qbar, varQ, varE, E;
const  char format[] = "%s $14.3f\n";    /******/
FILE   *out_file_fp;
int    n;

Qsum = ssQ = f1 = f2 = 0.0;

/*-------------------------------*\
  Compute the objective function
  to compare Q to Q_observed
\*-------------------------------*/
for (n=0; n < n_steps; n++) {
    Qsum = Qsum + Q_obs[n];
    ssQ  = ssQ  + (Q_obs[n] * Q_obs[n]);
    f1   = f1 + power(Q[n] - Q_obs[n], 2);   /**** power ****/
    f2   = f2 + fabs(Q[n] - Q_obs[n]);
}

Qbar = (Qsum / n_steps);
varQ = (ssQ  / n_steps) - (Qbar * Qbar);
varE = (f1   / n_steps);
E    = (1.0 - (varE / varQ));

/*---------------*\
  Print a report
\*---------------*/
printf("Objective function values:\n");
printf(format, "   f1          = ", f1);
printf(format, "   f2          = ", f2);
printf(format, "   e           = ", E);
printf(format, "   Mean(Q_obs) = ", Qbar);
printf(format, "   Var(Q_obs)  = ", varQ);
printf(format, "   Var(error)  = ", varE);
printf("\n");

/*-----------------------*\
  Print report to a file
\*-----------------------*/
out_file_fp = fopen(out_file, "w");
fprintf(out_file_fp, "Objective function values:\n");
fprintf(out_file_fp, format, "   f1          = ", f1);
fprintf(out_file_fp, format, "   f2          = ", f2);
fprintf(out_file_fp, format, "   e           = ", E);
fprintf(out_file_fp, format, "   Mean(Q_obs) = ", Qbar);
fprintf(out_file_fp, format, "   Var(Q_obs)  = ", varQ);
fprintf(out_file_fp, format, "   Var(error)  = ", varE);
fprintf(out_file_fp, "\n");
fclose(out_file_fp); 

}  /* Check_Results */
/*****************************************************************/
/*
void main(in_file, TI_file, AD_file, rain_file, out_file,
         INFILTRATE)

char in_file[100], TI_file[100], AD_file[100], rain_file[100],
     out_file[100];
*/

void main(INFILTRATE)
int  INFILTRATE;
{

/*---------------------------------------------------------------*\
Notes:  n_bins  = number of bins in ln(a/tanB) histogram (NAC)
        n_steps = number of timesteps in simulation (NSTEP)
        dt      = timestep [hours]
        R       = rainfall (size = n_steps)
        PE      = potential ET ? (size = n_steps)
        EX      = ??????
        Q_obs   = observed discharges (size = n_steps)
        AC      = histogram of ln(a/tanB)  (in TI_file)
                = amount of area with a particular value
                  Note: basin_area = total(AC)
        INFILTRATE: set this flag to use Green-Ampt method.

        If Q0 is too large, seems not to work correctly.
\*---------------------------------------------------------------*/

/*---------------------------*\
  Declare variables and types
\*---------------------------*/
double R_excess, Psum, dI, I, ACMAX, ACM, ACF;
double SZM, LN_T0, TD, CHV, RV, SRMAX, Q0, SR0,
       K0, d_psi, d_theta, fs, P, EP, EA;   /***********/
double dt, time, TL, rainrate, basin_area;
double Q_overland, dQ_overland, QUZ, UZ, QB, Q_out, Q_total;
double Sbar, RV_dt, CHV_dt, SZQ, TCH;
double SUMRZ, SUMUZ, SUMQ, SUMAE, SUMAC, balance;

int   IROF, NO_PONDING;
long  n, m, k, kmax, k2, j, index, NR, ND, IHROF;
long  Q_size, AC_size;
FILE  *out_file_fp;
const char format[10] = "%s 14.3f\n";
char  in_file[100], AD_file[100], TI_file[100];   /************/
char  rain_file[100], out_file[100];

/*********************************************/
long   n_bins, n_steps, n_AD;
double *R, *PE, *Q_obs, *Q, *cum_infil, *CA;
double *AC, *ST, *EX, *ihour, *SD;
double *D, *ACH, *AR;
double *SUZ, *SRZ;
/*********************************************/

/*----------------------------------------*\
  Build filepaths from directory and names
  (only if the filenames aren't provided)
\*----------------------------------------*/
strcpy(in_file,   directory);
strcpy(AD_file,   directory);
strcpy(TI_file,   directory);
strcpy(rain_file, directory);
strcpy(out_file,  directory);

strcat(in_file,   "param_file.txt");
strcat(AD_file,   "AD_file.txt");
strcat(TI_file,   "TI_file.txt");
strcat(rain_file, "rain_file.txt");
strcat(out_file,  "TopModel_out.txt");

/*-------------------------------*\
  Read variables from input files
\*-------------------------------*/
Read_Input_Vars(in_file, &SZM, &LN_T0, &TD, &CHV, &RV, &SRMAX,
                &Q0, &SR0, &INFILTRATE, &K0, &d_psi, &d_theta);

Read_TI_Histogram(TI_file, &n_bins, AC, ST, &SUMAC, &TL,
                  &basin_area);
AC_size = n_bins * sizeof(double);
EX      = (double *)malloc(AC_size);
ihour   = (double *)malloc(AC_size);
SD      = (double *)malloc(AC_size);

Read_Area_Distance_Curve(AD_file, &n_AD, D, ACH);

Read_Rainfall_Etc(rain_file, &n_steps, &dt, R, PE, Q_obs);
Q_size    = n_steps * sizeof(double);
Q         = (double *)malloc(Q_size);   /* init to zeros */
cum_infil = (double *)malloc(Q_size);
CA        = (double *)malloc(Q_size);

//Q         = (double *)calloc(Q_size);   /* init to zeros */
for (k=0; k < n_steps; k++) Q[k] = 0.0;

/*----------------*\
  Initialize vars
\*----------------*/
R_excess    = 0.0;        /* REX */
Q_total     = 0.0;        /* SUMQ */
Psum        = 0.0;        /* SUMP */
I           = 0.0;        /* CUMF */
IROF        = 0.0;        /* a ponding flag */
ACMAX = 0.0;
SUMAE = 0.0;
IHROF = 0;

Init_Vars(&LN_T0, &TL, &dt, D, &n_AD, &basin_area, &n_bins,
          &SR0, &SZM, &Q0, Q, &n_steps, &RV, &CHV, ACH,
          &RV_dt, &CHV_dt, &SZQ, &TCH, &AR, &NR, &ND,
          SUZ, SRZ, &Sbar, &balance);

/*------------------------------*\
  Open the output file to write
\*------------------------------*/
out_file_fp = fopen(out_file, "w");

/*-------------------------------*\
  Print a header for output vars
  format = '(1x, A6, 11(A15))'
\*-------------------------------*/      /*************/
fprintf(out_file_fp, "%6s %15s %15s %15s %15s %15s
        %15s %15s %15s %15s %15s %15s",
        "n", "time [h]", "R [m/h]", "EP [m/h]",
        "dI [m]", "I [m]", "Q [m/dt]", "Qob [m/dt]",
        "Quz [m/dt]", "Qb [m/dt]", "Sbar [m]", "Qof [m/dt]",
        "\n");

/*-------------------------------*\
  Start the main simulation loop
\*-------------------------------*/
for (n=0; n < n_steps; n++) {
    if ((n % 100) == 0)
        printf("%s %6.0f\n", "n = ", n);

    Q_overland = 0.0;   /* QOF */
    QUZ = 0.0;
    EP = PE[n];
    P  = R[n];
    Psum = Psum + P;

    /*---------------------------------------------*\
      Compute infiltration excess via Green-Ampt ?
      ---------------------------------------------
      P = rainfall avail. for infiltration after
      the surface control calculation
    \*---------------------------------------------*/
    if (INFILTRATE && (P > 0)) {
        rainrate = P / dt;     /* [m/dt] -> [m/hr] */
        Call_ExpInf2(&n, &dt, &rainrate, &dI, &I, 
                     &d_psi, &d_theta, &K0, &fs, &IROF);
        /*----------------------------------------------*/
        R_excess = P - dI;
        P = P - R_excess;     /* Does this make sense ? */

        if (IROF == 1) {
            IHROF = (IHROF + 1);
        } else {
            R_excess = 0.0;
            I        = 0.0;
            IROF     = 0;     /* redundant ?? */
        }
    } else dI = 0.0;
    cum_infil[n] = I;
 
    /*------------------------------*\
      Loop over the ln(a/tanB) bins
    \*------------------------------*/
    ACM = 0.0;
    kmax = (n_bins - 1);

    for (k=0; k < n_bins; k++) {
        if ((k+1) < kmax) k2=(k+1); else k2=kmax;
        ACF   = (AC[k] + AC[k2]) / 2.0;
        UZ    = 0.0;
        EX[k] = 0.0;

        /*--------------------------------*\
          Calculate local storage deficit
        \*--------------------------------*/
        SD[k] = Sbar + SZM*(TL - ST[k]);
        if (SD[k] < 0) SD[k]=0.0;

        /*-----------------------*\
          Root zone calculations
        \*-----------------------*/
        SRZ[k] = SRZ[k] - P;
        if (SRZ[k] < 0) {
            SUZ[k] = SUZ[k] - SRZ[k];
            SRZ[k] = 0.0;
        }

        /*------------------------------*\
          Unsaturated zone calculations
        \*------------------------------*/
        if (SUZ[k] > SD[k]) {
            EX[k]  = SUZ[k] - SD[k];
            SUZ[k] = SD[k];
        }

        /*--------------------------------------------*\
          Calculate drainage from unsat. zone storage
        \*--------------------------------------------*/
        if (SD[k] > 0) {
            /*------------------------------------*\
              Next line has bug fix from original
              with dt in the numerator
            \*------------------------------------*/
    	      UZ = (SUZ[k] * dt) / (SD[k] * TD);
	      if (UZ > SUZ[k]) UZ = SUZ[k];
	      SUZ[k] = SUZ[k] - UZ;
	      if (SUZ[k] < 0.0000001) SUZ[k]=0.0;
	      QUZ = QUZ + (UZ * ACF);
        }

        /*------------------------------------*\
          Calculate ET from root zone deficit
        \*------------------------------------*/
        if (EP > 0) {
	      EA = EP*(1 - SRZ[k]/SRMAX);
	      if (EA > (SRMAX-SRZ[k])) EA = SRMAX - SRZ[k];
	      SRZ[k] = SRZ[k] + EA;
        } else EA = 0;
        SUMAE = SUMAE + (EA * ACF);

        /*------------------------------------------*\
          Calculate flow from fully saturated area
          ------------------------------------------
          Assume (a/tanB) values ordered hi to low.
        \*------------------------------------------*/
        dQ_overland = 0.0;
        if (k > 0) {
            j = (k - 1);
            if (EX[k] > 0) {
                /*--------------------------*\
                  Both limits are saturated
                \*--------------------------*/
                dQ_overland = AC[k]*(EX[j] + EX[k])/2;
                ACM = ACM + ACF;
                ihour[j] = ihour[j] + 1;
            } else {
                /*---------------------------*\
                  Is lower limit saturated ?
                \*---------------------------*/
                if (EX[j] > 0) {
		        ACF = ACF*EX[j]/(EX[j] - EX[k]);
		        dQ_overland  = ACF * EX[j] / 2 ;
		        ACM = ACM + ACF;
		        ihour[j] = ihour[j] + 1;
                }
            }
        }
        Q_overland = Q_overland + dQ_overland;

        /*------------------------------------*\
          Set contributing area plotting area
        \*------------------------------------*/
        CA[n] = ACM;      /* DOUBLE-CHECK THIS LINE */
        if (ACM > ACMAX) ACMAX = ACM;

    }   /* for loop over bins in histogram */

    /*----------------------------*\
      Add the infiltration excess
    \*----------------------------*/
    Q_overland = Q_overland + R_excess;
    if (IROF == 1) ACMAX = 1.0;

    /*---------------------------------------*\
      Calculate drainage from saturated zone
    \*---------------------------------------*/
    /*
     printf(format, "LN_T0 = ", LN_T0);
     printf(format, "TL    = ", TL);
     printf(format, "SZQ   = ", SZQ);
     printf(format, "SZM   = ", SZM);
     printf(format, "Q0    = ", Q0);
     printf(format, "Sbar  = ", Sbar);
    */

    /*------------------------------------*\
      Initial value of QB works out to Q0
    \*------------------------------------*/
    QB      = SZQ * exp(-Sbar / SZM);
    Sbar    = Sbar - QUZ + QB;
    Q_out   = QB + Q_overland;
    Q_total = Q_total + Q_out;

    /*---------------------------------*\
      Channel routing calculations
      ---------------------------------
      ND = time delay to outlet
      NR, ND are computed by Init_Vars
    \*---------------------------------*/
    for (m=0; m < NR; m++) {
        index = (n + ND + m);
        /* index = (n + ND + m - 1) */
        if (index < n_steps)
            Q[index] = Q[index] + (Q_out * AR[m]);
    }

    /*-----------------------------------*\
      Print the values for this timestep
      format = '(1x, I6, 11(E15.3))
    \*-----------------------------------*/
    time = n * dt;
    fprintf(out_file_fp, "%6d %15.3f %15.3f %15.3f %15.3f %15.3f
            %15.3f %15.3f %15.3f %15.3 %15.3 %15.3",
            n, time, R[n], PE[n], dI, I,
            Q[n], Q_obs[n], QUZ, QB, Sbar, Q_overland);

}  /* for loop over time index, n */

/*-----------------------------------*\
  Calculate the (mass) balance terms
\*-----------------------------------*/
SUMRZ = 0.0;    /* root zone */
SUMUZ = 0.0;    /* unsat zone */
kmax  = (n_bins - 1);
for (k=0; k < n_bins; k++) {
    if ((k+1) < kmax) k2=(k+1); else k2=kmax;
    ACF   = (AC[k] + AC[k2]) / 2.0;
    SUMRZ = SUMRZ + (SRZ[k] * ACF);
    SUMUZ = SUMUZ + (SUZ[k] * ACF);
}

/*-----------------------------------------*\
  Balances are summed across subcatchments
\*-----------------------------------------*/
SUMQ = Q_total;
balance = balance + (Sbar  + Psum + SUMRZ);
balance = balance - (SUMAE + SUMQ + SUMUZ);

/*---------------------*\
  Print final messages
\*---------------------*/
printf("\n");
printf("Water balance for subcatchment:\n");
printf("%14s", "SUMP", "SUMAE", "SUMQ", "SUMRZ", "SUMUZ", "Sbar", "BAL");
printf("\n");
printf("14.3f", Psum, SUMAE, SUMQ, SUMRZ, SUMUZ, Sbar, balance);
printf("\n");
printf(format, "Max contributing area = ", ACMAX);
 
/*---------------------------------*\
  Print final messages to out_file
\*---------------------------------*/
fprintf(out_file_fp, "\n");
fprintf(out_file_fp, "Water balance for subcatchment:\n");
fprintf(out_file_fp, "%15s %15s %15s %15s %15s %15s %15s\n",
        "SUMP", "SUMAE", "SUMQ", "SUMRZ", "SUMUZ", "Sbar", "BAL");
fprintf(out_file_fp, "%15.3f %15.3f %15.3f %15.3f %15.3f %15.3f %15.3f",
        Psum, SUMAE, SUMQ, SUMRZ, SUMUZ, Sbar, balance);
fprintf(out_file_fp, "\n");
fprintf(out_file_fp, format, "Max contributing area = ", ACMAX);
fclose(out_file_fp);

/*-----------------------------*\
  Check results against data ?
\*-----------------------------*/
Check_Results(Q, Q_obs, n_steps, dt, out_file);

/*-------------------------*\
  Free all allocated memory
\*-------------------------*/
free(EX);
free(ihour);
free(SD);
free(n_AD);
free(D);
free(ACH);
free(n_steps);
free(dt);
free(R);
free(PE);
free(Q_obs);
free(Q);
free(cum_infil);
free(CA);

}  /* main (TopModel) */
/*****************************************************************/

