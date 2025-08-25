#include "../include/DisplayUtil.hpp"
#include <tabulate/table.hpp>
using namespace std;
using namespace tabulate;
void DisplayUtil::displayStocks(const std::vector<Stock>& stocks) { 
    // Dynamic centering setup
    const int tableWidth = 82;     // inside width of box
    const int terminalWidth = 164; // adjust for your terminal width
    int leftPadding = (terminalWidth - (tableWidth + 4)) / 2; // +4 for borders

    auto padLeft = [&](const string& text) {
        return string(leftPadding, ' ') + text;
    };
    
    auto centerText = [&](const string& text) {
        int spacesLeft = (tableWidth - text.size()) / 2;
        int spacesRight = tableWidth - text.size() - spacesLeft;
        return string(spacesLeft, ' ') + text + string(spacesRight, ' ');
    };

    if (stocks.empty()) {
        // Enhanced "no stocks" message
        cout << "\033[91m\033[1m";
        cout << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("📋 PRODUCT CATALOG - EMPTY 📋") << "\033[91m" << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[96m" << centerText("❌ No stock items found in the inventory") << "\033[91m" << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[95m" << centerText("Please contact admin to add products") << "\033[91m" << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
        return;
    }

    // Enhanced header with stock count
    cout << "\033[96m\033[1m";
    cout << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[93m" << centerText("📋 PRODUCT CATALOG - INVENTORY 📋") << "\033[96m" << "    ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("Available Products: " + to_string(stocks.size()) + " items") << "\033[96m" << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << "\033[0m";

    // Enhanced table header with colors
    cout << "\033[96m\033[1m";
    cout << padLeft("║") << "\033[94m" << "  ID  " << "\033[96m" << "│" << "\033[94m" << "      Product Name      " << "\033[96m" << "│" 
         << "\033[94m" << " Quantity " << "\033[96m" << " │" << "\033[94m" << "   Price ($)   " << "\033[96m" << "                       ║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << "\033[0m";

    // Display each stock item with enhanced formatting
    for (const auto& stock : stocks) {
        cout << "\033[96m\033[1m";
        
        // Format ID (center in 5 chars)
        string id_str = to_string(stock.getId());
        int id_padding = (5 - id_str.length()) / 2;
        string formatted_id = string(id_padding, ' ') + id_str + string(5 - id_str.length() - id_padding, ' ');
        
        // Format Name (truncate/pad to 22 chars)
        string name = stock.getName();
        if (name.length() > 22) {
            name = name.substr(0, 19) + "...";
        }
        string formatted_name = name + string(22 - name.length(), ' ');
        
        // Format Quantity (center in 9 chars)
        string qty_str = to_string(stock.getQuantity());
        int qty_padding = (9 - qty_str.length()) / 2;
        string formatted_qty = string(qty_padding, ' ') + qty_str + string(9 - qty_str.length() - qty_padding, ' ');
        
        // Format Price (center in 13 chars)
        string price_str = to_string(stock.getPrice());
        int price_padding = (13 - price_str.length()) / 2;
        string formatted_price = string(price_padding, ' ') + price_str + string(13 - price_str.length() - price_padding, ' ');
        
        // Color coding based on quantity
        string qty_color = "\033[92m"; // Green for good stock
        if (stock.getQuantity() < 10) qty_color = "\033[93m"; // Yellow for low stock
        if (stock.getQuantity() < 5) qty_color = "\033[91m";  // Red for very low stock
        
        cout << padLeft("║") << "\033[95m" << formatted_id << "\033[96m" << " │" 
             << "\033[94m" << formatted_name << "\033[96m" << "  │" 
             << qty_color << formatted_qty << "\033[96m" << "  │" 
             << "\033[92m" << formatted_price << "\033[96m" << "                         ║\n";
    }

    // Table footer with summary
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
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

