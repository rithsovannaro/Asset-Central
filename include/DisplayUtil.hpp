#ifndef DISPLAY_UTIL_HPP
#define DISPLAY_UTIL_HPP

#include <iostream>
#include <vector>
#include <string>
#include <tabulate/table.hpp>
#include "User.hpp"
#include "Stock.hpp"

// A utility class to handle displaying data in a clean, tabular format
class DisplayUtil {
public:
    // Display a welcome message
    // static void displayWelcome();
    
    // Display a list of users in a table
    static void displayUsers(const std::vector<User>& users);

    // Display a list of stocks in a table
    static void displayStocks(const std::vector<Stock>& stocks);

private:
    // Helper function to create and print a table
    static void printTable(const tabulate::Table& table);
};

#endif // DISPLAY_UTIL_HPP
