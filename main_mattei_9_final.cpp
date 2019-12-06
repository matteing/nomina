/*
Nombre: Sergio Mattei
ID: 801183252
Class/Seccion: CCOM 3033 SEC 001
Nombre de archivo: main_mattei_9.cpp
Descripción: This program calculates the salary for employees based on a simple database format, and places results in another file.

Features:
1. Automatic sorting by name
2. Searching for employees (ultra-fast, recursive binary search)
3. Uses vectors for safety
4. Resizable window/file sizes - just change those constants
5. Allows running multiple searches and opening multiple files
6. Standardized display code using overloading
7. A real nifty menu using enumerators
8. Input validation for ALL inputs!

Future upgrade path/to-do:
1. Multiple search results - display code already implemented
2. Terminal width detection - might require cross-platform hackery
3. More querying and sorting options

This was a fun project to make, and it's hard to stop adding little improvements to it! 
*/

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <vector>
#include <cstdlib>

using namespace std;

// Define constants
const int WINDOW_WIDTH = 150;
const int WINDOW_COL_SIZE = WINDOW_WIDTH / 9;
const int FILE_WIDTH = 150;
const int FILE_COL_SIZE = FILE_WIDTH / 9;

// UX: Define a few colors (escape sequences) to make things less awful
const string T_RESET = "\x1B[0m";
const string T_RED = "\x1B[31m";
const string T_GRN = "\x1B[32m";
const string T_YEL = "\x1B[33m";

// Define menu actions
enum menuOptions
{
    MENU_FILTER = 1,
    MENU_SEARCH,
    MENU_AGAIN,
    MENU_EXIT
};

// Declare structs
struct Employee
{
    int id;
    string name;
    string period;
    string ssn;
    double hours;
    double salary;
    double extra;
    double gross;
    double deductions;
    double net;
};

// Declare functions
void askForFilenames(ifstream &, ofstream &);

double calculateGross(double, double, double);
double calculateExtra(double, double);
double calculateDeductions(double);
double calculateNet(double, double);
void calculateAllSalaryData(Employee &);
void calculateTotal(vector<Employee>, double &);

vector<Employee> parseEmployeeFile(ifstream &);

void printTableHeading();
void printTableHeading(ofstream &);

void printTotal(double);
void printTotal(double, ofstream &);

void printRow(Employee e);
void printRow(Employee e, ofstream &);

void printAllRows(vector<Employee> &, ofstream &);

int askEmployeesToFilter(const vector<Employee> &);

void sortEmployeesByName(vector<Employee> &);
int searchEmployees(vector<Employee> &, string = "", int = -1, int = -1);
void filterEmployeesByRange(vector<Employee> &, int);

int displayMenuOptions();

int main()
{
    // note: see askRerun function
    bool again = false;

    cout << T_GRN << "Welcome to Nomina 8.0 (new and actually working!)" << T_RESET << endl;

    do
    {
        // Create a sentinel value for filterRange.
        int filterRange = -1;
        bool searching = false; // note: see askSearch function
        ifstream fileToRead;
        ofstream fileToWrite;

        // Disable again until user is prompted.
        again = false;

        // Ask for the filenames to process
        askForFilenames(fileToRead, fileToWrite);

        // Parse the files provided and create the database vector
        // We declare it as const because it makes it immutable
        const vector<Employee> employees = parseEmployeeFile(fileToRead);

        do
        {
            // Create a variable to keep track of operation failures.
            bool success = true;
            // Create a copy of employees to work on.
            // We can modify the querySet - the employees one should not be mutated.
            vector<Employee> querySet(employees);

            if (filterRange > 0)
            {
                filterEmployeesByRange(querySet, filterRange);
                // Now that we're filtered, prevent the next run.
                filterRange = -1;
            }

            // Set variables for searching mode.
            bool found = false;
            int foundPos = -1;

            // Set running total
            double total = 0.0;

            if (searching)
            {
                foundPos = searchEmployees(querySet);

                if (foundPos < 0)
                {
                    cout << T_RED << "No results found." << T_RESET << endl;
                    success = false;
                }
                else
                {
                    // Reduce queryset to found results.
                    // This allows for adding multiple result handling easily down the road.
                    Employee foundItem;
                    foundItem = querySet[foundPos];
                    querySet.clear();
                    querySet.push_back(foundItem);
                }

                // Now disable for the next run.
                searching = false;
            }

            // If the current operation is successful, display results.
            // Otherwise, we default to the menu.
            if (success)
            {
                // Calculate totals for the current queryset.
                calculateTotal(querySet, total);

                // Print table heading for the terminal
                printTableHeading();

                // Print table heading for the file
                printTableHeading(fileToWrite);

                // Now print all rows
                printAllRows(querySet, fileToWrite);

                // Write total to cout
                printTotal(total);

                // Now send total to file.
                printTotal(total, fileToWrite);
            }

            // Now ask what we do next!
            switch (displayMenuOptions())
            {
            case MENU_FILTER:
                // Ask the user whether they want to compute a specific amount of people.
                // This can be done to the current query set or all employees.
                filterRange = askEmployeesToFilter(employees);
                break;

            case MENU_SEARCH:
                // We doin' a search now.
                searching = true;
                break;

            case MENU_AGAIN:
                again = true;
                break;

            default:
                // let the program die.
                break;
            }
        } while (searching || filterRange > -1);

        // Close all files
        fileToRead.close();
        fileToWrite.close();
    } while (again);

    return 0;
}

