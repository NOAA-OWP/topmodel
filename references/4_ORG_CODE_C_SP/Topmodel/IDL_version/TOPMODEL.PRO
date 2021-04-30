
;*****************************************************************
;   topmodel.pro   (Sept. - Oct. 2002)

;   Read_Input_Vars,
;   Read_TI_Histogram,
;   Read_Area_Distance_Curve,
;   Read_Rainfall_Etc,

;   Init_Vars,
;   TopModel,
;   Call_ExpInf,
;   Call_ExpInf2,
;   Check_Results

;*****************************************************************
pro Read_Input_Vars, in_file, SZM, T0, TD, CHV, RV, $
                     SRMAX, Q0, SR0, INFEX, XK0, HF, DTH

;-----------------------
;Notes:  XK0 = K0
;        HF  = d_psi
;        DTH = d_theta
;-----------------------

;----------------
;Initialize vars
;----------------
SZM=0.0  &  T0=0.0  &  TD=0.0  &  CHV=0.0  &  RV=0.0
SRMAX=0.0  &  Q0=0.0  &  SR0=0.0  &  INFEX=0  &  XK0=0.0
HF=0.0  &  DTH=0.0

;--------------------
;Open the input file
;--------------------
openr, unit, in_file, /get_lun

;-----------------
;Strip off header
;-----------------
line = ''
for k=1,4 do readf, unit, line

;--------------------
;Read the parameters
;--------------------
readf, unit, SZM
readf, unit, T0
readf, unit, TD
readf, unit, CHV
readf, unit, RV
readf, unit, SRMAX
readf, unit, Q0
readf, unit, SR0
readf, unit, INFEX
readf, unit, XK0
readf, unit, HF     ;(d_psi)
readf, unit, DTH    ;(d_theta)

;---------------------
;Close the input file
;---------------------
free_lun, unit
 
END;  Read_Input_Vars
;*****************************************************************
pro Read_TI_Histogram, TI_file, n_bins, AC, ST, SUMAC, TL, $
                       basin_area

;---------------------------------------------------------------
;Notes:  TI_file should be a text file that has two numbers
;        per line.  The first number should be the percentage
;        of basin area (or percentage of pixels in a DEM) with
;        values in a TI bin, and the second number should be a
;        TI value for the right side of a bin. The TI
;        values should be ordered from highest (first line) to
;        lowest (last line), with ST[0] as an upper limit and
;        AC[0]=0.0.  AC[k] should be the area of pixels with
;        TI values between ST[k-1] and ST[k].     
;---------------------------------------------------------------
basin_area = 0.0
TI_value   = 0.0
area_value = 0.0

;-------------------------
;Open the TI input file
;and strip off the header
;-------------------------
openr, unit, TI_file, /get_lun
line = ''
for k=1,6 do readf, unit, line

;------------------------
;Read the number of bins
;------------------------
n_bins = 0L
readf, unit, n_bins

;------------------
;Initialize arrays
;------------------
ST = dblarr(n_bins)
AC = dblarr(n_bins)

;------------------------
;Read TI and area values
;------------------------
for n=0L,(n_bins - 1L) do begin
    readf, unit, area_value, TI_value
    AC[n] = area_value
    ST[n] = TI_value 
    basin_area = basin_area + area_value  ;(should add to 1.0)
endfor

;---------------------
;Close the input file
;---------------------
free_lun, unit

print,'Basin area sum = ' + strtrim(string(basin_area),2)

;--------------------------
;Compute the area integral
;--------------------------
TL = 0d
fractions = dblarr(n_bins)
fractions[0] = AC[0] / basin_area
SUMAC = fractions[0]
for k=1L,(n_bins - 1L) do begin
    fractions[k] = (AC[k] / basin_area)
    SUMAC = SUMAC + fractions[k]
    TL = TL + AC[k]*(ST[k] + ST[k-1])/2d
endfor
;AC[n_bins] = 0d   ;******

;--------------------------
;Compute the area integral
;--------------------------
;fractions = (AC / basin_area)
;mid_values = (shift(fractions,-1) + fractions)/2.0
;mid_values[n_bins - 1L] = 0.0  ;*******
;TL = total(fractions * mid_values)

END;  Read_TI_Histogram
;*****************************************************************
pro Read_Area_Distance_Curve, AD_file, n_bins, D, ACH

;----------------------------------------------------
;Notes:  ACH is cumulative distribution of area with
;        distance D from basin outlet.

;        D[0] = distance from subbasin outlet to the
;               main basin outlet
;        ACH[0]   = 0.0
;        ACH[n-1] = subbasin area
;----------------------------------------------------
dist_value = 0.0
area_value = 0.0

;-----------------------
;Open the AD input file
;-----------------------
openr, unit, AD_file, /get_lun

;---------------------
;Strip off the header
;---------------------
line = ''
for k=1,5 do readf, unit, line

;--------------------
;Read number of bins
;--------------------
n_bins = 0L
readf, unit, n_bins

;------------------
;Initialize arrays
;------------------
D   = dblarr(n_bins)  ;(distance from outlet)
ACH = dblarr(n_bins)  ;(amt of basin area at each distance)

;----------------------
;Read D and ACH values
;----------------------
for n=0L,(n_bins - 1L) do begin
    readf, unit, area_value, dist_value 
    ACH[n] = area_value
    D[n]   = dist_value 
endfor

;---------------------
;Close the input file
;---------------------
free_lun, unit

END;  Read_Area_Distance_Curve
;*****************************************************************
pro Read_Rainfall_Etc, rain_file, n_steps, dt, R, PE, Q_obs

