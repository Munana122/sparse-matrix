#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>
#include <algorithm> // For sorting triplets

using namespace std;

// Structure for a triplet (row, col, value)
struct Triplet {
    int row;
    int col;
    int value;
    
    // Comparator for sorting triplets
    bool operator<(const Triplet& other) const {
        if (row != other.row) return row < other.row;
        return col < other.col;
    }
};

// Structure for CSR (for multiplication)
struct CSR {
    int* values;       // Non-zero values
    int* col_indices;  // Column indices
    int* row_ptr;      // Row pointers
    int nnz;           // Number of non-zero elements
    int rows;          // Number of rows
    int cols;          // Number of columns
};

// Class for sparse matrix
class SparseMatrix {
private:
    int rows, cols;
    Triplet* triplets; // Dynamic array of triplets
    int nnz;           // Number of non-zero elements
    int capacity;      // Capacity of triplets array

    // Resize the array if necessary
    void resize(int new_capacity) {
        Triplet* new_triplets = new Triplet[new_capacity];
        for (int i = 0; i < nnz; i++) {
            new_triplets[i] = triplets[i];
        }
        delete[] triplets;
        triplets = new_triplets;
        capacity = new_capacity;
    }

    // Validate and parse a triplet line, supporting both formats: (row col value) or {row,col,value}
    bool validateTripletLine(const char* line, int& row, int& col, int& value) {
        int len = strlen(line);
        int r = 0, c = 0, v = 0;
        int state = 0; // 0: before row, 1: in row, 2: in col, 3: in value
        bool negative = false;
        char opening_char = 0;
        char separator = 0;

        for (int i = 0; i < len; i++) {
            if (line[i] == ' ' || line[i] == '\t') continue; // Ignore spaces
            
            if (state == 0) {
                if (line[i] == '(' || line[i] == '{') { 
                    opening_char = line[i];
                    separator = (opening_char == '(') ? ' ' : ',';
                    state = 1; 
                    continue; 
                }
            }
            
            if (state == 1) {
                if (line[i] == '-') { 
                    negative = true; 
                    continue; 
                }
                if (line[i] >= '0' && line[i] <= '9') {
                    r = r * 10 + (line[i] - '0');
                } else if (line[i] == separator) { 
                    if (negative) r = -r;
                    negative = false;
                    state = 2;
                } else {
                    return false; // Incorrect format
                }
            } else if (state == 2) {
                if (line[i] == '-') { 
                    negative = true; 
                    continue; 
                }
                if (line[i] >= '0' && line[i] <= '9') {
                    c = c * 10 + (line[i] - '0');
                } else if (line[i] == separator) { 
                    if (negative) c = -c;
                    negative = false;
                    state = 3;
                } else {
                    return false;
                }
            } else if (state == 3) {
                if (line[i] == '-') { 
                    negative = true; 
                    continue; 
                }
                if (line[i] >= '0' && line[i] <= '9') {
                    v = v * 10 + (line[i] - '0');
                } else if ((opening_char == '(' && line[i] == ')') || 
                           (opening_char == '{' && line[i] == '}')) {
                    if (negative) v = -v;
                    row = r; col = c; value = v;
                    return true;
                } else {
                    return false;
                }
            }
        }
        return false; // Incorrect format
    }

public:
    // Constructor from a file
    SparseMatrix(const char* matrixFilePath) {
        rows = 0;
        cols = 0;
        nnz = 0;
        capacity = 10;
        triplets = new Triplet[capacity];

        ifstream file(matrixFilePath);
        if (!file.is_open()) {
            throw invalid_argument("Cannot open file");
        }

        char line[256];
        int line_count = 0;

        while (file.getline(line, 256)) {
            // Ignore empty lines
            int len = strlen(line);
            bool is_empty = true;
            for (int i = 0; i < len; i++) {
                if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n') {
                    is_empty = false;
                    break;
                }
            }
            if (is_empty) continue;

            if (line_count == 0) {
                // Read rows - support formats "rows=n" or "rows = n"
                if (sscanf(line, "rows=%d", &rows) != 1 && sscanf(line, "rows = %d", &rows) != 1 || rows <= 0) {
                    throw invalid_argument("Input file has wrong format");
                }
            } else if (line_count == 1) {
                // Read cols - support formats "cols=n" or "cols = n"
                if (sscanf(line, "cols=%d", &cols) != 1 && sscanf(line, "cols = %d", &cols) != 1 || cols <= 0) {
                    throw invalid_argument("Input file has wrong format");
                }
            } else {
                // Read triplet
                int row, col, value;
                if (!validateTripletLine(line, row, col, value)) {
                    throw invalid_argument("Input file has wrong format");
                }
                if (row < 0 || row >= rows || col < 0 || col >= cols) {
                    throw invalid_argument("Invalid row or column index");
                }
                if (nnz >= capacity) {
                    resize(capacity * 2);
                }
                triplets[nnz] = {row, col, value};
                nnz++;
            }
            line_count++;
        }
        file.close();
        
