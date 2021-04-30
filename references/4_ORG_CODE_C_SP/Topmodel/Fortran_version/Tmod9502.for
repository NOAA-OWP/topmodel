C  TOPMODEL DEMONSTRATION PROGRAM VERSION 95.02
C 
C
C  Compiled using Lahey Fortran77 and Grafmatic Graphics
C
C  This version by Keith Beven 1985
C  Revised for distribution 1993,1995
C
C****************************************************************
C  This program is distributed freely with only two conditions.
C
C  1. In any use for commercial or paid consultancy purposes a 
C     suitable royalty agreement must be negotiated with Lancaster 
C     University (Contact Keith Beven)
C
C  2. In any publication arising from use for research purposes the
C     source of the program should be properly acknowledged and a 
C     pre-print of the publication sent to Keith Beven at the address
C     below.
C
C  All rights retained 1993, 1995
C  Keith Beven
C  Centre for Research on Environmental Systems and Statistics
C  Institute of Environmental and Biological Sciences
C  Lancaster University, Lancaster LA1 4YQ, UK
C
C  Tel: (+44) 1524 593892  Fax: (+44) 1524 593985
C  Email:  K.Beven@UK.AC.LANCASTER
C  
C****************************************************************
C
C  SIMPLE SUBCATCHMENT VERSION OF TOPMODEL
C
C  This program allows single or multiple subcatchment calculations 
C  but with single average rainfall and potential evapotranspiration
C  inputs to the whole catchment.  Subcatchment discharges are routed
C  to the catchment outlet using a linear routing algorithm with
C  constant main channel velocity and internal subcatchment 
C  routing velocity.  The program requires ln(a/tanB) distributions
C  for each subcatchment.  These may be calculated using the
C  GRIDATB program which requires raster elevation data as input.
C  It is recommended that those data should be 50 m resolution or
C  better.
C
C  NOTE that TOPMODEL is not intended to be a traditional model
C  package but is more a collection of concepts that can be used
C  **** where appropriate ****. It is up to the user to verify that
C  the assumptions are appropriate (see discussion in 
C  Beven et al.(1994).   This version of the model  will be
C  best suited to catchments with shallow soils and moderate
C  topography which do not suffer from excessively long dry 
C  periods.  Ideally predicted contributing areas should be
C  checked against what actually happens in the catchment.
C
C  It includes infiltration excess calculations and parameters
C  based on the exponential conductivity Green-Ampt model of 
C  Beven (HSJ, 1984) but if infiltration excess does occur it
C  does so over whole area of a subcatchment.  Spatial variability
C  in conductivities can however be handled by specifying 
C  Ko parameter values for different subcatchments, even if they
C  have the same ln(a/tanB) and routing parameters, ie. to 
C  represent different parts of the area. 
C
C  Note that time step calculations are explicit ie. SBAR
C  at start of time step is used to determine contributing area.  
C  Thus with long (daily) time steps contributing area depends on 
C  initial value together with any volume filling effect of daily 
C  inputs.  Also baseflow at start of time step is used to update 
C  SBAR at end of time step
C
C  Current program limits are:
C          Number of time steps = 2500
C          Number of subcatchments = 10
C          Number of ln(a/tanB) increments = 30
C          Number of subcatchment routing ordinates = 10
C          Number of time delay histogram ordinates = 20
C          Size of subcatchment pixel maps = 100 x 100
C
C  Limits are mostly set in Common blocks in file TMCOMMON.FOR
C*****************************************************************
C
C  This version uses five files as follows:
C       Channel 4 "TOPMOD.DAT" contains run and file information
C       Channel 7 <INPUTS$> contains rainfall, pe and qobs data
C       Channel 8 <SUBCAT$> contains subcatchment data      
C       Channel 9 <PARAMS$> contains parameter data
C       Channel 10 <OUTPUT$> is output file

C  In addition
C       Channel 12 <MAPFILE$> is used to read subcatchment ln(a/tanB)
C                   maps if IMAP = 1
C
C
C*****************************************************************
C
      INCLUDE TMCOMMON.FOR
      CHARACTER*15 INPUTS$,SUBCAT$,PARAMS$,OUTPUT$
      OPEN(4,FILE="TOPMOD.RUN",STATUS="OLD")
      READ(4,"(A)")TITLE
      READ(4,"(A)")INPUTS$
      READ(4,"(A)")SUBCAT$
      READ(4,"(A)")PARAMS$
      READ(4,"(A)")OUTPUT$
      OPEN(7,FILE=INPUTS$,STATUS="OLD")
      OPEN(8,FILE=SUBCAT$,STATUS="OLD")
      OPEN(9,FILE=PARAMS$,STATUS="OLD")
      OPEN(10,FILE=OUTPUT$)
      WRITE(10,1001)TITLE
 1001 FORMAT(1x,A)
      Write(6,1002)title
 1002 Format(///1x,'TOPMODEL Version: TMOD95.02'////
     11x,'This run :'/1x,A//////////
     11x,'Centre for Research on Environmental Systems and Statistics'/
     21x,'Lancaster University, Lancaster LA1 4YQ, UK')
      Write(6,602)
  602 format(/1x,'                       Press return to continue'/)
      Read(5,*)
C
C  READ IN DT and RAINFALL, PE, QOBS INPUTS
      CALL INPUTS
C
C  READ IN SUBCATCHMENT TOPOGRAPHIC DATA
      READ(8,*)NSC,IMAP,IOUT
C
C  OPEN PARAMETER FILE

C  START LOOP ON SUBCATCHMENTS
      DO 10 ISC=1,NSC
      If(iout.ge.2)Write(10,600)ISC
  600 Format(1x,'Starting Subcatchment',I6)
C
C  INITIALISATION FOR THIS SUBCATCHMENT
      CALL TREAD
      CALL INIT
C
C  RUN MODEL FOR THIS SUBCATCHMENT INCLUDING LINEAR ROUTING CALCULATIONS
      CALL TOPMOD
      
C
C  END LOOP ON SUBCATCHMENTS
C
   10 CONTINUE
C  CALL RESULTS ROUTINE:  if IRUN = 0 on return stop
      CALL RESULTS
c  IRUN Disabled at present
      CLOSE(5)
      CLOSE(7)
      CLOSE(8)
      CLOSE(9)
      CLOSE(10)
      STOP
      END
C
C
      SUBROUTINE TOPMOD
C
      INCLUDE TMCOMMON.FOR
      DIMENSION EX(30)
C
C*****************************************************************
C
C  THIS ROUTINE RUNS TOPMODEL FOR ONE SUBCATCHMENT, INCLUDING THE
C  LINEAR CHANNEL ROUTING CALCULATIONS.
C
C  The calculations are made for areal subdivisions based on the
C  NAC ln(a/tanB) subdivisions.  The saturation deficit for each 
C  subdivision is calculated from SBAR at the start of each time
C  step.
C
C  Each increment also has a root zone storage (SRZ) deficit which
C  is 0 at 'field capcacity' and becomes more positive as the soil
C  dries out; and an unsaturated zone storage (SUZ) which is zero at
C  field capacity and becomes more positive as storage increases.
C  SUZ has an upper limit of the local saturation deficit SD.
C  The local contributing area is where SD - SUZ is less than or
C  equal to zero.
C
C  REMEMBER SBAR,SD AND SRZ VALUES ARE DEFICITS; SUZ IS A STORAGE.
C
******************************************************************
      IROF=0
      REX=0.
      CUMF=0.
      ACMAX=0.
      SUMP=0.
      SUMAE = 0.
      SUMQ=0.
C
C  Initialise contributing area counts
      IHROF = 0
      do 5 ia = 1, nac
    5 ihour(ia)=0
C
C  START LOOP ON TIME STEPS
      If(IOUT.ge.2)Write(10,101)
  101 format(1x,'  it       p        ep       q(it)       quz',
     1'      q       sbar       qof')
C
      DO 10 IT=1,NSTEP
      QOF=0.
      QUZ=0.
C
      EP=PE(IT)
      P=R(IT)
      SUMP = SUMP + P      
C
C  SKIP INFILTRATION EXCESS CALCULATIONS IF INFEX = 0
      IF(INFEX.EQ.1) THEN
C
C****************************************************************
C  INFILTRATION EXCESS CALCULATIONS USING EXPINF ROUTINE BASED ON
C  GREEN-AMPT INFILTRATION IN A SOIL WITH CONDUCTIVITY DECLINING
C  EXPONENTIALLY WITH DEPTH (REF. BEVEN, HSJ, 1984)
C
C  NOTE THAT IF INFILTRATION EXCESS DOES OCCUR IT WILL DO SO OVER
C  THE WHOLE SUBCATCHMENT BECAUSE OF HOMOGENEOUS SOIL ASSUMPTION
C
C  ALL PARAMETERS AND VARIABLES ON INPUT MUST BE IN M/H
C
C  THIS SECTION CAN BE OMITTED WITHOUT PROBLEM
C************************************************************8***
	   IF(P.GT.0.)THEN
C
C  Adjust Rainfall rate from m/time step to m/h
	       RINT = P/DT
	       CALL EXPINF(IROF,IT,RINT,DF,CUMF)
C  DF is volumetric increment of infiltration and is returned in m/DT
	       REX = P - DF
	       P= P - REX
	If(IROF.EQ.1)IHROF = IHROF + 1
	   ELSE
	       REX=0.
	IROF=0
	       CUMF=0.
	   ENDIF
       ENDIF
C****************************************************************
C
C P IS RAINFALL AVAILABLE FOR INFILTRATION AFTER SURFACE CONTROL
C   CALCULATION
C
      ACM=0.
C  START LOOP ON A/TANB INCREMENTS
      DO 30 IA=1,NAC
      ACF=0.5*(AC(IA)+AC(IA+1))
      UZ=0.
      EX(IA)=0.
C
C  CALCULATE LOCAL STORAGE DEFICIT
      SD(IA)=SBAR+SZM*(TL-ST(IA))
      IF(SD(IA).LT.0.)SD(IA)=0.
C
C  ROOT ZONE CALCULATIONS
      SRZ(IA) = SRZ(IA) - P
      IF(SRZ(IA).LT.0.)THEN
	  SUZ(IA) = SUZ(IA) - SRZ(IA)
	  SRZ(IA) = 0.
      ENDIF
C
C  UZ CALCULATIONS
      IF(SUZ(IA).GT.SD(IA))THEN
	  EX(IA) = SUZ(IA) - SD(IA)
	  SUZ(IA)=SD(IA)
      ENDIF
C
C  CALCULATE DRAINAGE FROM SUZ
      IF(SD(IA).GT.0.)THEN
	  UZ=SUZ(IA)/(SD(IA)*TD*DT)
	  IF(UZ.GT.SUZ(IA))UZ=SUZ(IA)
	  SUZ(IA)=SUZ(IA)-UZ
	  IF(SUZ(IA).LT.0.0000001)SUZ(IA)=0.
	  QUZ=QUZ+UZ*ACF
      ENDIF
C
C***************************************************************
C  CALCULATE EVAPOTRANSPIRATION FROM ROOT ZONE DEFICIT
C
      EA=0.
      IF(EP.GT.0.)THEN
	  EA=EP*(1 - SRZ(IA)/SRMAX)
	  IF(EA.GT.SRMAX-SRZ(IA))EA=SRMAX-SRZ(IA)
	  SRZ(IA)=SRZ(IA)+EA
      ENDIF
      SUMAE = SUMAE + EA * ACF
      SAE = SAE + EA *ACF
C
C***************************************************************
C
C
C  CALCULATION OF FLOW FROM FULLY SATURATED AREA
C  This section assumes that a/tanB values are ordered from high to low
C
      OF=0.
      IF(IA.GT.1)THEN
	  IB=IA-1
	  IF(EX(IA).GT.0.)THEN
c  Both limits are saturated
		  OF=AC(IA)*(EX(IB)+EX(IA))/2
		  ACM=ACM+ACF
		  ihour(ib) = ihour(ib) + 1
	   ELSE
c  Check if lower limit saturated (higher a/tanB value)
	       IF(EX(IB).GT.0.)THEN
		   ACF=ACF*EX(IB)/(EX(IB)-EX(IA))
		   OF=ACF*EX(IB)/2
		   ACM=ACM+ACF
		   ihour(ib) = ihour(ib) + 1
	       ENDIF
	   ENDIF
      ENDIF
      QOF=QOF+OF
C
C  Set contributing area plotting array
      CA(IT) = ACM
      IF(ACM.GT.ACMAX)ACMAX=ACM
C
C  END OF A/TANB LOOP
   30 CONTINUE

C
C  ADD INFILTRATION EXCESS
      QOF=QOF+REX

      IF(IROF.EQ.1)ACMAX=1.
C
C  CALCULATE SATURATED ZONE DRAINAGE
      QB=SZQ*EXP(-SBAR/SZM)
      SBAR=SBAR-QUZ+QB
      QOUT=QB+QOF
      SUMQ=SUMQ+QOUT
C
C  CHANNEL ROUTING CALCULATIONS
C  allow for time delay to catchment outlet ND as well as 
C  internal routing array
      DO 40 IR=1,NR
      IN=IT+ND+IR-1
      IF(IN.GT.NSTEP)GO TO 10
      Q(IN)=Q(IN)+QOUT*AR(IR)
   40 CONTINUE
C
      If(IOUT.ge.2) write(10,100)it, p, ep, q(it), quz, qb, sbar, qof
  100 format(1x,i4,7e10.3)

C  END OF TIME STEP LOOP
   10 CONTINUE
C
C  CALCULATE BALANCE TERMS
      SUMRZ = 0.
      SUMUZ = 0.
      DO 50 IA =1,NAC
      ACF=0.5*(AC(IA)+AC(IA+1))
      SUMRZ = SUMRZ + SRZ(IA)*ACF
      SUMUZ = SUMUZ + SUZ(IA)*ACF 
   50 CONTINUE
      BAL = BAL + SBAR +SUMP - SUMAE - SUMQ + SUMRZ - SUMUZ
      Write(10,650)SUBCAT,SUMP,SUMAE,SUMQ,SUMRZ,SUMUZ,SBAR,BAL
      WRITE(6,650)SUBCAT,SUMP,SUMAE,SUMQ,SUMRZ,SUMUZ,SBAR,BAL
  650 FORMAT(1X,'Water Balance for Subcatchment : ',A/
     11x,'   SUMP      SUMAE       SUMQ      SUMRZ   ',
     2   '  SUMUZ      SBAR        BAL'/7e11.4)
      If(IOUT.ge.1)WRITE(10,651)ACMAX
  651 FORMAT(1X,'Maximum contributing area ', e12.5)
      RETURN
      END
*
C***************************************************************
*
      SUBROUTINE INPUTS

*
      INCLUDE TMCOMMON.FOR
*
*  This subroutine must read in rainfall, pe and observed 
*  discharges for T = 1,NSTEP with time step DT hours
*
      READ(7,*)NSTEP,DT
      READ(7,*)(R(I),PE(I),QOBS(I),I=1,NSTEP)
      CLOSE(7)
      DO 10 IT = 1,NSTEP
   10 Q(IT)=0.
      RETURN
      END
C
C**************************************************************
C
      SUBROUTINE TREAD
C
      INCLUDE TMCOMMON.FOR
C      
      READ(8,"(A)")subcat
      Write(10,1010)subcat
 1010 Format(1x,'Subcatchment : ',A)
      READ(8,*)NAC,AREA
*  NAC IS NUMBER OF A/TANB ORDINATES
*  AREA IS SUBCATCHMENT AREA AS PROPORTION OF TOTAL CATCHMENT 
      READ(8,*)(AC(J),ST(J),J=1,NAC)
*  AC IS DISTRIBUTION OF AREA WITH LN(A/TANB)
*  ST IS LN(A/TANB) VALUE
       tarea = ac(1)
       do 10 j=2,nac
       tarea = tarea + ac(j)
   10  continue

*
*  CALCULATE AREAL INTEGRAL OF LN(A/TANB)
*  NB.  a/tanB values should be ordered from high to low with ST(1)
*  as an upper limit such that AC(1) should be zero, with AC(2) representing
*  the area between ST(1) and ST(2)
      TL=0.
      AC(1)=AC(1)/tarea
      SUMAC=AC(1)
      DO 11 J=2,NAC
      AC(J)=AC(J)/tarea
      SUMAC=SUMAC+AC(J)
      TL=TL+AC(J)*(ST(J)+ST(J-1))/2
   11 CONTINUE
      AC(NAC+1)=0.
*
*  READ CHANNEL NETWORK DATA
      READ(8,*)NCH
      READ(8,*)(ACH(J),D(J),J=1,NCH)
*  ACH IS CUMULATIVE DISTRIBUTION OF AREA WITH DISTANCE D
*  FROM OUTLET.  D(1) is distance from subcatchment outlet
*  ACH(1) = 0.
*
      If(IOUT.ge.1)Write(10,600)TL, SUMAC
  600 Format(1x,'TL = ',f8.2,/'SUMAC = ', f8.2)
      RETURN
      END
*
C***************************************************************
*
*
      SUBROUTINE INIT
      DIMENSION TCH(10)
      INCLUDE TMCOMMON.FOR
*
*  READ PARAMETER DATA
      READ(9,"(A)")SUBCAT
      READ(9,*)SZM,T0,TD,CHV,RV,SRMAX,Q0,SR0,INFEX,XK0,HF,DTH
*
*  Convert parameters to m/time step DT
*  with exception of XK0 which must stay in m/h
*                    Q0 is already in m/time step
*                    T0 is input as Ln(To)
      RVDT = RV * DT
      CHVDT = CHV * DT
      T0DT = T0 + ALOG(DT)

*  Calculate SZQ parameter
      SZQ = EXP(T0DT-TL)
**
*  CONVERT DISTANCE/AREA FORM TO TIME DELAY HISTOGRAM ORDINATES
*  
      TCH(1) = D(1)/CHVDT
      DO 15 J = 2,NCH
      TCH(J) = TCH(1) + (D(J) - D(1))/RVDT
   15 CONTINUE
      NR = INT(TCH(NCH))
      IF(FLOAT(NR).LT.TCH(NCH))NR=NR+1
      ND = INT(TCH(1))
      NR = NR - ND
      DO 20 IR=1,NR
      TIME = ND+IR
      IF(TIME.GT.TCH(NCH))THEN
	 AR(IR)=1.0
      ELSE
	 DO 21 J=2,NCH
	 IF(TIME.LE.TCH(J))THEN
	    AR(IR)=ACH(J-1)+(ACH(J)-ACH(J-1))*(TIME-TCH(J-1))/
     1      (TCH(J)-TCH(J-1))
	    GOTO 20
	 ENDIF
   21    CONTINUE
      ENDIF
   20 CONTINUE
      A1= AR(1)
      SUMAR=AR(1)
      AR(1)=AR(1)*AREA
      IF(NR.GT.1)THEN
	 DO 22 IR=2,NR
	 A2=AR(IR)
	 AR(IR)=A2-A1
	 A1=A2
	 SUMAR=SUMAR+AR(IR)
	 AR(IR)=AR(IR)*AREA
   22    CONTINUE
      ENDIF
      If(IOUT.ge.1)write(10,603)szq
  603 format(1x,'SZQ  ',e12.5)
      If(IOUT.ge.1)WRITE(10,604)TCH(NCH),SUMAR,(AR(IR),IR=1,NR)
  604 FORMAT(1X,'SUBCATCHMENT ROUTING DATA'/
     1  1X,'Maximum Routing Delay  ',E12.5/
     2  1X,'Sum of histogram ordinates  ',f10.4/(1X,5E12.5))
*
*  INITIALISE SRZ AND Q0 VALUES HERE
*  SR0 IS INITIAL ROOT ZONE STORAGE DEFICIT BELOW FIELD CAPACITY
*  Q0 IS THE INITIAL DISCHARGE FOR THIS SUBCATCHMENT
*
*  INITIALISE STORES
      DO 25 IA=1,NAC
      SUZ(IA)=0.
   25 SRZ(IA)=SR0
      SBAR=-SZM*ALOG(Q0/SZQ)
c
c  Reinitialise discharge array
      SUM=0.
      DO 29 I=1,ND
   29 Q(I) = Q(I) + Q0*AREA
      DO 30 I=1,NR
      SUM=SUM+AR(I)
      IN = ND + I 
   30 Q(IN)=Q(IN)+Q0*(AREA-SUM)
*
*  Initialise water balance.  BAL is positive for storage
      BAL = - SBAR - SR0
      If(IOUT.ge.1)Write(10,605)BAL,SBAR,SR0
  605 Format(1x,'Initial Balance BAL ',e12.5/
     1       1x,'Initial SBAR        ',e12.5/
     2       1x,'Initial SR0         ',e12.5)
*
      RETURN
      END
C
C**************************************************************
C
      SUBROUTINE EXPINF(IROF,IT,RINT,DF,CUMF)
C
      INCLUDE TMCOMMON.FOR
      DOUBLE PRECISION CONST,SUM,FC,FUNC,CD,SZF,XKF
      DATA E/0.00001/
C*************************************************************
C
C  SUBROUTINE TO CALCULATE INFILTRATION EXCESS RUNOFF USING THE
C  EXPONENTIAL GREEN-AMPT MODEL.
C
C**************************************************************
C
C
C  Note that HF and DTH only appear in product CD
      CD=HF*DTH
      SZF = 1./SZM
      XKF = XK0
      IF(IROF.EQ.1)GO TO 10
C  PONDING HAS ALREADY OCCURRED - GO TO EXCESS CALCULATION
C
      IF(CUMF.EQ.0.)GOTO 7
C  FIRST TIME STEP, OVERFLOW IF CUMF=0, GO DIRECT TO F2 CALCULATION
C  INITIAL ESTIMATE OF TIME TO PONDING
      F1=CUMF
      R2=-XKF*SZF*(CD+F1)/(1-EXP(SZF*F1))
      IF(R2.LT.RINT)THEN
C  PONDING STARTS AT BEGINNING OF TIME STEP
      TP=(IT-1.)*DT
      IROF=1
      F=CUMF
      GO TO 8
      ENDIF
    7 F2=CUMF+DT*RINT
      IF(F2.EQ.0.)GO TO 20
      R2=-XKF*SZF*(CD+F2)/(1-EXP(SZF*F2))
      IF(R2.GT.RINT)GO TO 20
      F=CUMF+R2*DT
      DO 9 I=1,20
      R2=-XKF*SZF*(CD+F)/(1-EXP(SZF*F))
      IF(R2.GT.RINT)THEN
	  F1=F
	  F=(F2+F)*0.5
      IF(ABS(F-F1).LT.E)GO TO 11
      ELSE
	  F2=F
	  F=(F1+F)*0.5
      IF(ABS(F-F2).LT.E)GO TO 11
      ENDIF
    9 CONTINUE
      WRITE(6,600)
  600 FORMAT(1X,'MAXIMUM NO OF ITERATIONS EXCEEDED')
   11 CONTINUE
      TP=(IT-1)*DT+(F-CUMF)/RINT
      IF(TP.GT.IT*DT)GO TO 20
C
C  SET UP DEFINITE INTEGRAL CONSTANT USING FP
C
    8 CONST =0
      FAC=1
      FC=(F+CD)
      DO 12 J=1,10
      FAC=FAC*J
      ADD=(FC*SZF)**J/(J*FAC)
      CONST=CONST+ADD
   12 CONTINUE
      CONST=DLOG(FC)-(DLOG(FC)+CONST)/DEXP(SZF*CD)
C
      IROF=1
      F=F+0.5*RINT*(IT*DT-TP)
   10 CONTINUE
C
C  NEWTON-RAPHSON SOLUTION FOR F(T)
      DO 14 I=1,20
C
C  CALCULATE SUM OF SERIES TERMS
      FC=(F+CD)
      SUM=0.
      FAC=1.
      DO 13 J=1,10
      FAC=FAC*J
      ADD=(FC*SZF)**J/(J*FAC)
      SUM=SUM+ADD
   13 CONTINUE
      FUNC=-(DLOG(FC)-(DLOG(FC)+SUM)/DEXP(SZF*CD)-CONST)/(XKF*SZF)
     1     -(IT*DT-TP)
      DFUNC=(EXP(SZF*F)-1)/(XKF*SZF*FC)
      DF=-FUNC/DFUNC
      F=F+DF
      IF(ABS(DF).LE.E)GO TO 15
   14 CONTINUE
      WRITE(6,600)
   15 CONTINUE
      IF(F.LT.CUMF+RINT)THEN
	DF=F-CUMF
	CUMF=F
C  SET UP INITIAL ESTIMATE FOR NEXT TIME STEP
	F=F+DF
	RETURN
      ENDIF
   20 CONTINUE
C  THERE IS NO PONDING IN THIS TIME STEP
      IROF=0
      DF = RINT*DT
      CUMF=CUMF+DF
      RETURN
      END
C
C
C***************************************************************
C
      SUBROUTINE RESULTS
      INCLUDE TMCOMMON.FOR
C
C  OBJECTIVE FUNCTION CALCULATIONS
      F1=0.
      F2=0.
      SUMQ=0.
      SSQ=0.
      DO 60 IT=1,NSTEP
      SUMQ=SUMQ+QOBS(IT)
      SSQ = SSQ + QOBS(IT)*QOBS(IT)
      F1=F1 + (Q(IT)-QOBS(IT))**2
      F2=F2 + ABS(Q(IT)-QOBS(IT))
   60 CONTINUE
      QBAR = SUMQ / NSTEP
      VARQ = (SSQ/NSTEP - QBAR*QBAR)
      VARE = F1/NSTEP
      E=1-VARE/VARQ
c
c  add objective function values to output file
      write(6,621)f1,e,f2,qbar,varq,vare
      write(10,621)f1,e,f2,qbar,varq,vare
  621 format(//1x,'Objective function values'/
     1 1x,'F1 ',e12.5,'   E ',f12.5,'   F2 'e12.5//
     2 1x,'Mean Obs Q ',e12.5,'    Variance Obs Q ',e12.5/
     3 '     Error Variance',e12.5)
c
c
c
      RETURN
      END
C
C**************************************************************
C