;----------------------------------------------------
;Notes: 
;----------------------------------------------------

;-------------------------
;Open the rain input file
;-------------------------
openr, unit, rain_file, /get_lun

;---------------------
;Strip off the header
;---------------------
line = ''
for k=1,6 do readf, unit, line

;------------------------------
;Read number of timesteps & dt
;------------------------------
n_steps = 0L
dt      = 0d
readf, unit, n_steps, dt

print,'N_steps = ', n_steps
print,'dt      = ', dt
print,' '

;------------------
;Initialize arrays
;------------------
R     = dblarr(n_steps)  ;(distance from outlet)
PE    = dblarr(n_steps)  ;(amt of basin area at each distance)
Q_obs = dblarr(n_steps)  ;(observed discharge values)

;-----------------------------
;Read R, PE, and Q_obs values
;-----------------------------
;data = dblarr(3, n_steps)
;readf, unit, data
;R     = data[0,*]
;PE    = data[1,*]
;Q_obs = data[2,*]

;-----------------------------
;Read R, PE, and Q_obs values
;-----------------------------
;R_value  = 0.0
;PE_value = 0.0
;Q_value  = 0.0
;for n=0L,(n_steps - 1L) do begin
;    readf, unit, R_value, PE_value, Q_value 
;    R[n]     = R_value
;    PE[n]    = PE_value
;    Q_obs[n] = Q_value
;endfor

;-----------------------------
;Read R, PE, and Q_obs values
;-----------------------------
tab  = string(9b)
line = ''
pattern = ' ' + tab + ','
for n=0L,(n_steps - 1L) do begin
    readf, unit, line
    words = strsplit(line, pattern, /extract)
    n_words = n_elements(words)
    R[n]    = float(words[0])
    PE[n]   = float(words[1])
    if (n_words eq 3) then begin
        Q_obs[n] = float(words[2])
    endif else begin
        Q_obs[n] = -9999.0
        nstr = strtrim(string(n),2)
        ;*** print,'Line ' + nstr + ' has less than 3 values.'
    endelse
endfor

;---------------------
;Close the input file
;---------------------
free_lun, unit

END;  Read_Rainfall_Etc 
;*****************************************************************
pro Init_Vars, T0, TL, dt, D, n_AD, area, n_bins, SR0, $
               SZM, Q0, Q, n_steps, RV, CHV, ACH, $
               ;--------------
               ;Returned vars
               ;--------------
               RV_DT, CHV_DT, T0_DT, SZQ, TCH, AR, NR, ND, $
               SUZ, SRZ, SBAR, balance

;-----------------------------------------------
;Convert parameters to [m/timestep (dt)]
;with exception of XK0 which must stay in [m/h]
;Q0 is already in [m/timestep]
;T0 is input as Ln(To)
;-----------------------------------------------
RV_dt  = RV  * dt
CHV_dt = CHV * dt
T0_dt  = T0 + alog(dt)
SZQ    = exp(T0_dt - TL)

;-------------------------------
;Convert distance/area form to
;time delay histogram ordinates
;-------------------------------
TCH = (D[0] / CHV_dt) + (D - D[0])/RV_dt
;------------------------------------------
;TCH[0] = (D[0] / CHV_dt)
;for j=1L,(n_AD - 1L) do begin
;   TCH[j] = TCH[0] + (D[j] - D[0])/RV_dt
;endfor

;** NR = ceil(TCH[n_AD - 1L])
NR = fix(TCH[n_AD - 1L])
if (float(NR) lt TCH[n_AD - 1L]) then NR=(NR + 1L)
ND = fix(TCH[0])
NR = NR - ND

;-----------------------
;Compute AR from TCH ??
;-----------------------
AR = dblarr(NR)  ;****
for i=0L,(NR - 1L) do begin
    ;** time = ND + i
    time = (ND + i + 1)
    if (time gt TCH[n_AD - 1L]) then begin
        AR[i] = 1d
    endif else begin
        for j=1L,(n_AD - 1L) do begin
            if (time le TCH[j]) then begin
                frac  = (time - TCH[j-1]) / (TCH[j] - TCH[j-1])
                AR[i] = ACH[j-1] + (ACH[j] - ACH[j-1]) * frac
                j = n_AD
                ;**j = (n_AD - 1L)  ;(break out of inner loop)  ;********
            endif
        endfor
    endelse
endfor

A1    = AR[0]
SUMAR = AR[0]
AR[0] = AR[0] * area            ;*****
if (NR gt 1) then begin
    for k=1L,(NR - 1L) do begin
        A2 = AR[k]
        AR[k] = (A2 - A1)
        A1 = A2
        SUMAR = SUMAR + AR[k]
        AR[k] = AR[k] * area
    endfor
endif

f0 = '(E16.5)'
sstr = strtrim(string(SZQ, format=f0), 2)
mstr = strtrim(string(TCH[n_AD - 1], format=f0), 2)
hstr = strtrim(string(SUMAR, format=f0), 2)
print,' SZQ                        = ' + sstr
print,' Maximum routing delay      = ' + mstr
print,' Sum of histogram ordinates = ' + hstr

print,'AR  = ', AR  ;****************
;print,' '
;print,'TCH = ', TCH
;stop
;if (SUMAR eq 0d) then STOP  ;*****************

;---------------------------
;Initialize SRZ, SUZ & SBAR
;---------------------------
SUZ  = dblarr(n_bins)
SRZ  = dblarr(n_bins) + SR0
SBAR = -SZM * alog(Q0 / SZQ)

