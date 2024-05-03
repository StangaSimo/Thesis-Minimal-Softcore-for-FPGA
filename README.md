# Thesis-Minimal-Softcore-for-FPGA

## Introduction 
The main goal of this Bachelor’s degree thesis was the development of a simple interpreter capable of executing a subset of Microblaze processor instructions, compiling it, and running it on the FPGA. Different versions of the interpreter were created to demonstrate the flexibility and configurability of this solution. Furthermore, one of the interpreter versions was used to simulate a GPU with independent cores and no SIMD controller. The operation of each version of the interpreter was verified and validated through specific ad-hoc tests. Additionally, the resource utilization of each interpreter variant on the FPGA was demonstrated and discussed. This allowed estimating the maximum number of instances that can be inserted into the device. In conclusion, all objectives were successfully achieved.

## Documentation 
The entire documentation of the entire project is located in the PDF thesis/thesis.pdf written in Italian

## Project Structure 
* **on_host** : the code that helped develop the host version of the MicroBlaze Interpreter.
* **thesis** : latex code and the pdf of the full thesis.
* **versions** : all the configuration of the interpreter for the FPGA.