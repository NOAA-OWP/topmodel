cmake_minimum_required(VERSION 3.17)

option(TESTS_WITH_ASAN  "Enable AddressSanitizer on the test targets"   ON)

project(topmodelbmi VERSION 1.0.0 DESCRIPTION "OWP TOPMODEL Model and BMI Module Shared Library")

set(TOPMODEL_LIB_NAME_CMAKE topmodelbmi)
set(TOPMODEL_LIB_DESC_CMAKE "OWP TOPMODEL Model and BMI Module Shared Library")

####################
# Some global properties
set_property(GLOBAL PROPERTY C_STANDARD 99)      # Note that code requires minimum of C99 standard to compile
set_property(GLOBAL PROPERTY C_STANDARD_REQUIRED ON)
####################

####################
# Stuff for shared library
if(WIN32)
    add_library(topmodelbmi src/bmi_topmodel.c src/topmodel.c)
else()
    add_library(topmodelbmi SHARED src/bmi_topmodel.c src/topmodel.c)
endif()

target_compile_definitions(topmodelbmi PRIVATE BMI_ACTIVE)
target_include_directories(topmodelbmi PRIVATE include)
set_target_properties(topmodelbmi PROPERTIES
        VERSION ${PROJECT_VERSION}
        PUBLIC_HEADER include/bmi_topmodel.h
)
####################

####################
# Stuff for standalone driver executable
add_executable(topmodeldriver src/main.c src/bmi_topmodel.c src/topmodel.c)
target_include_directories(topmodeldriver PRIVATE include)
####################

####################
# Stuff for simple test routine
add_executable(simple_unit_tests test/main_unit_test_bmi.c src/bmi_topmodel.c src/topmodel.c)
target_compile_definitions(simple_unit_tests PRIVATE REPO_ROOT_DIR="${CMAKE_CURRENT_SOURCE_DIR}")
if (TESTS_WITH_ASAN)
    # Seems this should work in all compiler
    target_compile_options(simple_unit_tests PRIVATE -fsanitize=address)
    target_link_options(simple_unit_tests PRIVATE -fsanitize=address)
endif ()
####################

include(GNUInstallDirs)

install(TARGETS topmodelbmi
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})