;------------------------------
;Reinitialize the discharge, Q
;------------------------------
print,'Reinitializing Q...'  ;****************
print,'NR = ', NR
print,'ND = ', ND
wait, 5.0

Q[0:ND] = Q[0:ND] + (Q0 * area)   ;*******
;*** Q[0:ND-1L] = Q[0:ND-1L] + (Q0 * area)   ;*******
sum = 0.0
for i=0L,(NR - 1L) do begin
    sum   = sum + AR[i]
    index = (ND + i + 1)    ;***********
    ;*** index = (ND + i)
    if (index lt n_steps) then begin
        Q[index] = Q[index] + Q0*(area - sum)
    endif 

endfor

;--------------------------------
;Initialize the water balance
;Balance is positive for storage
;--------------------------------
balance = -(SBAR + SR0)
bstr  = strtrim(string(balance, format=f0), 2)
sbstr = strtrim(string(SBAR, format=f0), 2)
srstr = strtrim(string(SR0,  format=f0), 2)
print,' Initial balance = ' + bstr
print,' Initial SBAR    = ' + sbstr
print,' Initial SR0     = ' + srstr

END;  Init_Vars
;*****************************************************************
pro TopModel, in_file, TI_file, AD_file, rain_file, $
              out_file, INFILTRATE=INFILTRATE
              ;**** n_steps, dt, R, PE, Q_obs

;----------------------------------------------------------------
;Notes:  n_bins  = number of bins in ln(a/tanB) histogram (NAC)
;        n_steps = number of timesteps in simulation (NSTEP)
;        dt      = timestep [hours]
;        R       = rainfall (size = n_steps)
;        PE      = potential ET ? (size = n_steps)
;        EX      = ??????
;        Q_obs   = observed discharges (size = n_steps)
;        AC      = histogram of ln(a/tanB)  (in TI_file)
;                = amount of area with a particular value
;                  Note: basin_area = total(AC)
;        INFILTRATE: set this keyword to use Green-Ampt method.
;----------------------------------------------------------------

;-------------------------------
;Change to input file directory
;-------------------------------
directory = 'C:\My Documents\My_IDL\TopModel\Input_Files\'
cd, directory

if (n_elements(in_file) eq 0) then begin
    in_file   = directory + 'param_file.txt'
    AD_file   = directory + 'AD_file.txt'
    TI_file   = directory + 'TI_file.txt'
    rain_file = directory + 'rain_file.txt'
    out_file  = directory + 'TopModel_out.txt'
endif

;------------------------------
;Open the output file to write
;------------------------------
openw, OUNIT, out_file, /get_lun

;--------------------------------
;Read variables from input files
;--------------------------------
Read_Input_Vars, in_file, SZM, T0, TD, CHV, RV, $
                 SRMAX, Q0, SR0, INFEX, XK0, HF, DTH

;-----------------
;Keyword defaults
;-----------------
INFILTRATE = byte(INFEX)
;INFILTRATE = keyword_set(INFILTRATE)

K0      = XK0
fs      = (1d / SZM)   ;(was SZF)
d_psi   = HF
d_theta = DTH
Read_TI_Histogram, TI_file, n_bins, AC, ST, SUMAC, TL, $
                   basin_area   ;(n_AC -> n_bins)
EX = dblarr(n_bins)  ;*************
Read_Area_Distance_Curve, AD_file, n_AD, D, ACH

Read_Rainfall_Etc, rain_file, n_steps, dt, R, PE, Q_obs

;----------------
;Initialize vars
;----------------
Q  = dblarr(n_steps)  ;(hydrograph)
R_excess    = 0d    ;(REX)
Q_total     = 0d    ;(SUMQ)
Psum        = 0d    ;(SUMP)
I           = 0d    ;(CUMF)
IROF        = 0b    ;(a ponding flag)
ACMAX = 0d
SUMAE = 0d
Init_Vars, T0, TL, dt, D, n_AD, basin_area, n_bins, SR0, $
               SZM, Q0, Q, n_steps, RV, CHV, ACH, $
               ;--------------
               ;Returned vars
               ;--------------
               RV_DT, CHV_DT, T0_DT, SZQ, TCH, AR, NR, ND, $
               SUZ, SRZ, SBAR, balance

;------------------------------------
;Initialize contributing area counts
;IA -> area_counts, NAC -> n_bins
;------------------------------------
IHROF = 0L
ihour = lonarr(n_bins)

;-------------------------------
;Print a header for output vars
;-------------------------------
printf, OUNIT, 'n', 'time [h]', 'R [m/h]', 'EP [m/h]', 'dI [m]', 'I [m]', $
        'Q', 'Q_obs', 'Quz', 'Qb', 'Sbar', 'Qof', $
        format='(1x, A6, 11(A14))'

