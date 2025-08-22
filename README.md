# Sparse Matrix Operations Tool
----------------------------------------------------------

## Project Overview

This is a tool that perfom certain operations on sparse matrices, built using C++ Programing Languages. Supported operations include:

1. Addition
2. Subtraction
3. Multiplication

The program reads matrix data input from text files that are found in sample_inputs directory, processes them as sparse matrices, and allows you to display or save the results in sparse format.

---

## How to Use


### 1. Navigate to Github Codespaces
- Click the green **"Code"** button on this GitHub repository.
- From **"Codespaces** option select **"Create codespaces on main"**.
<img width="1918" height="972" alt="image" src="https://github.com/user-attachments/assets/630761ed-eb2c-4edf-b91c-82ff948c5831" />
Project will be opened in a VS Code–like workspace in your browser with all necessary tools pre-installed, including g++.


### 2. Move to directory where it's supposed to be run ( ../dsa/code)

``` 
cd dsa/code
```
### 3. Compile an executable C++ file with any name

```
g++ SparseMatrix.cpp -o sparse_matrix
```
this will create **sparse_matrix** file in dsa/code directory

### 4. Run the executable file created 

```
./sparse_matrix
```
### 5. Guiding through the whole process till it's finished
You will be shown the existing files to perform operations on.
**NB.** if you wanna choose Addition or Subtraction, you must choose matrix files with the same dimensions of row and columns. Those are either;** sample-01.txt to sample-02.txt** , otherwise you will see dimension error!
and for multiplication only choose **sample-01.txt** to **sample-03.txt** or **sample-02.txt** to **sample-03.txt**

### 6. Example of how result is obtained
on the following image, i perfomed addition on sample-01.txt and sample-02.txt (you can even save the result and new file will be saved under the currrent directory)

<img width="1919" height="764" alt="image" src="https://github.com/user-attachments/assets/6fd8a2ef-e59f-4afe-b76e-5e1a01660850" />

