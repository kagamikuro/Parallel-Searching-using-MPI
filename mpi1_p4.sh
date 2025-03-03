#!/bin/bash

# Execute job from current working directory
#$ -cwd

# Gives the name for output of execution
#$ -o programoutput.$JOB_ID

# Ask the scheduler for allocating 4 MPI slots
# -pe mpislots-verbose 4

# Load mpi module
module add mpi/openmpi

# Prints date
date

# Compiling the Program
mpicc searching_MPI_1.c -o mpi1

# Prints starting new job
echo "Starting new job"

# Executes the compiled program on 2 MPI processes
mpirun -np 4 mpi1

# Prints finished job
echo "Finished job"

# Prints date
date