;-------------------------------
;Start the main simulation loop
;-------------------------------
CA = dblarr(n_steps)
cum_infil = dblarr(n_steps)
for n = 0L,(n_steps - 1L) do begin
    print, 'n = ' + strtrim(string(n), 2)

    Q_overland = 0d   ;(QOF)
    QUZ = 0d
    ;*** Q_unsaturated = 0d   ;(QUZ)

    EP = PE[n]
    P  = R[n]
    psum = psum + P

    ;---------------------------------------------
    ;Compute infiltration excess via Green-Ampt ?
    ;---------------------------------------------
    ;P = rainfall avail. for infiltration after
    ;the surface control calculation
    ;---------------------------------------------
    if (INFILTRATE AND (P gt 0)) then begin
        rainrate = P / dt    ;[m/timestep] -> [m/hr]
        ;------------------------------------------
        ;Call_Expinf, IROF, n, rainrate, dI, I, $
        ;             d_psi, d_theta, K0, f
        ;;Call_Expinf, IROF, n, rainrate, DF, CUMF
        ;------------------------------------------
        Call_ExpInf2, n, dt, rainrate, dI, I, $
                      d_psi, d_theta, K0, fs, IROF
        ;------------------------------------------
        R_excess = P - dI
        P = P - R_excess      ;**** DOESN'T MAKE SENSE ?? ****

        if (IROF eq 1) then begin
            IHROF = (IHROF + 1L)
        endif else begin
            R_excess = 0d
            I        = 0d
            IROF     = 0b     ;(redundant ??)
        endelse
    endif else dI = 0d    ;**********
    cum_infil[n] = I
 
    ;------------------------------
    ;Loop over the ln(a/tanB) bins
    ;------------------------------
    ACM = 0d
    SD = dblarr(n_bins)
    kmax = (n_bins - 1L)
    ;--------------------------------
    for k=0L,(n_bins - 1L) do begin
        k2    = (k + 1) < kmax  ;****************************
        ACF   = (AC[k] + AC[k2]) / 2d
        UZ    = 0d
        EX[k] = 0d

        ;--------------------------------
        ;Calculate local storage deficit
        ;--------------------------------
        SD[k] = SBAR + SZM*(TL - ST[k])
        SD[k] = SD[k] > 0d

        ;-----------------------
        ;Root zone calculations
        ;-----------------------
        SRZ[k] = SRZ[k] - P
        if (SRZ[k] lt 0d) then begin
            SUZ[k] = SUZ[k] - SRZ[k]
            SRZ[k] = 0d
        endif

        ;------------------------------
        ;Unsaturated zone calculations
        ;------------------------------
        if (SUZ[k] gt SD[k]) then begin
            EX[k]  = SUZ[k] - SD[k]
            SUZ[k] = SD[k]
        endif

        ;--------------------------------------------
        ;Calculate drainage from unsat. zone storage
        ;--------------------------------------------
        if (SD[k] gt 0d) then begin
    	      UZ = SUZ[k] / (SD[k] * TD * dt)
	      if (UZ gt SUZ[k]) then UZ = SUZ[k]
	      SUZ[k] = SUZ[k] - UZ
	      if (SUZ[k] lt 0.0000001d) then SUZ[k]=0d
	      QUZ = QUZ + (UZ * ACF)
        endif

        ;------------------------------------
        ;Calculate ET from root zone deficit
        ;------------------------------------
        if (EP gt 0d) then begin
	      EA = EP*(1 - SRZ[k]/SRMAX)
	      if (EA gt (SRMAX-SRZ[k])) then EA = SRMAX - SRZ[k]
	      SRZ[k] = SRZ[k] + EA
        endif else EA = 0d
        SUMAE = SUMAE + (EA * ACF)

        ;-----------------------------------------
        ;Calculate flow from fully saturated area
        ;------------------------------------------
        ;Assume (a/tanB) values ordered hi to low.
        ;------------------------------------------
        dQ_overland = 0d
        if (k gt 0) then begin  ;******
            j = (k - 1)
            if (EX[k] gt 0d) then begin
                ;--------------------------
                ;Both limits are saturated
                ;--------------------------
                dQ_overland = AC[k]*(EX[j] + EX[k])/2d
                ACM = ACM + ACF
                ihour[j] = ihour[j] + 1L
            endif else begin
                ;---------------------------
                ;Is lower limit saturated ?
                ;---------------------------
                if (EX[j] gt 0d) then begin
		        ACF = ACF*EX[j]/(EX[j] - EX[k])
		        dQ_overland  = ACF * EX[j] /2d
		        ACM = ACM + ACF
		        ihour[j] = ihour[j] + 1L
                endif
            endelse
        endif
        Q_overland = Q_overland + dQ_overland

        ;------------------------------------
        ;Set contributing area plotting area
        ;------------------------------------
        CA[n] = ACM               ;****** DOUBLE-CHECK THIS LINE
        ACMAX = (ACMAX > ACM)
        ;** if (ACM gt ACMAX) then ACMAX=ACM

    endfor  ;(loop over bins in histogram)

    ;----------------------------
    ;Add the infiltration excess
    ;----------------------------
    Q_overland = Q_overland + R_excess
    if (IROF eq 1) then ACMAX = 1.0

    ;---------------------------------------
    ;Calculate drainage from saturated zone
    ;---------------------------------------
    print,'T0   = ',T0
    print,'TL   = ',TL
    print,'SZQ  = ',SZQ
    print,'SZM  = ',SZM
    print,'Q0   = ',Q0
    print,'Sbar = ',Sbar
    ;---------------------------------
    QB      = SZQ * exp(-Sbar / SZM)
    Sbar    = Sbar - QUZ + QB
    Q_out   = QB + Q_overland
    Q_total = Q_total + Q_out
    ;---------------------------------
    print,'QB   = ',QB
    STOP ;***********

    ;-----------------------------
    ;Channel routing calculations
    ;---------------------------------
    ;ND = time delay to outlet
    ;NR, ND are computed by Init_Vars
    ;---------------------------------
    for m=0L,(NR - 1L) do begin
        index = (n + ND + m)      ;***********
        ;*** index = (n + ND + m - 1L) 
        if (index lt n_steps) then begin
            Q[index] = Q[index] + (Q_out * AR[m])
        endif
    endfor

    ;-----------------------------------
    ;Print the values for this timestep
    ;-----------------------------------
    time = n * dt
    printf, OUNIT, n, time, R[n], PE[n], dI, I, Q[n], Q_obs[n], $
            QUZ, QB, Sbar, Q_overland, format='(1x, I6, 11(E14.3))'

