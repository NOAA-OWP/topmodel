# define the C compiler to use
CC = gcc -g

# define any compile-time flags
# for newer compiler, the right hand side is not needed...
# ... for instance, remove the flags if using gnu/10.1.0 on Cheyenne.
CFLAGS = -fsanitize=address

INCLUDES = -I../include

LFLAGS =

LIBS = -lm

# define the C source files
SRCS = main_unit_test_bmi.c ../src/bmi_topmodel.c ../src/topmodel.c

# define the C object files 
OBJS = $(SRCS:.c=.o)

# define the executable file 
MAIN = run_bmi_unit_test

.PHONY: depend clean

all:    $(MAIN)
	@echo  Simple compiler named mycc has been compile

$(MAIN): $(OBJS) 
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

# this is a suffix replacement rule for building .o's from .c's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .c file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.c.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN)

depend: $(SRCS)
	makedepend $(INCLUDES) $^