void askForFilenames(ifstream &fileToRead, ofstream &fileToWrite)
{
    string fileName;
    // Open stream to file.
    do
    {
        if (fileToRead.fail())
        {
            cout << T_RED + "Wrong filename." + T_RESET << endl;
        }

        cout << "Filename to read: ";
        getline(cin, fileName);
        fileToRead.open(fileName.c_str()); // apparently the VM doesn't bundle C++11.
    } while (!fileToRead);

    cout << "Filename to write results: ";
    getline(cin, fileName);
    fileToWrite.open(fileName.c_str(), ios_base::app);
}

void printTableHeading()
{
    cout << "\n";
    cout << setw((WINDOW_WIDTH / 2) + 7) << "NOMINA DE EMPRESA" << endl;
    cout << "\n";
    cout << left << setw(WINDOW_COL_SIZE) << "PERIODO"
         << setw(WINDOW_COL_SIZE) << "NOMBRE"
         << setw(WINDOW_COL_SIZE) << "SSN"
         << setw(WINDOW_COL_SIZE) << "HORAS"
         << setw(WINDOW_COL_SIZE) << "SALARIO"
         << setw(WINDOW_COL_SIZE) << "BRUTO EXTRA"
         << setw(WINDOW_COL_SIZE) << "TOTAL BRUTO"
         << setw(WINDOW_COL_SIZE) << "DEDUC"
         << setw(WINDOW_COL_SIZE) << "TOTAL NETO" << endl;

    // This took a lot of work to get right.
    // So turns out that setfill has to go before setw, and for it to work a character must be printed.
    cout << setfill('=') << setw(WINDOW_WIDTH) << "" << endl;
    // Reset fill character for next setw's.
    cout << setfill(' ') << fixed << setprecision(2) << showpoint;
}

void printTableHeading(ofstream &file)
{
    file << endl;
    file << setw((FILE_WIDTH / 2) + 7) << "NOMINA DE EMPRESA" << endl;
    file << "\n";
    file << left << setw(FILE_COL_SIZE) << "PERIODO"
         << setw(FILE_COL_SIZE) << "NOMBRE"
         << setw(FILE_COL_SIZE) << "SSN"
         << setw(FILE_COL_SIZE) << "HORAS"
         << setw(FILE_COL_SIZE) << "SALARIO"
         << setw(FILE_COL_SIZE) << "BRUTO EXTRA"
         << setw(FILE_COL_SIZE) << "TOTAL BRUTO"
         << setw(FILE_COL_SIZE) << "DEDUC"
         << setw(FILE_COL_SIZE) << "TOTAL NETO" << endl;
    file << setfill('=') << setw(FILE_WIDTH) << "" << endl;
    file << setfill(' ') << fixed << setprecision(2) << showpoint;
}

void printTableRow(Employee e)
{
    cout << setw(WINDOW_COL_SIZE) << e.period;
    cout << setw(WINDOW_COL_SIZE) << e.name;
    cout << setw(WINDOW_COL_SIZE) << e.ssn;
    cout << setw(WINDOW_COL_SIZE) << e.hours;
    cout << setw(WINDOW_COL_SIZE) << e.salary;
    cout << setw(WINDOW_COL_SIZE) << e.extra;
    cout << setw(WINDOW_COL_SIZE) << e.gross;
    cout << setw(WINDOW_COL_SIZE) << e.deductions;
    cout << setw(WINDOW_COL_SIZE) << e.net;
    cout << endl;
}

void printTableRow(Employee e, ofstream &file)
{
    file << setw(FILE_COL_SIZE) << e.period;
    file << setw(FILE_COL_SIZE) << e.name;
    file << setw(FILE_COL_SIZE) << e.ssn;
    file << setw(FILE_COL_SIZE) << e.hours;
    file << setw(FILE_COL_SIZE) << e.salary;
    file << setw(FILE_COL_SIZE) << e.extra;
    file << setw(FILE_COL_SIZE) << e.gross;
    file << setw(FILE_COL_SIZE) << e.deductions;
    file << setw(FILE_COL_SIZE) << e.net;
    file << endl;
}

void calculateAllSalaryData(Employee &e)
{
    // initialize all vars to prevent random values
    double extra = 0.0;
    double gross = 0.0;
    double deductions = 0.0;
    double net = 0.0;
    extra = calculateExtra(e.salary, e.hours);
    gross = calculateGross(e.salary, e.hours, extra);
    deductions = calculateDeductions(gross);
    net = calculateNet(gross, deductions);

    e.extra = extra;
    e.gross = gross;
    e.deductions = deductions;
    e.net = net;
}

double calculateGross(double salary, double hours, double extra)
{
    return (salary * hours) + extra;
}