endfor  ;(loop over time index, n)

;-----------------------------------
;Calculate the (mass) balance terms
;-----------------------------------
SUMRZ = 0d  ;(root zone)
SUMUZ = 0d  ;(unsat zone)
kmax  = (n_bins - 1L)
for k=0L,(n_bins - 1L) do begin
    k2    = (k + 1) < kmax        ;***************
    ACF   = (AC[k] + AC[k2]) / 2d
    SUMRZ = SUMRZ + (SRZ[k] * ACF)
    SUMUZ = SUMUZ + (SUZ[k] * ACF)
endfor

;-----------------------------------------
;Balances are summed across subcatchments
;-----------------------------------------
SUMQ = Q_total  ;*******
balance = balance + (SBAR  + psum + SUMRZ)
balance = balance - (SUMAE + SUMQ + SUMUZ)

;---------------------
;Print final messages
;---------------------
print,' '
print,'Water balance for subcatchment:'
print, 'SUMP', 'SUMAE', 'SUMQ', 'SUMRZ', 'SUMUZ', 'SBAR', 'BAL', $
       format='(7(A14))'
print, psum, SUMAE, SUMQ, SUMRZ, SUMUZ, SBAR, balance, $
       format='(7(E14.3))'
print,' '

astr = strtrim(string(ACMAX, FORMAT='(E14.3)'), 2)
print,'Max contributing area = ' + astr

;---------------------------------
;Print final messages to out_file
;---------------------------------
printf, OUNIT, ' '
printf, OUNIT, 'Water balance for subcatchment:'
printf, OUNIT,'SUMP', 'SUMAE', 'SUMQ', 'SUMRZ', 'SUMUZ', 'SBAR', 'BAL', $
       format='(7(A14))'
printf, OUNIT, psum, SUMAE, SUMQ, SUMRZ, SUMUZ, SBAR, balance, $
        format='(7(E14.3))'
printf, OUNIT, ' '
printf, OUNIT, 'Max contributing area = ' + astr
free_lun, OUNIT

;-----------------------------
;Check results against data ?
;-----------------------------
Check_Results, Q, Q_obs, OUTFILE=outfile

;-----------------------------------------
;Plot the cumulative infiltration curve ?
;-----------------------------------------
window, 0, xsize=800, ysize=600
device, decomposed=0
time = findgen(n_steps) * dt
plot, time, cum_infil, psym=-3
;** plot, time[0:199], cum_infil[0:199], psym=-3

END;  TopModel
;*****************************************************************
pro Call_ExpInf, IROF, n, rainrate, DF, CUMF, $
                 HF, DTH, SZM, XK0, F   ;(were these globals ?)

;--------------------------------------------------------------
;Notes:  Calculate infiltration excess runoff using Beven's
;        version of the Green-Ampt model in which the
;        saturated hydraulic conductivity decreases as an
;        exponential function of depth and the storage-suction
;        factor is a constant.  This version of Green-Ampt
;        is documented in the paper:

;        Beven, K. (1984) Infiltration into a class of
;        vertically non-uniform soils, Hydrol. Sci. J.,
;        29(4), 425-434.

;Doubles:   CONST, SUM, FC, FUNC, CD, SZF, XKF
;-----------------------------------------------------------
CD  = (HF * DTH)    ;(HF = d_psi, DTH = d_theta)
SZF = (1.0 / SZM)
XKF = XK0          ;***********
err = 0.00001d     ;(tolerance)

NO_PONDING_YET = (IROF ne 1)

