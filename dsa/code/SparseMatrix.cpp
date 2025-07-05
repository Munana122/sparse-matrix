#include <iostream>
#include <fstream>
#include <cstring>
#include <stdexcept>

using namespace std;

struct Triplet {
    int row;
    int col;
    int value;
};

struct CSR {
    int* values;
    int* col_indices;
    int* row_ptr;
    int nnz;
    int rows;
    int cols;
};

class SparseMatrix {
private:
    int rows, cols;
    Triplet* triplets;
    int nnz;
    int capacity;

    void resize(int new_capacity) {
        Triplet* new_triplets = new Triplet[new_capacity];
        for (int i = 0; i < nnz; i++) {
            new_triplets[i] = triplets[i];
        }
        delete[] triplets;
        triplets = new_triplets;
        capacity = new_capacity;
    }

    bool validateTripletLine(const char* line, int& row, int& col, int& value) {
        int len = strlen(line);
        int r = 0, c = 0, v = 0;
        int state = 0;
        bool negative = false;

        for (int i = 0; i < len; i++) {
            if (line[i] == ' ' || line[i] == '\t') continue;
            if (state == 0 && (line[i] == '(' || line[i] == '{')) { state = 1; continue; }
            if (state == 1) {
                if (line[i] == '-') { negative = true; continue; }
                if (line[i] >= '0' && line[i] <= '9') {
                    r = r * 10 + (line[i] - '0');
                } else if (line[i] == ',') {
                    if (negative) r = -r;
                    negative = false;
                    state = 2;
                } else {
                    return false;
                }
            } else if (state == 2) {
                if (line[i] == '-') { negative = true; continue; }
                if (line[i] >= '0' && line[i] <= '9') {
                    c = c * 10 + (line[i] - '0');
                } else if (line[i] == ',') {
                    if (negative) c = -c;
                    negative = false;
                    state = 3;
                } else {
                    return false;
                }
            } else if (state == 3) {
                if (line[i] == '-') { negative = true; continue; }
                if (line[i] >= '0' && line[i] <= '9') {
                    v = v * 10 + (line[i] - '0');
                } else if (line[i] == '.' || line[i] == 'e' || line[i] == 'E') {
                    return false;
                } else if (line[i] == ')' || line[i] == '}') {
                    if (negative) v = -v;
                    row = r; col = c; value = v;
                    return true;
                } else {
                    return false;
                }
            }
        }
        return false;
    }

public:
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
                if (sscanf(line, "rows = %d", &rows) != 1 || rows <= 0) {
                    throw invalid_argument("Input file has wrong format");
                }
            } else if (line_count == 1) {
                if (sscanf(line, "cols = %d", &cols) != 1 || cols <= 0) {
                    throw invalid_argument("Input file has wrong format");
                }
            } else {
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
    }

    SparseMatrix(int numRows, int numCols) : rows(numRows), cols(numCols), nnz(0), capacity(10) {
        if (numRows <= 0 || numCols <= 0) {
            throw invalid_argument("Invalid matrix dimensions");
        }
        triplets = new Triplet[capacity];
    }

    ~SparseMatrix() {
        delete[] triplets;
    }

    int getElement(int currRow, int currCol) const {
        if (currRow < 0 || currRow >= rows || currCol < 0 || currCol >= cols) {
            throw invalid_argument("Invalid row or column index");
        }
        for (int i = 0; i < nnz; i++) {
            if (triplets[i].row == currRow && triplets[i].col == currCol) {
                return triplets[i].value;
            }
        }
        return 0;
    }

    void setElement(int currRow, int currCol, int value) {
        if (currRow < 0 || currRow >= rows || currCol < 0 || currCol >= cols) {
            throw invalid_argument("Invalid row or column index");
        }
        for (int i = 0; i < nnz; i++) {
            if (triplets[i].row == currRow && triplets[i].col == currCol) {
                if (value == 0) {
                    for (int j = i; j < nnz - 1; j++) {
                        triplets[j] = triplets[j + 1];
                    }
                    nnz--;
                } else {
                    triplets[i].value = value;
                }
                return;
            }
        }
        if (value != 0) {
            if (nnz >= capacity) {
                resize(capacity * 2);
            }
            triplets[nnz] = {currRow, currCol, value};
            nnz++;
        }
    }

    SparseMatrix add(const SparseMatrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw invalid_argument("Matrix dimensions must match for addition");
        }
        SparseMatrix result(rows, cols);
        for (int i = 0; i < nnz; i++) {
            result.setElement(triplets[i].row, triplets[i].col, triplets[i].value);
        }
        for (int i = 0; i < other.nnz; i++) {
            int curr_val = result.getElement(other.triplets[i].row, other.triplets[i].col);
            result.setElement(other.triplets[i].row, other.triplets[i].col, curr_val + other.triplets[i].value);
        }
        return result;
    }

    SparseMatrix subtract(const SparseMatrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw invalid_argument("Matrix dimensions must match for subtraction");
        }
        SparseMatrix result(rows, cols);
        for (int i = 0; i < nnz; i++) {
            result.setElement(triplets[i].row, triplets[i].col, triplets[i].value);
        }
        for (int i = 0; i < other.nnz; i++) {
            int curr_val = result.getElement(other.triplets[i].row, other.triplets[i].col);
            result.setElement(other.triplets[i].row, other.triplets[i].col, curr_val - other.triplets[i].value);
        }
        return result;
    }

    CSR toCSR() const {
        CSR csr;
        csr.rows = rows;
        csr.cols = cols;
        csr.nnz = nnz;
        csr.values = new int[nnz];
        csr.col_indices = new int[nnz];
        csr.row_ptr = new int[rows + 1]();

        for (int i = 0; i < nnz; i++) {
            csr.values[i] = triplets[i].value;
            csr.col_indices[i] = triplets[i].col;
            csr.row_ptr[triplets[i].row + 1]++;
        }

        for (int i = 1; i <= rows; i++) {
            csr.row_ptr[i] += csr.row_ptr[i - 1];
        }

        return csr;
    }

    SparseMatrix multiply(const SparseMatrix& other) const {
        if (cols != other.rows) {
            throw invalid_argument("Invalid dimensions for matrix multiplication");
        }
        SparseMatrix result(rows, other.cols);
        CSR csr_a = toCSR();
        CSR csr_b = other.toCSR();

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

        delete[] csr_a.values;
        delete[] csr_a.col_indices;
        delete[] csr_a.row_ptr;
        delete[] csr_b.values;
        delete[] csr_b.col_indices;
        delete[] csr_b.row_ptr;

        return result;
    }

    void display() const {
        cout << "rows = " << rows << "\n";
        cout << "cols = " << cols << "\n";
        for (int i = 0; i < nnz; i++) {
            cout << "(" << triplets[i].row << "," << triplets[i].col << "," << triplets[i].value << ")" << endl;
        }
    }

    void saveToFile(const char* output_file) const {
        ofstream file(output_file);
        if (!file.is_open()) {
            throw invalid_argument("Cannot open output file");
        }
        file << "rows = " << rows << "\n";
        file << "cols = " << cols << "\n";
        for (int i = 0; i < nnz; i++) {
            file << "(" << triplets[i].row << "," << triplets[i].col << "," << triplets[i].value << ")" << "\n";
        }
        file.close();
    }
};

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
        SparseMatrix result(1, 1);

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
        result.display();
        cout << "Operation completed. Result saved to " << output_file << endl;
    } catch (const invalid_argument& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
