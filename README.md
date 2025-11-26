`Matrix Operations Project`

Real-Time Applications & Embedded Systems — ENCS4330


 `Overview`

This project implements a matrix operations tool using:

Multi-processing with pipes, and signals

OpenMP parallelism (for hybrid execution)


Supported operations:

Enter new matrix (manual or from file)

Display / Delete / Modify matrices

Add, Subtract, and Multiply matrices

Compute Determinant

Compute Eigenvalues & Eigenvectors (Power Method)

Compare execution times between:

Single-threaded

Multi-process

Hybrid (OpenMP + Multi-process)

 `Compilation`

All build configurations are controlled through the provided Makefile.

` Build all executables`
make all

`Build single-threaded version`
make single

`Build multi-processing version`
make multi

`Build hybrid (OpenMP + Multi-processing)`
make hybrid


Each build produces one of the following executables:

Mode	Output File
Single	matrix_calc_single
Multi	matrix_calc_multi
Hybrid	matrix_calc_hybrid


` Running the Program`

Use the helper targets to compile and run:

make run-single
make run-multi
make run-hybrid


Or run manually after building as following:

./matrix_calc_multi


A menu will appear, allowing you to choose operations interactively.

`Implementation Notes`

Pipes & Forks: Used for inter-process communication -create children and make them communicate with their parent.

Signals: Used as a notification in multiple places in the code

OpenMP: Used to accelerate loop operations in hybrid mode.

Eigen computation: Uses the Power Method, parallelized across multiple processes.

Each operation measures execution time using now_ms() to help compare speed across build modes.

`Environment`


Linux environment (tested on Ubuntu / WSL)
some commands of Linex are used so this code cannot be compiled and runned on other enviroments.


To remove all executables and object files:

make clean

`Team`

Leen Alqazaqi   1220380
Leen Aldeek     1212391
Rand Awadallah  1211963
Areej Younis    1221419