if (NO_PONDING_YET) then begin
    ;--------------------------------
    ;Has any water infiltrated yet ?
    ;Has overflow occurred ?
    ;--------------------------------
    if (CUMF ne 0) then begin
        ;----------------------------------------
        ;Estimate time to ponding, but note that
        ;formula blows up if (CUMF eq 0).
        ;----------------------------------------
        F1 = CUMF  ;(cum. infil rate)
        R2 = -XKF * SZF * (CD + F1)/(1d - exp(SZF * F1))
        if (R2 lt rainrate) then begin    ;(infil rate vs. rainrate)
            ;------------------------------------------
            ;(Infil_rate lt rainrate) in this timestep
            ;implies that ponding must have started at
            ;the beginning of this timestep.
            ;------------------------------------------
            TP   = (n - 1d) * dt   ;(TP = time to ponding ?)
            IROF = 1
            F    = CUMF
            ;*****************
            ;GOTO_8 = 1b
            ;Don't do the F2 branch at all.  ***************
            ;*****************
            ;-------------------------------------------
            ;Set up definite integral constant using FP
            ;-------------------------------------------
            ;*** TAG = 8
            ;--------------
            CONST = 0d
            FAC   = 1
            FC = (F + CD)
            for j=1,10 do begin
                FAC = FAC * j                   ;(factorial)
                ADD = (FC * SZF)^j /(j * FAC)
                CONST = CONST + ADD
            endfor

            ;------------------------------------
            ;Note: CONST should have type DOUBLE
            ;------------------------------------
            ;** CONST = DLOG(FC)-(DLOG(FC) + CONST)/DEXP(SZF*CD)
            CONST = alog(FC) - (alog(FC) + CONST)/exp(SZF * CD)
            IROF = 1
            F = F + (0.5 * rainrate * (n*dt - TP))

        endif else GOTO_8 = 0b  ;(R2 lt rainrate)
    endif                       ;(NO_OVERFLOW)

    ;--------------
    ;*** TAG = 7
    ;--------------
    F2 = CUMF + (rainrate * dt)   ;(establish an upper bound)

    if (F2 ne 0.0) then begin    ;(Don't GOTO 20)
        R2 = -XKF * SZF * (CD + F2)/(1d - exp(SZF * F2))
        if (R2 le rainrate) then begin   ;(Don't GOTO 20)
            ;----------------------------------------
            ;Ponding will occur during this timestep
            ;----------------------------------------
            F = CUMF + (R2 * dt)     ;(establish a lower bound)
            ;-----------------------------------
            ;Try to get F(t) with 20 iterations
            ;-----------------------------------
            n_tries  = 0
            CONVERGED = 0b
            while (n_tries le 20) and NOT(CONVERGED) do begin
                R2 = -XKF * SZF * (CD + F)/(1d - exp(SZF * F)) 
                if (R2 gt rainrate) then begin
                    F1 = F
                    F = (F2 + F)/2.0
                    if (abs(F - F1) lt err) then CONVERGED=1b
                    ;* CONVERGED = (abs(F - F1) lt err)
                endif else begin
                    F2 = F
	              F = (F1 + F)/2.0
                    if (abs(F - F2) lt err) then CONVERGED=1b
                    ;* CONVERGED = (abs(F - F2) lt err)
                endelse
                n_tries = n_tries + 1
            endwhile

            ;** if NOT(CONVERGED) then begin
                print,'Max number of iterations exceeded.'
                TP = ((n - 1.0) * dt) + (F - CUMF)/rainrate
            if (TP le (n*dt)) then begin   ;(Don't GOTO 20)

                ;-------------------------------------------
                ;Set up definite integral constant using FP
                ;-------------------------------------------
                ;*** TAG = 8
                ;--------------
                CONST = 0d
                FAC   = 1
                FC = (F + CD)
                for j=1,10 do begin
                    FAC = FAC * j                   ;(factorial)
                    ADD = (FC * SZF)^j /(j * FAC)
                    CONST = CONST + ADD
                endfor

                ;------------------------------------
                ;Note: CONST should have type DOUBLE
                ;------------------------------------
                ;** CONST = DLOG(FC)-(DLOG(FC) + CONST)/DEXP(SZF*CD)
                CONST = alog(FC) - (alog(FC) + CONST)/exp(SZF * CD)
                IROF = 1
                F = F + (0.5 * rainrate * (n*dt - TP))
            endif else NO_PONDING = 1b   ;(TP le (n*dt)) **** GOTO 20
        endif else     NO_PONDING = 1b   ;(R2 le rainrate)   **** GOTO 20
    endif else         NO_PONDING = 1b   ;(F2 ne 0.0)    **** GOTO 20
endif  ;(NO_PONDING)

;---------------
;*** TAG = 10
;----------------------------------
;If ponding has occurred, get the
;Newton-Raphson solution for F(t)
;----------------------------------
DONE = NO_PONDING_YET
n_tries = 0

while (n_tries lt 20) AND NOT(DONE) do begin
    ;------------------------------
    ;Calculate sum of series terms
    ;------------------------------
    FC  = (F + CD)
    SUM = 0d
    FAC = 1d
    for j=1,10 do begin
        FAC = (FAC * j)
        ADD = (FC * SZF)^j / (j * FAC)
        SUM = SUM + ADD
    endfor

    ;-----------------------------
    ;FUNC should have type DOUBLE
    ;-----------------------------
    ;FUNC = -(DLOG(FC)-(DLOG(FC)+SUM)/DEXP(SZF*CD)-CONST)/(XKF*SZF)
    ;     1     -(IT*DT-TP)

    term2 = (alog(FC) + SUM) / exp(SZF * CD)
    FUNC  = -(alog(FC)- term2 - CONST) / (XKF * SZF)
    FUNC  = FUNC - ((n*dt) - TP)
    DFUNC = (exp(SZF * F) - 1) / (XKF * SZF * FC)
    DF    = -FUNC / DFUNC
    F     = F + DF
    if (abs(DF) le err) then DONE = 1b
    ;** CONVERGED = (abs(DF) le err)
    n_tries = n_tries + 1
endwhile

;if NOT(CONVERGED) then begin
;    print,'Max number of iterations exceeded.'
;endif

;if (F lt (CUMF + rainrate)) AND NOT(NO_PONDING) then begin
;    DF   = F - CUMF
;    CUMF = F
;    F    = F + DF     ;(Initial guess for next step)   
;    RETURN
;endif

;--------------------------------------
;Was there ponding in this time step ?
;--------------------------------------
;*** TAG = 20
;---------------
if (NO_PONDING) then begin
    IROF = 0
    DF   = rainrate * dt
    CUMF = CUMF + DF
endif else begin
    if (F ge (CUMF + rainrate)) then RETURN
    DF   = F - CUMF
    CUMF = F
    F    = F + DF    ;(Initial guess for next step)
endelse

END;  Call_ExpInf
;*****************************************************************
pro Call_ExpInf2, n, dt, rainrate, dI, I, $
                  d_psi, d_theta, K0, fs, IROF
                  ;*** IROF = NOT(NO_PONDING) ??

;--------------------------------------------------------------
;Notes:  Calculate infiltration excess runoff using Beven's
;        version of the Green-Ampt model in which the
;        saturated hydraulic conductivity decreases as an
;        exponential function of depth and the storage-suction
;        factor is a constant.  This version of Green-Ampt
;        is documented in the paper:

;        Beven, K. (1984) Infiltration into a class of
;        vertically non-uniform soils, Hydrol. Sci. J.,
;        29(4), 425-434.

;        Variable names closely follow those in the above
;        reference:
;        infil = infiltration rate [m/s] = dI/dt
;        I     = cumulative infiltration [meters]
;                at end of a timestep
;        tp    = ponding time [hr or s]
;        Ip    = I at ponding time, tp
;        C     = d_psi * d_theta   (assumed constant)
;        K0    = hydraulic conductivity at surface (z=0)
;        f     = parameter for how K decreases with z
;        fs    = -(f / d_theta)
;        n     = timestep number that starts at 0
;        dt    = timestep length in hours

;NB!     Should rainrate be divided by dt prior to call ??
;--------------------------------------------------------------
;Table from Beven paper with values for sand layers

; Layer     Ks        d_psi     d_theta   C
; [m]       [m/h]     [m]                 [m]
;------------------------------------------------
;0.0-0.3    13.2      0.060     0.35      0.021
;0.3-0.6     7.5      0.080     0.355     0.028
;0.6-0.9     4.2      0.100     0.36      0.036
;0.9-1.2     2.9      0.125     0.36      0.045
;1.2-1.5     1.7      0.159     0.365     0.058
;1.5-1.8     0.5      0.178     0.37      0.066

;*** WHAT IS A TYPICAL VALUE FOR f OR fs ???
;--------------------------------------------------------------
;fs     = -(f / d_theta)     ;(passed from caller)

;** print, 'Calling ExpInf2 routine...'

C       = (d_psi * d_theta)
t_start = n * dt             ;(time at start of timestep)
t_end   = (n + 1) * dt       ;(time at end of timestep)
err     = 0.00001d           ;(tolerance)
n_max   = 20

;------------------------------------
;Initial guess for Ip, but note that
;Ip could be much bigger than Ip_hi.
;------------------------------------
Ip_hi = I + (rainrate * dt)    ;(upper bound)
Ip_lo = I                      ;(lower bound)
Ip    = Ip_hi
;-------------
;Ip   = Ip_hi * 10.0
;Ip   = (Ip_lo + Ip_hi)/2d     ;(midpoint)
;Ip   = Ip_lo     ;(Don't use this, it may be 0.)

;-----------------------------------------
;Iterate eqn (9) in Beven paper to get Ip
;and then compute time to ponding, tp.
;-----------------------------------------------
;This seems to assume that infil(Ip) is an
;increasing function, but it isn't for small Ip.
;Maybe okay if we start high and slide down.
;--------------------------------------------------
;NB!  Ip may not be initialized to zero since that
;would cause the expression for infil to blow up.
;--------------------------------------------------
n_tries   = 0
CONVERGED = (Ip ne 0.0)  ;******
while (n_tries le n_max) and NOT(CONVERGED) do begin
    infil = -K0 * fs * (C + Ip)/(1d - exp(fs * Ip)) 
    if (infil gt rainrate) then begin
        ;------------------------------
        ;Lower the upper bound and
        ;reduce Ip toward lower bound
        ;------------------------------
        Ip_hi = Ip
        Ip    = (Ip + Ip_lo) / 2.0
        CONVERGED = (abs(Ip - Ip_lo) lt err)
    endif else begin
        ;-------------------------------
        ;Raise the lower bound and
        ;increase Ip toward upper bound
        ;-------------------------------
        Ip_lo = Ip
        Ip    = (Ip + Ip_hi) / 2.0
        CONVERGED = (abs(Ip - Ip_hi) lt err)
    endelse
    n_tries = n_tries + 1
endwhile

if NOT(CONVERGED) AND (Ip ne 0.0) then begin
    print,'ERROR:  Max number of iterations exceeded'
    print,'while computing time to ponding.'
    print,' '
    RETURN  ;******
endif

;--------------------------------------
;Use Ip to check if ponding will occur
;in this timestep
;--------------------------------------
NO_PONDING = ((Ip - I) gt (rainrate * dt)) OR $
             (rainrate eq 0) ;***** OR (rainrate lt K0)   ;***********

;-----------------------------------------
;Case where there is ponding already
;by the first timestep.  Is this needed ?
;-----------------------------------------
;if (I eq 0.0) AND (n eq 1) then begin
;    NO_PONDING = 0b
;    tp = t_start
;endif

;----------------------------
;Compute I for this timestep
;----------------------------
if (NO_PONDING) then begin
    IROF = 0  ;****

    ;----------------------------
    ;All of the rain infiltrates
    ;----------------------------
    dI = rainrate * dt
    I  = I + dI
endif else begin
    IROF = 1  ;****
    ;pstr = strtrim(string(n), 2)
    ;print,'Ponding occurred at timestep = ' + pstr

    ;------------------------------------
    ;Ponding occurs in this timestep, but
    ;may have started in a previous step.
    ;Compute time to ponding, tp from Ip
    ;------------------------------------
    ;All rain infiltrates up until t=tp,
    ;then there is ponding for remainder.
    ;------------------------------------
    ;Note: (rainrate ne 0) to get here.
    ;Note: tp may be greater than t_end.
    ;Note: If (Ip lt I) then (dI lt 0).
    ;------------------------------------
    dI = (Ip - I)
    tp = t_start + (dI / rainrate)
    ;** tp = (tp > t_start)

    ;-------------------------------------
    ;Compute 10 terms in a Taylor series
    ;that depends on Ip, to compute lambda
    ;-------------------------------------
    sum       = 0d
    factorial = 1
    x         = (Ip + C)
    for j=1,10 do begin
        factorial = factorial * j
        term      = (fs * x)^j /(j * factorial)
        sum       = sum + term 
    endfor

    ;-----------------------------------------
    ;Compute the integration constant, lambda
    ;-----------------------------------------
    lambda = alog(x) - (alog(x) + sum)/exp(fs * C)

    ;---------------------------------------
    ;Initial guess for Newton-Raphson is Ip
    ;plus half of potential contribution
    ;---------------------------------------
    Ir = Ip + (0.5 * rainrate * (t_end - tp))

    ;----------------------------------
    ;Iterate eqn (8) in Beven paper to
    ;directly compute new I = I(t_end)
    ;----------------------------------
    n_tries = 0
    CONVERGED = 0b
    while (n_tries lt n_max) AND NOT(CONVERGED) do begin
        ;-------------------------------------
        ;Compute 10 terms in a Taylor series
        ;that depends on I
        ;-------------------------------------
        sum       = 0d
        factorial = 1
        x         = (Ir + C)
        for j=1,10 do begin
            factorial = factorial * j
            term      = (fs * x)^j /(j * factorial)
            sum       = sum + term 
        endfor

        ;------------------------------
        ;Use the Newton-Raphson method
        ;-------------------------------------------
        ;Recall from Beven paper that:
        ;(1 - exp(fs*I))/(I+C) * dI = -K0 * fs * dt
        ;is what was integrated for I = Ip to I_end
        ;and t = tp to t_end to get eqn (8).
        ;-------------------------------------------
        term  = (alog(x) + sum) / exp(fs * C)
        rhs   = (alog(x)- term - lambda)
        lhs   = (t_end - tp) * (-K0 * fs)
        h     = (rhs - lhs)
        dh_dI = (1d - exp(fs * Ir)) / (Ir + C)
        ;---------------------------------------------  
        ;rhs   = -(alog(x)- term - lambda) / (fs * K0)
        ;lhs   = (t_stop - tp)
        ;h     = (rhs - lhs)
        ;dh_dI = (exp(fs * Ir) - 1d) / (K0 * fs * x)
        ;---------------------------------------------
        del_I = -(h / dh_dI)
        Ir  = Ir + del_I
        CONVERGED = (abs(del_I) le err)
        n_tries = n_tries + 1
    endwhile

    if NOT(CONVERGED) then begin
        print,'Max number of iterations exceeded'
        print,'while computing cumulative infiltration.'
        print,' '
        RETURN   ;*******
    endif

    ;------------------------------------
    ;How much was contributed in this
    ;timestep, before and after tp ?
    ;------------------------------------
    dI1 = (Ip - I) > 0d
    dI2 = (Ir - Ip)

    ;print,'(Ip - I)  = ',string(dI1)
    ;print,'(Ir - Ip) = ',string(dI2)
    ;print,' '

    ;---------------------------
    ;Replace old I with new one
    ;---------------------------
    I = Ir
    ;** I = (I > Ir) ;****** I should only increase.

endelse

END;  Call_ExpInf2
;*****************************************************************
pro Check_Results, Q, Q_obs, OUTFILE=outfile

;----------------------------
;Use TOTAL to avoid for loop
;----------------------------
;Qsum = total(Q_obs, /double)
;ssQ  = total(Q_obs * Q_obs, /double)
;f1   = total((Q - Q_obs)^2d, /double
;f2   = total(abs(Q - Q_obs), /double)

n_steps = n_elements(Q)
Qsum = 0.0
ssQ  = 0.0
f1   = 0.0
f2   = 0.0

;-------------------------------
;Compute the objective function
;to compare Q to Q_observed
;-------------------------------
for n=0L,(n_steps - 1L) do begin
    Qsum = Qsum + Q_obs[n]
    ssQ  = ssQ  + (Q_obs[n] * Q_obs[n])
    f1   = f1 + (Q[n] - Q_obs[n])^2.0
    f2   = f2 + abs(Q[n] - Q_obs[n])
endfor

Qbar = (Qsum / n_steps)
varQ = (ssQ  / n_steps) - (Qbar * Qbar)
varE = (f1   / n_steps)
E    = (1.0 - (varE / varQ))

;---------------
;Print a report
;---------------
f0 = '(E14.3)'
f1str = strtrim(string(f1,   format=f0), 2)
f2str = strtrim(string(f2,   format=f0), 2)
estr  = strtrim(string(e,    format=f0), 2)
mstr  = strtrim(string(Qbar, format=f0), 2)
vstr  = strtrim(string(varQ, format=f0), 2)
vestr = strtrim(string(varE, format=f0), 2)
;-------------------------------------------
print,'Objective function values:'
print,'   f1          = ' + f1str
print,'   f2          = ' + f2str
print,'   e           = ' + estr
print,'   Mean(Q_obs) = ' + mstr
print,'   Var(Q_obs)  = ' + vstr
print,'   Var(error)  = ' + vestr
print,' '

;-----------------------
;Print report to a file
;-----------------------
if (keyword_set(outfile)) then begin
    openw, U, outfile, /get_lun
    printf,U,'Objective function values:'
    printf,U,'   f1          = ' + f1str
    printf,U,'   f2          = ' + f2str
    printf,U,'   e           = ' + estr
    printf,U,'   Mean(Q_obs) = ' + mstr
    printf,U,'   Var(Q_obs)  = ' + vstr
    printf,U,'   Var(error)  = ' + vestr
    printf,U,' '
    free_lun, U
endif

END;  Check_Results
;*****************************************************************

