#Topmodel BMI
Developing a BMI topmodel function based on tmod9502.c, a C code written based on the original Beven Keith code Fortran code. See specifications for the original code at: https://csdms.colorado.edu/wiki/Model:TOPMODEL

tmod9502.c

  TOPMODEL DEMONSTRATION PROGRAM VERSION 95.02

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


*****************************************************************
