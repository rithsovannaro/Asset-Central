#include "../include/DisplayUtil.hpp"
#include <tabulate/table.hpp>
using namespace std;

// Implementation of the DisplayUtil class
void DisplayUtil::displayWelcome() {
    //  string brown = "\033[38;2;165;42;42m"; // RGB Brown
    // string violet = "\033[95m";            // Bright violet (magenta)
    // string reset = "\033[0m";
         cout << "\033[38;2;165;42;42m+----------------------------------------+\n";
    cout << "\033[38;2;165;42;42m| " << "\033[95m       Stock Management system      "  << "\033[38;2;165;42;42m   |\n";
    cout << "\033[38;2;165;42;42m+----------------------------------------+\n";
    cout << "\033[0m"; // Reset color

}

void DisplayUtil::displayUsers(const std::vector<User>& users) {
    if (users.empty()) {
        std::cout << "No user accounts found." << std::endl;
        return;
    }

    // Create a table with headers
    tabulate::Table users_table;
    users_table.add_row({"Username", "Password", "Is Admin"});

    // Add user data to the table
    for (const auto& user : users) {
        users_table.add_row({
            user.getUsername(),
            user.getPassword(),
            user.isAdmin() ? "True" : "False"
        });
    }

    // Apply some formatting for better aesthetics
    users_table.format().font_style({tabulate::FontStyle::bold});
    users_table[0].format().font_style({tabulate::FontStyle::bold})
                           .background_color(tabulate::Color::cyan);

    // Print the table to the console
    std::cout << "\n--- User Accounts ---" << std::endl;
    std::cout << users_table << std::endl;
}

void DisplayUtil::displayStocks(const std::vector<Stock>& stocks) {

    if (stocks.empty()) {
        std::cout << "No stock items found." << std::endl;
        return;
    }

    // Create a table with headers
    tabulate::Table stocks_table;
    stocks_table.add_row({"ID", "Name", "Quantity", "Price"});

    // Add stock data to the table
    for (const auto& stock : stocks) {
        stocks_table.add_row({
            std::to_string(stock.getId()),
            stock.getName(),
            std::to_string(stock.getQuantity()),
            std::to_string(stock.getPrice())
        });
    }

    // Apply some formatting for better aesthetics
    stocks_table.format().font_style({tabulate::FontStyle::bold});
    stocks_table[0].format().font_style({tabulate::FontStyle::bold})
                            .background_color(tabulate::Color::magenta);

    // Print the table to the console
    std::cout << "                        " << std::endl;
    std::cout << "+--------------------+\n";
    std::cout << "|      All Stocks    |" << std::endl;
    std::cout << "+--------------------+\n";
    std::cout << stocks_table << std::endl;
}
