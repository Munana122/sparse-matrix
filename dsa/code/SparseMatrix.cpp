#include <bits/stdc++.h>
#include <filesystem>
using namespace std;
namespace fs = std::filesystem;

class SparseMatrix {
public:
    int rows, cols;
    map<pair<int,int>, int> elements;

    SparseMatrix() : rows(0), cols(0) {}

    void readFromFile(const string &filename) {
        ifstream infile(filename);
        if (!infile.is_open()) {
            cerr << "Error opening file: " << filename << endl;
            return;
        }

        string line;

        // Read rows=...
        if (getline(infile, line)) {
            size_t pos = line.find('=');
            if (pos != string::npos) {
                rows = stoi(line.substr(pos + 1));
            }
        }

        // Read cols=...
        if (getline(infile, line)) {
            size_t pos = line.find('=');
            if (pos != string::npos) {
                cols = stoi(line.substr(pos + 1));
            }
        }

        // Read elements: (row, col, value)
        int row, col, value;
        while (getline(infile, line)) {
            if (line.empty()) continue;
            if (sscanf(line.c_str(), "(%d, %d, %d)", &row, &col, &value) == 3) {
                if (value != 0) {
                    elements[{row, col}] = value;
                }
            }
        }
    }

    void writeToFile(const string &filename) const {
        ofstream outfile(filename);
        if (!outfile.is_open()) {
            cerr << "Error writing file: " << filename << endl;
            return;
        }

        // Write header
        outfile << "rows=" << rows << "\n";
        outfile << "cols=" << cols << "\n";

        // Write non-zero entries
        for (auto &entry : elements) {
            int r = entry.first.first;
            int c = entry.first.second;
            int v = entry.second;
            outfile << "(" << r << ", " << c << ", " << v << ")\n";
        }
    }

    SparseMatrix add(const SparseMatrix &other) const {
        SparseMatrix result;
        result.rows = rows;
        result.cols = cols;

        result.elements = elements;
        for (auto &entry : other.elements) {
            result.elements[entry.first] += entry.second;
            if (result.elements[entry.first] == 0) {
                result.elements.erase(entry.first);
            }
        }
        return result;
    }

    SparseMatrix subtract(const SparseMatrix &other) const {
        SparseMatrix result;
        result.rows = rows;
        result.cols = cols;

        result.elements = elements;
        for (auto &entry : other.elements) {
            result.elements[entry.first] -= entry.second;
            if (result.elements[entry.first] == 0) {
                result.elements.erase(entry.first);
            }
        }
        return result;
    }

    SparseMatrix multiply(const SparseMatrix &other) const {
        SparseMatrix result;
        if (cols != other.rows) {
            cerr << "Matrix dimensions do not allow multiplication" << endl;
            return result;
        }
        result.rows = rows;
        result.cols = other.cols;

        for (auto &a : elements) {
            int aRow = a.first.first;
            int aCol = a.first.second;
            int aVal = a.second;

            for (int k = 0; k < other.cols; k++) {
                auto it = other.elements.find({aCol, k});
                if (it != other.elements.end()) {
                    result.elements[{aRow, k}] += aVal * it->second;
                }
            }
        }
        return result;
    }
};

vector<string> listSampleFiles(const string &folder) {
    vector<string> files;
    for (const auto& entry : fs::directory_iterator(folder)) {
        files.push_back(entry.path().string());
    }
    return files;
}

int main() {
    string folder = "../sample_inputs";

    vector<string> files = listSampleFiles(folder);
    if (files.empty()) {
        cerr << "No sample files found in " << folder << endl;
        return 1;
    }

    cout << "Available files for Matrix A:\n";
    for (size_t i = 0; i < files.size(); i++) {
        cout << i+1 << ". " << fs::path(files[i]).filename().string() << endl;
    }
    cout << "Enter the number for Matrix A: ";
    int choiceA; cin >> choiceA;

    cout << "\nAvailable files for Matrix B:\n";
    for (size_t i = 0; i < files.size(); i++) {
        cout << i+1 << ". " << fs::path(files[i]).filename().string() << endl;
    }
    cout << "Enter the number for Matrix B: ";
    int choiceB; cin >> choiceB;

    SparseMatrix A, B;
    A.readFromFile(files[choiceA-1]);
    B.readFromFile(files[choiceB-1]);

    cout << "Select operation (1: Addition, 2: Subtraction, 3: Multiplication): ";
    int op; cin >> op;

    SparseMatrix result;
    if (op == 1) result = A.add(B);
    else if (op == 2) result = A.subtract(B);
    else if (op == 3) result = A.multiply(B);
    else {
        cerr << "Invalid operation" << endl;
        return 1;
    }

    cout << "Enter output file path: ";
    string outFile; cin >> outFile;
    result.writeToFile(outFile);

    cout << "Result written to " << outFile << endl;
    return 0;
}
