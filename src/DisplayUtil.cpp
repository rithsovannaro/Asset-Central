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
    // Convert table to string
    std::stringstream ss;
    ss << stocks_table;
    std::string table_str = ss.str();

    // Find the widest line
    size_t max_width = 1000;
    std::stringstream ss_lines(table_str);
    std::string line;
    while (std::getline(ss_lines, line)) {
        if (line.length() > max_width) max_width = line.length();
    }

    // Assume console width (you can adjust this based on your terminal)
    const int console_width = 1000;
    int padding = (console_width - static_cast<int>(max_width)) / 2;
    if (padding < 0) padding = 0;

    // Print title centered
    std::string title = "All Stocks";
    int title_padding = (console_width - static_cast<int>(title.size())) / 2;
    std::cout << std::string(title_padding, ' ') << "+--------------------+\n";
    std::cout << std::string(title_padding, ' ') << "|      All Stocks    |\n";
    std::cout << std::string(title_padding, ' ') << "+--------------------+\n";

    // Print table with padding
    std::stringstream ss_final(table_str);
    while (std::getline(ss_final, line)) {
        std::cout << std::string(padding, ' ') << line << "\n";
    }

}
