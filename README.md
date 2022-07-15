# TOPMODEL BMI

This model is in development for use in the
[NGen Water Modeling Framework Prototype](https://github.com/NOAA-OWP/ngen).
It includes wrapper functions for the
[Basic Model Interface](https://bmi-spec.readthedocs.io/en/latest/).
More BMI related references can be found [here](refs/csdms).
  
TOPMODEL is a physically based, distributed watershed model that simulates
hydrologic fluxes of water (infiltration-excess overland flow, saturation
overland flow, infiltration, exfiltration, subsurface flow, evapotranspiration,
and channel routing) through a watershed. The model simulates explicit
groundwater/surface water interactions by predicting the movement of the
water table, which determines where saturated land-surface areas develop
and have the potential to produce saturation overland flow. TOPMODEL was
originally developed by
[Beven & Kirby (1979)](https://www.tandfonline.com/doi/abs/10.1080/02626667909491834).

### Links and Documentation
- [INSTALL](./INSTALL.md): How to build and run topmodel-bmi
- [BMI_ADAPTION](./docs/BMI_ADAPTION.md): Details of how TOPMODEL source code was adapted and extended to BMI
- [STAND_ALONE](./docs/STAND_ALONE.md): Outlines a newly introduced Boolean toggle
- [OUTPUT_FILES_EXPLAINED](./docs/OUTPUT_FILES_EXPLAINED.md): Explanation of model output file(s) and results variables
- [INPUT_FILES_EXPLAINED](./docs/INPUT_FILES_EXPLAINED.md): Explanation of model input file(s) and associated variables
- [VARIABLE_ROLES](./docs/VARIABLE_ROLES.md): Description of variable role categories
- [BMI_UNIT_TEST](./test/README.md): Instructions on how to run various unit testing scripts
- [PARAMS](./params/README.md): Demonstrates parameter [`subcat.dat`](.data/subcat.dat) generation workflow