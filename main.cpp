/*
 * Sensor Calibration Tool
 *
 * PURPOSE:
 * This program calibrates sensors by mapping raw readings to real-world values
 * using a linear model: Real Value = Slope × Raw Reading + Offset
 * COMPILATION:
 * Windows:   g++ sensor_calibrate.cpp -o sensor_calibrate.exe
 * RUN:
 * Windows:   sensor_calibrate.exe
 */

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <limits>
#include <iomanip>

using namespace std;

// Structure to hold calibration coefficients
struct Calibration {
    double slope;
    double offset;
    bool is_valid;  // Track whether calibration has been set

    Calibration() : slope(0.0), offset(0.0), is_valid(false) {}
};

// Global calibration object
Calibration current_calibration;

// Function prototypes
void display_menu();
void enter_calibration_data();
void load_calibration_from_file();
void convert_raw_reading();
void save_calibration_to_file();
void clear_input_buffer();
void pause_screen();

int main() {
    int choice;

    cout << "\n========================================\n";
    cout << "    SENSOR CALIBRATION TOOL\n";
    cout << "========================================\n\n";

    while (true) {
        display_menu();

        // Get user choice with validation
        if (!(cin >> choice)) {
            cout << "\nInvalid input. Enter a number between 1 and 5.\n";
            clear_input_buffer();
            pause_screen();
            continue;
        }

        clear_input_buffer();  // Clear the newline left by >>

        switch (choice) {
            case 1:
                enter_calibration_data();
                break;
            case 2:
                load_calibration_from_file();
                break;
            case 3:
                convert_raw_reading();
                break;
            case 4:
                save_calibration_to_file();
                break;
            case 5:
                cout << "\nExiting program. Goodbye!\n";
                return 0;
            default:
                cout << "\nInvalid option. Choose between 1 and 5.\n";
                pause_screen();
        }
    }

    return 0;
}

/*
 * Display the main menu options
 */
void display_menu() {
    cout << "\n--- MAIN MENU ---\n";
    cout << "1. Enter new calibration data\n";
    cout << "2. Load existing calibration from file\n";
    cout << "3. Convert a raw reading\n";
    cout << "4. Save current calibration to file\n";
    cout << "5. Exit\n";
    cout << "\nChoose an option: ";
}

/*
 * Collect calibration data points from user and compute slope/offset
 * using least squares linear regression
 */
void enter_calibration_data() {
    int num_points;

    cout << "\n=== ENTER CALIBRATION DATA ===\n";

    // Get number of data points
    while (true) {
        cout << "Enter number of data points (minimum 2): ";
        if (cin >> num_points && num_points >= 2) {
            break;
        } else {
            cout << "Invalid input. Enter an integer >= 2.\n";
            clear_input_buffer();
        }
    }

    clear_input_buffer();

    // Store data points
    vector<double> reference_values(num_points);
    vector<double> raw_readings(num_points);

    // Collect each data point
    for (int i = 0; i < num_points; i++) {
        cout << "\nPoint " << (i + 1) << ":\n";

        // Get reference value (real-world measurement)
        while (true) {
            cout << "  Reference value: ";
            if (cin >> reference_values[i]) {
                break;
            } else {
                cout << "  Invalid input. Enter a number.\n";
                clear_input_buffer();
            }
        }

        // Get raw sensor reading
        while (true) {
            cout << "  Raw reading: ";
            if (cin >> raw_readings[i]) {
                break;
            } else {
                cout << "  Invalid input. Enter a number.\n";
                clear_input_buffer();
            }
        }

        clear_input_buffer();
    }

    /*
     * LINEAR REGRESSION CALCULATION (Least Squares Method)
     *
     * We want to fit: y = slope * x + offset
     * where x = raw reading, y = reference value
     *
     * Formulas:
     * slope = (n * sum_xy - sum_x * sum_y) / (n * sum_x² - (sum_x)²)
     * offset = (sum_y - slope * sum_x) / n
     *
     * This minimizes the sum of squared errors between actual and predicted values
     */

    double sum_x = 0.0;      // Sum of raw readings
    double sum_y = 0.0;      // Sum of reference values
    double sum_xy = 0.0;     // Sum of (raw × reference)
    double sum_x2 = 0.0;     // Sum of (raw²)

    for (int i = 0; i < num_points; i++) {
        double x = raw_readings[i];
        double y = reference_values[i];

        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_x2 += x * x;
    }

    double n = static_cast<double>(num_points);

    // Calculate slope
    double numerator = n * sum_xy - sum_x * sum_y;
    double denominator = n * sum_x2 - sum_x * sum_x;

    // Check for division by zero (all raw readings identical)
    if (fabs(denominator) < 1e-10) {
        cout << "\nError: All raw readings are identical. Cannot compute calibration.\n";
        pause_screen();
        return;
    }

    double slope = numerator / denominator;
    double offset = (sum_y - slope * sum_x) / n;

    // Update current calibration
    current_calibration.slope = slope;
    current_calibration.offset = offset;
    current_calibration.is_valid = true;

    // Display results
    cout << fixed << setprecision(4);
    cout << "\n--- CALIBRATION RESULTS ---\n";
    cout << "Slope:  " << slope << "\n";
    cout << "Offset: " << offset << "\n";
    cout << "\nCalibration updated successfully.\n";
    cout << "Formula: Real Value = " << slope << " × Raw Reading + " << offset << "\n";

    pause_screen();
}

