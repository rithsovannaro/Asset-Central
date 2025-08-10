#include "../include/User.hpp"

// Default constructor implementation
User::User() : m_username(""), m_password(""), m_isAdmin(false) {}

// Parameterized constructor implementation
User::User(const std::string& username, const std::string& password, bool isAdmin)
    : m_username(username), m_password(password), m_isAdmin(isAdmin) {}

// Getter implementations
std::string User::getUsername() const {
    return m_username;
}

std::string User::getPassword() const {
    return m_password;
}

bool User::isAdmin() const {
    return m_isAdmin;
}

// Setter implementations
void User::setUsername(const std::string& username) {
    m_username = username;
}

void User::setPassword(const std::string& password) {
    m_password = password;
}

void User::setIsAdmin(bool isAdmin) {
    m_isAdmin = isAdmin;
}
