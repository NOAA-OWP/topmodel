# Variable Roles in the Basic Model Interface (BMI)

Here, we use the term “variable” very broadly (as a “catch all”) for any type of variable that appears in a computer model.
The purpose of this document is to define a set of distinct and non-overlapping “roles” that a variable can play in a model.
The purpose of introducing variable roles is to support tasks such as serialization and calibration that require working with some subset of all model variables.
Note that a variable’s role may sometimes change based on a setting or option in the model’s configuration file.
For example, some variables may only be computed and made available as outputs when a certain option is activated.

0. **all**:   This special role name is used to refer to the set of all model variables, except for purely “local” variables that are only initialized and used within a function (like a loop counter) but are neither an argument nor return value of that function.
This is the set of variables that must be serialized if we wish to save the complete “state” of the computer model.
(Here, we are thinking of “state” in the computer science sense, as “remembered information” or a “stateful system”.)
Purely local variables do not need to be serialized since models are not serialized “mid function”, but only after a BMI `initialize()` or `update()` call.
Note that this role name is not assigned to any variable in the model, but can be used in any BMI function that has a “role” argument; i.e.  `Get_model_var_count()` shown below.
```
static int Get_model_var_count (Bmi *self, int * count, char *role)
{
    // If role is "all", don't filter just return VAR_NAME_COUNT
    if (strcmp(role, "all") == 0) {
        *count = VAR_NAME_COUNT;
        return BMI_SUCCESS;
    }  
```


1. **array_length**:  Models often contain arrays (1D, 2D, 3D, etc.) and some of the variables in the model are used to set the dimensions of these arrays.
They often have names that start with `num` or `n` or `max` as in `num_steps`, `num_items`, `ncols`, `nrows`, `nx`, `ny`, `nz`, `n_steps`, `max_length`, etc.

2. **constant**:   These are physical or mathematical constants, like `pi`, the gravitational constant, `g”`, or the speed of light, `c`.
This role is similar but distinct from the role of a “fixed parameter” (see below).
Einstein’s famous equation:   `e = m*c^2` contains `c`, the speed of light constant, but has `m `as the independent variable and `e` as the dependent variable.
It can be useful to retrieve variables that have this role, since different models may use a different number of significant digits, or could contain typos, etc.

3. **diagnostic**:   These are variables that are introduced for the purpose of “checking” or “diagnosing” the model’s performance.
This includes variables to check conservation of mass, momentum, energy, volume, charge, number, etc.  as well as sums.
Additional examples would be the number of `NaN` values, nodata values, out-of-range values, warnings or corrupt files that the model encounters.
Or the number of times some other condition occurs.
Or comparison to a closed form solution.

4. **directory**:   The name of a directory that the model may read from (for input) or write to (to save output).
A directory will always be a string.

5. **filename**:   The name of a file that the model may read from (for input) or write to (to save output).
A filename will always be a string and may include a complete file path.
Note that this is not for file pointers.

6. **file_offset**:  This refers to a variable that stores the offset, in bytes, of the current position of a file pointer.
If a model is ever serialized while it is reading from or writing to an output file, this will be needed so that a new instance of the model can continue reading or writing from the same position in the file.

7. **info_string**:   A string or `char[]` variable (non-numeric) that provides model details, such as `title` or `catchment_name`.
This can also be used for a model version string, model name, author name, author email, or the name of a river basin, etc.

8. **input_from_bmi**:  These are model input variables whose names are returned via the BMI `get_input_var_names()` function, or the new `get_var_names()` function with the role set to **input_from_bmi**.
The framework may get the values of these variables from other BMI objects using their BMI `get_value()` function and then set them in this model using its BMI `set_value()` function.
This will often include “forcing variables”, “climatic variables”, or “environmental variables” which may or may not vary with time.
These variables (as exchange items) need to have standardized variable names to facilitate accurate model coupling, but they typically have a shorter, internal name.

9. **input_from_file**:  Similar to **input_from_bmi**, except these variables are read from a file by the model itself.
Often, a model’s configuration file will contain filenames that the model is supposed to read to obtain initial values, boundary conditions, forcing variables, etc.
Examples are initial snow depth, initial water depth, and initial soil moisture.
Another example is observational data that the model may read in for a region of interest and use for calibration.

10. **not_set**:  This special role name can be used to simplify BMI implementation.
It indicates that the role has not yet been determined or set for this variable.
It can also be used for any variable for which no existing role name is appropriate.