/*
 * Load calibration coefficients from a text file
 * Expected format: first line = slope, second line = offset
 */
void load_calibration_from_file() {
    string filename;

    cout << "\n=== LOAD CALIBRATION ===\n";
    cout << "Enter filename (e.g., calibration.txt): ";
    getline(cin, filename);

    ifstream file(filename);

    if (!file.is_open()) {
        cout << "\nError: Cannot open file '" << filename << "'\n";
        cout << "Make sure the file exists in the current directory.\n";
        pause_screen();
        return;
    }

    double slope, offset;

    // Read slope from first line
    if (!(file >> slope)) {
        cout << "\nError: Cannot read slope from file.\n";
        file.close();
        pause_screen();
        return;
    }

    // Read offset from second line
    if (!(file >> offset)) {
        cout << "\nError: Cannot read offset from file.\n";
        file.close();
        pause_screen();
        return;
    }

    file.close();

    // Update current calibration
    current_calibration.slope = slope;
    current_calibration.offset = offset;
    current_calibration.is_valid = true;

    cout << fixed << setprecision(4);
    cout << "\n--- LOADED CALIBRATION ---\n";
    cout << "Slope:  " << slope << "\n";
    cout << "Offset: " << offset << "\n";
    cout << "\nCalibration loaded successfully from '" << filename << "'\n";

    pause_screen();
}

/*
 * Convert raw sensor readings to real-world values using current calibration
 * Allows multiple conversions in sequence
 */
void convert_raw_reading() {
    cout << "\n=== CONVERT RAW READING ===\n";

    // Check if calibration is available
    if (!current_calibration.is_valid) {
        cout << "\nNo calibration loaded.\n";
        cout << "Please enter calibration data (option 1) or load from file (option 2) first.\n";
        pause_screen();
        return;
    }

    cout << fixed << setprecision(4);
    cout << "Current calibration: Slope = " << current_calibration.slope
         << ", Offset = " << current_calibration.offset << "\n\n";

    char continue_choice;

    do {
        double raw_reading;

        // Get raw reading from user
        while (true) {
            cout << "Enter raw sensor reading: ";
            if (cin >> raw_reading) {
                break;
            } else {
                cout << "Invalid input. Enter a number.\n";
                clear_input_buffer();
            }
        }

        clear_input_buffer();

        // Apply calibration formula: Real Value = Slope × Raw + Offset
        double real_value = current_calibration.slope * raw_reading + current_calibration.offset;

        cout << "\nRaw Reading: " << raw_reading << "\n";
        cout << "Real Value:  " << real_value << "\n\n";

        // Ask if user wants to convert another reading
        cout << "Convert another reading? (y/n): ";
        cin >> continue_choice;
        clear_input_buffer();
        cout << "\n";

    } while (continue_choice == 'y' || continue_choice == 'Y');
}

/*
 * Save current calibration coefficients to a text file
 * Format: slope on first line, offset on second line
 */
void save_calibration_to_file() {
    cout << "\n=== SAVE CALIBRATION ===\n";

    // Check if calibration exists
    if (!current_calibration.is_valid) {
        cout << "\nNo calibration to save.\n";
        cout << "Please enter calibration data (option 1) or load from file (option 2) first.\n";
        pause_screen();
        return;
    }

    string filename;
    cout << "Enter filename to save (e.g., calibration.txt): ";
    getline(cin, filename);

    ofstream file(filename);

    if (!file.is_open()) {
        cout << "\nError: Cannot create file '" << filename << "'\n";
        pause_screen();
        return;
    }

    // Write slope and offset to file (one per line)
    file << fixed << setprecision(10);
    file << current_calibration.slope << "\n";
    file << current_calibration.offset << "\n";

    file.close();

    cout << "\nCalibration saved successfully to '" << filename << "'\n";
    cout << "Slope:  " << current_calibration.slope << "\n";
    cout << "Offset: " << current_calibration.offset << "\n";

    pause_screen();
}

/*
 * Clear the input buffer after invalid input or after using >>
 * Prevents leftover characters from causing problems
 */
void clear_input_buffer() {
    cin.clear();  // Clear error flags
    cin.ignore(numeric_limits<streamsize>::max(), '\n');  // Discard buffer contents
}

/*
 * Pause the screen until user presses Enter
 * Gives user time to read output before menu refreshes
 */
void pause_screen() {
    cout << "\nPress Enter to continue...";
    cin.get();
}
