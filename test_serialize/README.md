# TOPMODEL Serialization and Deserialization Test

Several new functions have been added to the Basic Model Interface (BMI)
in order to support the serialization and deserialization of all of a
model's state variables.  State variables are variables that describe
the current state of the model at any given time and that persist between
BMI function calls.  This excludes variables that are created only for
temporary use within any given function.  A model's state variables will
typically not be defined until after the model's BMI initialize() function
has been called.  When a model's BMI update() function is called, the values
of many of the state variables will be modified as the model advances in
time.  If, after one or more BMI update() calls, all of a model's state
variables are serialized and saved (i.e. to a buffer or a file), it becomes
possible to deserialize them and to then set them as the state of a new
instance of the same model.  The new instance can then continue running
from the point in time that the original model's variables were serialized.

Serialization is a complex and error-prone process that model developers
should not have to be concerned with.  The strategy employed here therefore
follows a "separation of concerns" philosophy where the model developer
only implements new BMI functions that allow the model coupling
framework (e.g. nextGen) to get and set the values of all state variables
and also to retrieve metadata about them.  A separate, general-purpose
framework utility can then be written to perform the tasks of serialization
and deserialization that calls and relies on these new BMI functions.
The current implementation of this general utility uses the
[msgpack-c library](https://github.com/msgpack/msgpack-c)
and is currently found in
```topmodel/test/serialization/serialize_state.c.```
Msgpack serializes to binary and is both fast and compact.

**Note:**  You must install the msgpack-c library to run the test.
See instructions at: 
[msgpack-c library](https://github.com/msgpack/msgpack-c).

The new BMI functions that support serialization of models written in C
are as follows.  These new functions are defined in
```topmodel/include/bmi.h```.
Their implementations are straight-forward but model specific.
They have been implemented for TOPMODEL in:
```topmodel/src/bmi_topmodel.c```

```
get_state_var_count():  Return the total number of state variables
get_state_var_names():  Return string array of state variable internal names
get_state_var_types():  Return string array of state variable C types
get_state_var_ptrs():   Return pointer array to all state variables
get_state_var_sizes():  Return unsigned int array of sizes (number of elements)
set_state_var():        Set the value of a state variable given its index
```

The code for the actual test can be found in:
```topmodel/test_serialize/topmodel_serialize_test.c```,
along with a small shell script called
```make_and_run_ser_test.sh``` for making and running the test.

 
