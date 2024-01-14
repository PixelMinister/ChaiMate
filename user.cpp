// User.cpp

#include "user.h"

using namespace std;

User::User(const string& n, const string& pass, const string& dept,
           const string& c, const vector<string>& ints, int deg)
    : name(n), password(pass), department(dept), city(c), interests(ints), degree(deg) {}

bool User::registerUser() {
    // Check if the user already exists
    vector<User> users = loadUserData();
    for (const User& user : users) {
        if (user.name == name) {
            cerr << "Error: Username already exists. Please choose a different username." << endl;
            return false;
        }
    }

    // Add new user to the vector
    users.push_back(*this);

    // Save user information to a CSV file
    saveUserData(*this);

    cout << "User registered successfully!" << endl;
    return true;
}

bool User::loginUser() {
    // Load user data from the CSV file
    vector<User> users = loadUserData();

    // Check if the user exists and password matches
    for (User& user : users) {
        if (user.name == name && user.password == password) {
            // Update the current user's properties
            *this = user;

            cout << "User " << name << " logged in successfully!" << endl;
            return true;
        }
    }

    cerr << "Error: Username or password is incorrect. Please try again." << endl;
    return false;
}

bool User::saveUserData(const User& user) {
    ofstream file("users.csv", ios::out | ios::app);
    if (file.is_open()) {
        file << user.name << ',' << user.password << ',' << user.department << ','
             << user.city << ',' << user.interests[0] << ',' << user.interests[1] << ','
             << user.interests[2] << ',' << user.degree << '\n';
        file.close();
        return true;
    } else {
        cerr << "Error: Unable to open users.csv for writing." << endl;
        return false;
    }
}

vector<User> User::loadUserData() {
    vector<User> users;

    ifstream file("users.csv");
    if (file.is_open()) {
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string n, pass, dept, c, i1, i2, i3;
            int deg;

            getline(ss, n, ',');
            getline(ss, pass, ',');
            getline(ss, dept, ',');
            getline(ss, c, ',');
            getline(ss, i1, ',');
            getline(ss, i2, ',');
            getline(ss, i3, ',');
            ss >> deg;

            users.emplace_back(n, pass, dept, c, vector<string>{i1, i2, i3}, deg);
        }
        file.close();
    } else {
        cerr << "Error: Unable to open users.csv for reading." << endl;
    }

    return users;
}

// Getter functions implementation
string User::getName() const {
    return name;
}

string User::getDepartment() const {
    return department;
}

string User::getCity() const {
    return city;
}

vector<string> User::getInterests() const {
    return interests;
}

int User::getDegree() const {
    return degree;
}
string User::getUserDetails() const {
  return "Name: " + name + "\nDepartment: " + department + "\nCity: " + city +
         "\nInterests: " + interests[0] + ", " + interests[1] + ", " + interests[2] +
         "\nDegree: " + std::to_string(degree);
}