11. **option**:  An option will often be a `Boolean` variable (on/off, yes/no, true/false, etc.), but can also be an enumerated variable with more than two possible settings.
Also called flags, switches or toggles, these are set at the beginning of a model run (and held fixed for the duration of a run) to control some aspect of the model’s behavior, like its verbosity, whether it prints output to files, or whether to use a Cartesian or polar coordinate system.

12. **output_to_bmi**:  These are model output variables whose names are returned via the BMI `get_output_var_names()` function, or the new `get_model_var_names()` function with the role set to `output_to_bmi`.
The framework may get the values of these variables from this model using its BMI `get_value()` function and then set them in other BMI objects using their BMI `set_value()` function.
These variables (as exchange items) need to have standardized variable names to facilitate accurate model coupling, but they typically have a shorter, internal name.

13. **output_to_file**:  These are output variables that the model writes to a file.
These could potentially overlap with the **output_to_bmi** variables, but the goal is to allow only one role name for each variable.

14. **parameter_adjustable**:  In mathematics (see References), the word parameter often has a specialized meaning and the phrases “control parameter”, “design parameter”, “fitting parameter” or “model parameter” are synonyms.
The simplest concrete example is the general equation for a line:  `y = m*x + b`.
Here, `x` is the independent (or input) variable, `y` is the dependent (or output) variable and `m`and `b` are parameters (slope and y-intercept).
To use the equation for a line, `m` and `b` are first set or fixed and then the equation is evaluated for different values of `x`.
Another example is the general power-law equation:  `y = c * x^p`, where `c` is the coefficient and `p` is the power.
When we use a general function class like this, we often “adjust” the parameters to “fit” data that we have for `x` and `y`.
A distinguishing feature of parameters is that they are fixed at the beginning of a model run, but are often adjusted/varied between model runs to calibrate or “fit” the model so that predictions match observations as closely as possible.
A model should assign this role name to any parameter that it allows to be varied for calibration.

Note:  It would be good to have a `get_var_min()` and `get_var_max()` function in BMI, especially for use with adjustable parameters.

Note:  If no variables have this role, it indicates that the model cannot be calibrated.  A role name like **not_set** or “internal” could be used as a placeholder to simplify BMI implementation.

15. **parameter_fixed**:  This role name is used for the purpose of indicating that the given model parameter is to be held fixed and not adjusted for model calibration.
The only difference between the roles **fixed parameter** and **constant** is that the value of a fixed parameter could potentially be varied but is deliberately held fixed, while a constant always has a fixed value.
Density of water could potentially qualify as having this role, since it is often assumed to have a fixed value even though it can change with temperature.

16. **state**:  A computational model is usually trying to represent the “state” of some physical system, and to predict how the variables that describe this system state vary over time.
For example, the air in a piston may be modeled with the Ideal Gas Law, `PV = nRT`, and the state of the piston-air system is described by the variables `P` (pressure), `V` (volume), `T` (temperature) and `n` (amount of substance) as well as the ideal gas constant, `R`.
This example shows that state variables don’t necessarily change over time, because it is possible for two of these to be held fixed while the other two are allowed to vary.
(They are still describing the system state when fixed.)
The system being modeled may have other properties that describe the system state but that don’t change over time, like total mass, momentum, energy, charge or volume.
In a given model, a system state variable may be treated as either an independent (input) or dependent (output) variable.
So input and output variables often qualify as state variables, but this role name is reserved for any system state variables that are *not* treated as model input or output variables.
The potential and kinetic energies of a projectile provide another example of state variables.
“State space” refers to the set of all possible states that a system can be in (i.e. all valid combinations of state variables), which is often both infinite and constrained.

17. **time_info**:   Computational models that predict values of variables at future times from initial values using some set of equations are often called “time-stepping” models.
At a minimum, a timestep size and a number of timesteps or a “time index” are required.
But many models also include “clock time”, calendar dates or ISO 8601 standard “datetimes”.
There can also be a `start_time`, `current_time`, `end_time`, or `run_time`.


## References

- Dependent and independent variables: https://en.wikipedia.org/wiki/Dependent_and_independent_variables
- Parameter (mathematics): https://www.britannica.com/topic/parameter
- State variable: https://en.wikipedia.org/wiki/State_variable
- Equation of state: https://en.wikipedia.org/wiki/Equation_of_state
- State (computer science): https://en.wikipedia.org/wiki/State_(computer_science)
- State space: https://en.wikipedia.org/wiki/State_space
