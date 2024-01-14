
#ifndef USER_H
#define USER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

class User {
public:
    User(const std::string& n, const std::string& pass, const std::string& dept,
         const std::string& c, const std::vector<std::string>& ints, int deg);

    bool registerUser();
    bool loginUser();

    // Getter functions
    std::string getName() const;
    std::string getDepartment() const;
    std::string getCity() const;
    std::vector<std::string> getInterests() const;
    int getDegree() const;

    std::string getUserDetails() const;


private:
    std::string name;
    std::string password;
    std::string department;
    std::string city;
    std::vector<std::string> interests;
    int degree;

    static bool saveUserData(const User& user);
    static std::vector<User> loadUserData();
};

#endif
