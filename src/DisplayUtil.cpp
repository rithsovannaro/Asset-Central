#include "../include/DisplayUtil.hpp"
#include <tabulate/table.hpp>
using namespace std;
void DisplayUtil::displayStocks(const std::vector<Stock>& stocks) {

    if (stocks.empty()) {
        std::cout << "No stock items found." << std::endl;
        return;
    }

    // Create table with headers (no tabs)
    tabulate::Table stocks_table;
    stocks_table.add_row({"ID", "Name", "Quantity", "Price"});

    // Add stock data
    for (const auto& stock : stocks) {
        stocks_table.add_row({
            std::to_string(stock.getId()),
            stock.getName(),
            std::to_string(stock.getQuantity()),
            std::to_string(stock.getPrice())
        });
    }

    // Format table
    stocks_table.format().font_style({tabulate::FontStyle::bold});
    stocks_table[0].format()
        .font_style({tabulate::FontStyle::bold})
        .background_color(tabulate::Color::magenta);

    // Convert table to string
    std::stringstream ss;
    ss << stocks_table;
    std::string table_str = ss.str();

    // Find widest line in table
    size_t max_width = 0;
    {
        std::stringstream ss_lines(table_str);
        std::string line;
        while (std::getline(ss_lines, line)) {
            if (line.length() > max_width)
                max_width = line.length();
        }
    }

    // Terminal width (adjust based on your console size)
    const int console_width = 170;

    // Calculate padding to center table
    int padding = (console_width - static_cast<int>(max_width)) / 2;
    if (padding < 0) padding = 0;

    // Create title box matching table width
    std::string title = "All Stocks";
    int box_width = static_cast<int>(max_width);
    std::string horizontal_line = "+" + std::string(box_width - 2, '-') + "+";

    // Center the title box
    int box_padding = (console_width - box_width) / 2;
    if (box_padding < 0) box_padding = 0;

    std::cout << std::string(box_padding, ' ') << horizontal_line << "\n";
    int title_space = (box_width - 2 - static_cast<int>(title.size())) / 2;
    std::cout << std::string(box_padding, ' ') 
              << "|" << std::string(title_space, ' ')
              << title
              << std::string(box_width - 2 - title_space - static_cast<int>(title.size()), ' ')
              << "|\n";
    std::cout << std::string(box_padding, ' ') << horizontal_line << "\n";

    // Print centered table (including header row)
    std::stringstream ss_final(table_str);
    std::string line;
    while (std::getline(ss_final, line)) {
        std::cout << std::string(padding, ' ') << line << "\n";
    }
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
    std::cout << "                                                           ------------- User Accounts -------------" << std::endl;
    std::cout << users_table << std::endl;
}


    // // Apply some formatting for better aesthetics
    // stocks_table.format().font_style({tabulate::FontStyle::bold});
    // stocks_table[0].format().font_style({tabulate::FontStyle::bold})
    //                         .background_color(tabulate::Color::magenta);
    // // Convert table to string
    // std::stringstream ss;
    // ss << stocks_table;
    // std::string table_str = ss.str();

    // // Find the widest line
    // size_t max_width = 1000;
    // std::stringstream ss_lines(table_str);
    // std::string line;
    // while (std::getline(ss_lines, line)) {
    //     if (line.length() > max_width) max_width = line.length();
    // }

    // // Assume console width (you can adjust this based on your terminal)
    // const int console_width = 200;
    // int padding = (console_width - static_cast<int>(max_width)) / 2;
    // if (padding < 0) padding = 0;

//     // Print title centered
//     std::string title = "All Stocks";
//     int title_padding = (console_width - static_cast<int>(title.size())) / 2;
//     std::cout << std::string(title_padding, ' ') << "+--------------------+\n";
//     std::cout << std::string(title_padding, ' ') << "|      All Stocks    |\n";
//     std::cout << std::string(title_padding, ' ') << "+--------------------+\n";

//     // Print table with padding
//     std::stringstream ss_final(table_str);
//     while (std::getline(ss_final, line)) {
//         std::cout << std::string(padding, ' ') << line << "\n";
//     }

//     // Print the table to the console

//     // std::cout << "+--------------------+\n";
//     // std::cout << "|      All Stocks    |" << std::endl;
//     // std::cout << "+--------------------+\n";
//     // std::cout << stocks_table << endl;
// }