double calculateExtra(double salary, double hours)
{
    double extra = 0.0;
    double extraDifference = hours - 40;

    if (extraDifference > 0)
    {
        // double the hourly wage for the difference in hours
        extra = extraDifference * salary;
    }
    else
    {
        extra = 0.0;
    }

    return extra;
}

double calculateDeductions(double gross)
{
    return gross * 0.18;
}

double calculateNet(double gross, double deductions)
{
    return gross - deductions;
}

void printTotal(double total)
{
    cout << "\n";
    cout << right << setw(WINDOW_WIDTH - 15) << T_GRN << "Total pagado: " << T_RESET << total << endl;
}

void printTotal(double total, ofstream &file)
{
    file << "\n";
    file << right << setw(FILE_WIDTH - 15) << "Total pagado: " << total;
}

vector<Employee> parseEmployeeFile(ifstream &file)
{
    vector<Employee> employees;
    int index = 0;
    string period, name, ssn;
    double net = 0.0, gross = 0.0, deductions = 0.0, extra = 0.0, hours = 0.0, salary = 0.0;
    while (file >> period >> name >> ssn >> hours >> salary)
    {
        Employee e = {
            index, name, period, ssn, hours, salary};
        calculateAllSalaryData(e);
        employees.push_back(e);
        index++; // update ID counter
    }

    sortEmployeesByName(employees);

    return employees;
}

void printAllRows(vector<Employee> &employees, ofstream &file)
{
    for (int i = 0; i < employees.size(); i++)
    {
        // Write terminal row
        printTableRow(employees[i]);
        // Write file row
        printTableRow(employees[i], file);

        // end row...
        file << endl;
    }
}

void sortEmployeesByName(vector<Employee> &employees)
{
    for (int start = 0; start < employees.size() - 1; start++)
    {
        Employee min = employees[start];
        int minInd = start;
        for (int i = start; i < employees.size(); i++)
        {
            if (employees[i].name < min.name)
            {
                min = employees[i];
                minInd = i;
            }
        }
        employees[minInd] = employees[start];
        employees[start] = min;
    }
}

int askEmployeesToFilter(const vector<Employee> &querySet)
{
    int newSize = querySet.size();
    bool valid = true;
    do
    {
        if (!valid)
        {
            cout << T_RED << "Invalid size." << T_RESET << endl;
            valid = true;
        }

        // If we use strings for this, it allows hitting enter to continue.
        // Cin does not allow this without a lot of hacks, as it messes with the stdin buffer.
        // This is better user experience.
        string s; // better UX
        cout << "How many employees should we process? [1-" << querySet.size() << "]: ";
        getline(cin, s);
        try
        {
            if (!s.empty())
            {
                newSize = atoi(s.c_str());
            }
            else
            {
                valid = false;
            }
        }
        catch (...)
        {
            valid = false;
        }

        if (newSize < 1)
        {
            valid = false;
        }
    } while (!valid);

    return newSize;
}

int searchEmployees(vector<Employee> &employees, string query, int first, int last)
{
    while (query.empty())
    {
        cout << "Write your query: ";
        getline(cin, query);
    };

    if (last < 0 && first < 0)
    {
        // Replace default parameters
        first = 0;
        last = employees.size() - 1;
    }

    // Calculate middle.
    int middle = (first + last) / 2;

    // Base case.
    if (first > last)
        return -1;

    if (employees[middle].name == query)
    {
        return middle;
    }
    else if (employees[middle].name > query)
    {
        return searchEmployees(employees, query, first, middle - 1);
    }
    else
    {
        return searchEmployees(employees, query, middle + 1, last);
    }
}

void calculateTotal(vector<Employee> employees, double &total)
{
    for (int i = 0; i < employees.size(); i++)
    {
        total += employees[i].net;
    }
}

void filterEmployeesByRange(vector<Employee> &querySet, int filterRange)
{
    if (filterRange > querySet.size())
        return;
    if (filterRange < 0)
        return;
    querySet.resize(filterRange);
}

int displayMenuOptions()
{
    cout << '\n';
    cout << T_GRN << "Select an action to take:" << T_RESET << endl;
    cout << MENU_FILTER << ". Filter employees" << endl;
    cout << MENU_SEARCH << ". Search the set" << endl;
    cout << MENU_AGAIN << ". Process another file" << endl;
    cout << MENU_EXIT << ". Exit" << endl;
    cout << '\n';

    bool valid = true;
    int value = -1;

    do
    {
        if (!valid)
        {
            cout << T_RED << "Invalid selection." << T_RESET << endl;
        }

        // better UX
        string selection;
        cout << T_YEL << "Pick an item: " << T_RESET;
        getline(cin, selection);

        // atoi can fail so we use try catch to get any exceptions.
        // This seems like overkill, but it allows hitting enter to do a default value.
        // see askEmployeesToFilter comments
        try
        {
            if (!selection.empty())
            {
                value = atoi(selection.c_str());
                valid = true;
            }
        }
        catch (...)
        {
            valid = false;
        }
    } while (!valid);

    return value;
}
