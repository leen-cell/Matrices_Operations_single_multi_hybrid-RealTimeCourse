# ========================================
# Matrix Project - Universal Makefile
# ========================================

CC      = gcc
CFLAGS  = -Wall -g
OMPFLAG = -fopenmp
LIBS    = -lm

# All source files
SRC = main.c matrix_calc.c matrix_io_mem.c matrix_multi.c matrix_omp_multi.c

# Targets
all: single multi hybrid

# -------------------------------
# Single (Sequential)
# -------------------------------
single:
	$(CC) $(CFLAGS) $(OMPFLAG) -DSINGLE -o matrix_calc_single $(SRC) $(LIBS)

# -------------------------------
# Multi-processing
# -------------------------------
multi:
	$(CC) $(CFLAGS) $(OMPFLAG) -DMULTI -o matrix_calc_multi $(SRC) $(LIBS)

# -------------------------------
# Hybrid (OpenMP + Multi-processing)
# -------------------------------
hybrid:
	$(CC) $(CFLAGS) $(OMPFLAG) -DHYBRID -o matrix_calc_hybrid $(SRC) $(LIBS)

# ------------------------------
# Clean up
# -------------------------------
clean:
	rm -f matrix_calc_single matrix_calc_multi matrix_calc_hybrid *.o

# -------------------------------
# Run helpers
# -------------------------------
run-single: single
	./matrix_calc_single

run-multi: multi
	./matrix_calc_multi

run-hybrid: hybrid
	./matrix_calc_hybrid
