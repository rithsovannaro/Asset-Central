#ifndef USER_HPP
#define USER_HPP

#include <string>

// A class to represent a user in the system
class User {
public:
    // Default constructor
    User();

    // Parameterized constructor
    User(const std::string& username, const std::string& password, bool isAdmin);

    // Getters for user properties
    std::string getUsername() const;
    std::string getPassword() const;
    bool isAdmin() const;

    // Setters for user properties
    void setUsername(const std::string& username);
    void setPassword(const std::string& password);
    void setIsAdmin(bool isAdmin);

private:
    std::string m_username;
    std::string m_password;
    bool m_isAdmin;
};

#endif // USER_HPP