        // Sort triplets for better performance
        sort(triplets, triplets + nnz);
    }

    // Constructor for empty matrix
    SparseMatrix(int numRows, int numCols) : rows(numRows), cols(numCols), nnz(0), capacity(10) {
        if (numRows <= 0 || numCols <= 0) {
            throw invalid_argument("Invalid matrix dimensions");
        }
        triplets = new Triplet[capacity];
    }
    
    // Copy constructor
    SparseMatrix(const SparseMatrix& other) : rows(other.rows), cols(other.cols), nnz(other.nnz), capacity(other.capacity) {
        triplets = new Triplet[capacity];
        for (int i = 0; i < nnz; i++) {
            triplets[i] = other.triplets[i];
        }
    }
    
    // Assignment operator
    SparseMatrix& operator=(const SparseMatrix& other) {
        if (this != &other) {
            delete[] triplets;
            rows = other.rows;
            cols = other.cols;
            nnz = other.nnz;
            capacity = other.capacity;
            triplets = new Triplet[capacity];
            for (int i = 0; i < nnz; i++) {
                triplets[i] = other.triplets[i];
            }
        }
        return *this;
    }

    // Destructor
    ~SparseMatrix() {
        delete[] triplets;
    }

    // Get an element
    int getElement(int currRow, int currCol) const {
        if (currRow < 0 || currRow >= rows || currCol < 0 || currCol >= cols) {
            throw invalid_argument("Invalid row or column index");
        }
        
        // Binary search for better efficiency since triplets are sorted
        int low = 0, high = nnz - 1;
        while (low <= high) {
            int mid = (low + high) / 2;
            if (triplets[mid].row < currRow) {
                low = mid + 1;
            } else if (triplets[mid].row > currRow) {
                high = mid - 1;
            } else if (triplets[mid].col < currCol) {
                low = mid + 1;
            } else if (triplets[mid].col > currCol) {
                high = mid - 1;
            } else {
                return triplets[mid].value;
            }
        }
        return 0; // Default value
    }

    // Set an element
    void setElement(int currRow, int currCol, int value) {
        if (currRow < 0 || currRow >= rows || currCol < 0 || currCol >= cols) {
            throw invalid_argument("Invalid row or column index");
        }
        
        // Search if the element exists
        int pos = -1;
        for (int i = 0; i < nnz; i++) {
            if (triplets[i].row == currRow && triplets[i].col == currCol) {
                pos = i;
                break;
            }
        }
        
        if (pos != -1) {
            if (value == 0) {
                // Remove the element
                for (int j = pos; j < nnz - 1; j++) {
                    triplets[j] = triplets[j + 1];
                }
                nnz--;
            } else {
                triplets[pos].value = value;
            }
        } else if (value != 0) {
            // Add a new element if value != 0
            if (nnz >= capacity) {
                resize(capacity * 2);
            }
            triplets[nnz] = {currRow, currCol, value};
            nnz++;
            
            // Maintain sorted order
            sort(triplets, triplets + nnz);
        }
    }

    // Addition of two matrices
    SparseMatrix add(const SparseMatrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw invalid_argument("Matrix dimensions must match for addition");
        }
        SparseMatrix result(rows, cols);
        
        // Traverse both matrices simultaneously (more efficient since they are sorted)
        int i = 0, j = 0;
        while (i < nnz && j < other.nnz) {
            if (triplets[i].row < other.triplets[j].row || 
               (triplets[i].row == other.triplets[j].row && triplets[i].col < other.triplets[j].col)) {
                result.setElement(triplets[i].row, triplets[i].col, triplets[i].value);
                i++;
            } else if (triplets[i].row > other.triplets[j].row || 
                      (triplets[i].row == other.triplets[j].row && triplets[i].col > other.triplets[j].col)) {
                result.setElement(other.triplets[j].row, other.triplets[j].col, other.triplets[j].value);
                j++;
            } else {
                // Same position, add the values
                int sum = triplets[i].value + other.triplets[j].value;
                if (sum != 0) {
                    result.setElement(triplets[i].row, triplets[i].col, sum);
                }
                i++;
                j++;
            }
        }
        
        // Add remaining elements from the first matrix
        while (i < nnz) {
            result.setElement(triplets[i].row, triplets[i].col, triplets[i].value);
            i++;
        }
        
        // Add remaining elements from the second matrix
        while (j < other.nnz) {
            result.setElement(other.triplets[j].row, other.triplets[j].col, other.triplets[j].value);
            j++;
        }
        
        return result;
    }

    // Subtraction of two matrices
    SparseMatrix subtract(const SparseMatrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw invalid_argument("Matrix dimensions must match for subtraction");
        }
        SparseMatrix result(rows, cols);
        
        // Traverse both matrices simultaneously (more efficient since they are sorted)
        int i = 0, j = 0;
        while (i < nnz && j < other.nnz) {
            if (triplets[i].row < other.triplets[j].row || 
               (triplets[i].row == other.triplets[j].row && triplets[i].col < other.triplets[j].col)) {
                result.setElement(triplets[i].row, triplets[i].col, triplets[i].value);
                i++;
            } else if (triplets[i].row > other.triplets[j].row || 
                      (triplets[i].row == other.triplets[j].row && triplets[i].col > other.triplets[j].col)) {
                result.setElement(other.triplets[j].row, other.triplets[j].col, -other.triplets[j].value);
                j++;
            } else {
                // Same position, subtract the values
                int diff = triplets[i].value - other.triplets[j].value;
                if (diff != 0) {
                    result.setElement(triplets[i].row, triplets[i].col, diff);
                }
                i++;
                j++;
            }
        }
        
        // Add remaining elements from the first matrix
        while (i < nnz) {
            result.setElement(triplets[i].row, triplets[i].col, triplets[i].value);
            i++;
        }
        
        // Add remaining elements from second matrix (with negative sign)
        while (j < other.nnz) {
            result.setElement(other.triplets[j].row, other.triplets[j].col, -other.triplets[j].value);
            j++;
        }
        
        return result;
    }

    // Conversion to CSR for multiplication
    CSR toCSR() const {
        CSR csr;
        csr.rows = rows;
        csr.cols = cols;
        csr.nnz = nnz;
        csr.values = new int[nnz];
        csr.col_indices = new int[nnz];
        csr.row_ptr = new int[rows + 1]();

        // Copy values and column indices
        for (int i = 0; i < nnz; i++) {
            csr.values[i] = triplets[i].value;
            csr.col_indices[i] = triplets[i].col;
            csr.row_ptr[triplets[i].row + 1]++;
        }

        // Calculate row pointers
        for (int i = 1; i <= rows; i++) {
            csr.row_ptr[i] += csr.row_ptr[i - 1];
        }

        return csr;
    }

    // Multiplication of two matrices
    SparseMatrix multiply(const SparseMatrix& other) const {
        if (cols != other.rows) {
            throw invalid_argument("Invalid dimensions for matrix multiplication");
        }
        SparseMatrix result(rows, other.cols);
        CSR csr_a = toCSR();
        CSR csr_b = other.toCSR();

        // Optimized multiplication with CSR
        for (int i = 0; i < rows; i++) {
            for (int j = csr_a.row_ptr[i]; j < csr_a.row_ptr[i + 1]; j++) {
                int col_a = csr_a.col_indices[j];
                int val_a = csr_a.values[j];
                for (int k = csr_b.row_ptr[col_a]; k < csr_b.row_ptr[col_a + 1]; k++) {
                    int col_b = csr_b.col_indices[k];
                    int val_b = csr_b.values[k];
                    int curr_val = result.getElement(i, col_b);
                    result.setElement(i, col_b, curr_val + val_a * val_b);
                }
            }
        }

        // Clean up CSR
        delete[] csr_a.values;
        delete[] csr_a.col_indices;
        delete[] csr_a.row_ptr;
        delete[] csr_b.values;
        delete[] csr_b.col_indices;
        delete[] csr_b.row_ptr;

        return result;
    }

    // Save the matrix to a file (format with curly braces)
    void saveToFile(const char* filePath) const {
        ofstream file(filePath);
        file << "rows = " << rows << "\n";
        file << "cols = " << cols << "\n";
        for (int i = 0; i < nnz; i++) {
            file << "{" << triplets[i].row << "," << triplets[i].col << "," << triplets[i].value << "}\n";
        }
        file.close();
    }
};

// Main program
int main() {
    char file1[256], file2[256], output_file[256];
    int operation;

    cout << "Enter path to first matrix file: ";
    cin >> file1;
    cout << "Enter path to second matrix file: ";
    cin >> file2;
    cout << "Select operation (1: Addition, 2: Subtraction, 3: Multiplication): ";
    cin >> operation;
    cout << "Enter output file path: ";
    cin >> output_file;

    try {
        SparseMatrix mat1(file1);
        SparseMatrix mat2(file2);
        SparseMatrix result(1, 1); // Temporary initialization

        switch (operation) {
            case 1:
                result = mat1.add(mat2);
                break;
            case 2:
                result = mat1.subtract(mat2);
                break;
            case 3:
                result = mat1.multiply(mat2);
                break;
            default:
                throw invalid_argument("Invalid operation selected");
        }

        result.saveToFile(output_file);
        cout << "Operation completed. Result saved to " << output_file << endl;
    } catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
