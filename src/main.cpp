#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <limits>
#include <algorithm> // for sort, transform
#include <iomanip>  // for setprecision 
#include <stdexcept> // for exception handling
#include <numeric> // for accumulate
#include <ctime> // for time_t
#include "../include/User.hpp"
#include "../include/Stock.hpp"
#include "../include/ExcelUtil.hpp"
#include "../include/DisplayUtil.hpp"
#include "../include/Receipt.hpp"

using namespace std;
const int LOW_STOCK_THRESHOLD = 20; 

// ─── Function Prototypes ────────────────────────────────────────
void displayMainMenu();
void adminLogin();
void userRegister();
void userLogin();
void adminDashboard();
void staffDashboard();
void searchStock();
void addStock();
void updateStock();
void deleteStock();
void displayAllStocks();
void trackInventory();
void generateLowStockAlerts();
void printStockReport();

// for User
void addItemToCart();
void viewCart(); 
void buyStock();
void checkoutCart(const string& username);

// Global data storage
vector<User> users;
vector<Stock> stocks;
vector<Receipt> receipts;  // Store all receipts
vector<pair<Stock, int>> cart; // Global cart to hold items added by users
User* currentUser = nullptr;

// ─── Main Function ──────────────────────────────────────────────
int main() {
   string lightBlue = "\033[94m";
    string cyan = "\033[36m";
    string reset = "\033[0m";

    // Top section (light blue)
    cout << lightBlue;
    cout << R"(
 _    _      _                            _____      ______                                           
| |  | |    | |                          |_   _|     |  _  \                                          
| |  | | ___| | ___ ___  _ __ ___   ___    | | ___   | | | |___  _ __ __ _  ___ _ __ ___   ___  _ __  
| |/\| |/ _ \ |/ __/ _ \| '_ ` _ \ / _ \   | |/ _ \  | | | / _ \| '__/ _` |/ _ \ '_ ` _ \ / _ \| '_ \ 
\  /\  /  __/ | (_| (_) | | | | | |  __/   | | (_) | | |/ / (_) | | | (_| |  __/ | | | | | (_) | | | |
 \/  \/ \___|_|\___\___/|_| |_| |_|\___|   \_/\___/  |___/ \___/|_|  \__,_|\___|_| |_| |_|\___/|_| |_|
                                                                                                      
                                                                                                      
)";

    // Bottom section (cyan)
    cout << cyan;
    cout << R"(          
           _             _     ___  ___                                                  _            
          | |           | |    |  \/  |                                                 | |           
       ___| |_ ___   ___| | __ | .  . | __ _ _ __   __ _  __ _  ___ _ __ ___   ___ _ __ | |_          
      / __| __/ _ \ / __| |/ / | |\/| |/ _` | '_ \ / _` |/ _` |/ _ \ '_ ` _ \ / _ \ '_ \| __|         
      \__ \ || (_) | (__|   <  | |  | | (_| | | | | (_| | (_| |  __/ | | | | |  __/ | | | |_          
      |___/\__\___/ \___|_|\_\ \_|  |_/\__,_|_| |_|\__,_|\__, |\___|_| |_| |_|\___|_| |_|\__|         
                                                          __/ |                                       
                                                         |___/                                        
)";

    cout << reset;


    try {
        DisplayUtil::displayWelcome();

        users = ExcelUtil::readUsersFromFile("data/users.xlsx");
        if (users.empty()) {
            users.emplace_back("admin", "adminpass", true);
            ExcelUtil::writeUsersToFile("data/users.xlsx", users);
        }

        receipts = ExcelUtil::readTransactionsFromFile("data/transactions.xlsx");
        if (receipts.empty()) {
        }

        stocks = ExcelUtil::readStockFromFile("data/stock.xlsx");
        if (stocks.empty()) {
            stocks.emplace_back(1, "Laptop", 10, 599.99);
            stocks.emplace_back(2, "Mouse", 30, 12.5);
            stocks.emplace_back(3, "Keyboard", 20, 25.0);
            ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
        }

        displayMainMenu();
    } catch (const exception& e) {
        return 1;
    }

    return 0;
}

// ─── Main Menu ──────────────────────────────────────────────────
void displayMainMenu() {
    int choice;
    do {
    cout << "\033[94m-------Select the role for your Requirement-------" << endl;
    cout << "\033[94m[1] Admin\033[0m" << endl;
    cout << "\033[94m[2] User\033[0m" << endl;
    cout << "\033[94m[3] Exit\033[0m" << endl;
    cout << "\033[94mEnter your choice: ";
        cin >> choice;
        switch (choice) {
            case 1:{
                system("cls");
                adminLogin();
                break;
            }
            case 2:{
                system("cls");
                userLogin(); // just add add to allow user login or registration
                break;
            }
            case 3:
                cout << "Exiting program. Goodbye!" << endl;
                break;
            default:
                cout << "Invalid choice. Please try again." << endl;
        }
    } while (choice != 3);
}
// ─── Admin Login ────────────────────────────────────────────────
void adminLogin() {
    string username, password;
    cout << "+--------------------+\n";
    cout << "|    Admin Login     |\n";
    cout << "+--------------------+\n";

    cout << "Username: ";
    cin >> username;
    cout << "Password: ";
    cin >> password;

    for (const auto& user : users) {
        if (user.getUsername() == username && user.getPassword() == password && user.isAdmin()) {
            cout << "Admin login successful!\n";
            adminDashboard();
            return;
        }
    }
    cout << "Invalid credentials or not an admin.\n";
}

// ─── Admin Dashboard ────────────────────────────────────────────
void adminDashboard() {
    int choice;
    system("cls");
    do {
        cout << "+--------------------+\n";
        cout << "|   Admin Dashboard  |\n";
        cout << "+--------------------+\n";
        cout << "[1] Add Stock\n";
        cout << "[2] Update Stock\n";
        cout << "[3] Delete Stock\n";
        cout << "[4] Search Stock\n";
        cout << "[5] Display All Stocks\n";
        cout << "[6] Track Inventory\n";
        cout << "[7] Generate Low-Stock Alerts\n";
        cout << "[8] Print Reports\n";
        cout << "[9] Logout\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch(choice) {
            case 1:{
                system("cls");
                addStock();
                break;
            }
            case 2:{
                system("cls");
                updateStock();
                break;
            }
            case 3:{
                system("cls");
                deleteStock();
                break;
            }
            case 4:{
                system("cls");
                searchStock();
                break;
            }
            case 5:{
                system("cls");
                displayAllStocks();
                break;
            }
            case 6:{
                system("cls");
                trackInventory();
                break;
            }
            case 7:{
                system("cls");
                generateLowStockAlerts(); 
                break;
            }
            case 8:{
                system("cls");
                printStockReport();
                break;  
            }
            case 9:{
                system("cls");
                cout << "Logging out..." << endl;
                currentUser = nullptr;
                break;
            }
            default:
                cout << "Feature not yet implemented." << endl;
                break;
        }
    } while (choice != 9);
}

// ─── Add Stock ──────────────────────────────────────────────────
void addStock() {
    string name;
    int quantity;
    double price;

    cout << "+--------------------+\n";
    cout << "|    Add new Stock   |\n";
    cout << "+--------------------+\n";
    cout << "Enter stock name: ";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, name);
    cout << "Quantity: ";
    cin >> quantity;
    cout << "Price: ";
    cin >> price;

    int newId = ExcelUtil::getNextStockId(stocks);
    stocks.emplace_back(newId, name, quantity, price);
    ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
    cout << "Stock added! ID: " << newId << endl;
}

// ─── Update Stock ───────────────────────────────────────────────
void updateStock() {
    int id;
    int choice;
    cout << "+--------------------+\n";
    cout << "|    Update Stock    |" << endl;
    cout << "+--------------------+\n";
    cout << "Enter stock ID to update: ";
    cin >> id;

    // Find the stock item by ID
    auto it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s) {
        return s.getId() == id;
    });

    if (it != stocks.end()) {
        cout << "Stock found: " << it->getName() << endl;
        cout << "[1] Update Name" << endl;
        cout << "[2] Update Quantity" << endl;
        cout << "[3] Update Price" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        string newName;
        int newQuantity;
        double newPrice;

        switch (choice) {
            case 1:
                cout << "Enter new name: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, newName);
                it->setName(newName);
                break;
            case 2:
                cout << "Enter new quantity: ";
                cin >> newQuantity;
                it->setQuantity(newQuantity);
                break;
            case 3:
                cout << "Enter new price: ";
                cin >> newPrice;
                it->setPrice(newPrice);
                break;
            default:
                cout << "Invalid choice. No changes made." << endl;
                return;
        }

        ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
        cout << "Stock updated successfully!" << endl;
    } else {
        cout << "Stock with ID " << id << " not found." << endl;
    }
}

// ─── Delete Stock ───────────────────────────────────────────────
void deleteStock() {
    int id;
    cout << "+--------------------+\n";
    cout << "|    Delete Stock    |\n";
    cout << "+--------------------+\n";
    cout << "Enter stock ID: ";
    cin >> id;

    auto it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s) {
        return s.getId() == id;
    });

    if (it != stocks.end()) {
        stocks.erase(it);
        ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
        cout << "Deleted stock with ID " << id << endl;
    } else {
        cout << " Stock not found.\n";
    }
}

// ─── Search Stock ───────────────────────────────────────────────
void searchStock() {
    cout << "+--------------------------+\n";
    cout << "|    Search Stock by ID    |\n";
    cout << "+--------------------------+\n";
    
    int id;
    cout << "Enter ID: ";
    cin >> id;

    bool found = false;
    for (const auto& s : stocks) {
        if (s.getId() == id) {
            cout << "Stock Found:\n";
            cout << "ID: " << s.getId()
                 << " | Name: " << s.getName()
                 << " | Qty: " << s.getQuantity()
                 << " | Price: $" << s.getPrice() << endl;
            found = true;
            break; // Since ID is unique, break early
        }
    }

    if (!found) {
        cout << "No stock item found with ID: " << id << endl;
    }
}


// ─── Display All Stocks ─────────────────────────────────────────
void displayAllStocks() {
    DisplayUtil::displayStocks(stocks);
}
void trackInventory() {
    cout << "+----------------------------------+\n";
    cout << "|    Inventory Tracking Summary    |" << endl;
    cout << "+----------------------------------+\n";

    if (stocks.empty()) {
        cout << "No stock items to track. Inventory is empty." << endl;
        return;
    }

    int totalUniqueItems = stocks.size();
    long long totalQuantity = 0; // Use long long for total quantity to prevent overflow
    double totalPrice = 0.0; // Variable to store total price
    vector<Stock> lowStockItems;

    for (const auto& stock : stocks) {
        totalQuantity += stock.getQuantity();
        totalPrice += (static_cast<double>(stock.getQuantity()) * stock.getPrice()); // Calculate total price
        if (stock.getQuantity() < LOW_STOCK_THRESHOLD) {
            lowStockItems.push_back(stock);
        }
    }

    cout << "Total unique stock items: " << totalUniqueItems << endl;
    cout << "Total quantity of all items: " << totalQuantity << endl;
    cout << "Total value of all items: $" << fixed << setprecision(2) << totalPrice << endl; // Display total price

    cout << "\n--- Low Stock Alerts (Quantity < " << LOW_STOCK_THRESHOLD << ") ---" << endl;
    if (lowStockItems.empty()) {
        cout << "No items are currently low in stock. Good job!" << endl;
    } else {
        cout << "The following items are running low:" << endl;
        cout << "ID\tName\t\tQuantity\tPrice" << endl;
        cout << "-----------------------------------------------------" << endl;
        for (const auto& item : lowStockItems) {
            // Adjust spacing for better alignment if name is short
            cout << item.getId() << "\t" << item.getName();
            if (item.getName().length() < 20) { // Heuristic for short names
                cout << "\t\t";
            } else {
                cout << "\t";
            }
            cout << item.getQuantity() << "\t\t" << item.getPrice() << endl;
        }
        cout << "-----------------------------------------------------" << endl;
    }
}

// ─── Stub Functions ─────────────────────────────────────────────


//Function Generate Low Stock Alerts
void generateLowStockAlerts() {
    cout << "+----------------------------+\n";
    cout << "|       Low Stock Alerts     |" << endl;
    cout << "+----------------------------+\n";
    vector<Stock> LowStockItems;
    
    for (const auto& stock : stocks){
        if (stock.getQuantity() < LOW_STOCK_THRESHOLD){
            LowStockItems.push_back(stock);
        }
    }
    if (LowStockItems.empty()){
        cout << "No items are currently Low in stock." << endl;
    }else{
        cout << "The following items are low:" << endl;
        for (const auto& item : LowStockItems){
            cout << "ID: " << item.getId()
                 << " | Name:" << item.getName()
                 << " | Quantity:" << item.getQuantity()
                 << " | Price:" << item.getPrice() << endl;
        }
    }
}


// Function to register a new user
void userRegister() { // just add add for allowing new user registration
    string username, password;
    cout << "+----------------------------+\n";
    cout << "|     User Registration      |" << endl;
    cout << "+----------------------------+\n";
    cout << "Enter new username: ";
    getline(cin, username);
    cout << "Enter new password: ";
    getline(cin, password);

    bool userExists = false;
    for (const auto& user : users) {
        if (user.getUsername() == username) {
            userExists = true;
            break;
        }
    }

    if (userExists) {
        cout << "Username already exists. Please choose a different one." << endl;
    } else {
        users.emplace_back(username, password, false);
        ExcelUtil::writeUsersToFile("data/users.xlsx", users);
        cout << "User registered successfully!" << endl;
    }
}

// Function for user login and registration menu
void userLogin() { // just add add to enable user to login or register
    int choice;
    cout << "+----------------------+\n";
    cout << "|       User Menu      |" << endl;
    cout << "+----------------------+\n";
    cout << "[1] Register" << endl;
    cout << "[2] login" << endl;
    cout << "[3] Back to Main Menu" << endl;
    cout << "Enter your choice: ";
    cin >> choice;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    switch (choice) {
        case 1: {
            system("cls");
            userRegister();
            break;
        }
        case 2:{
            system("cls");
            string username, password;
            cout << "+----------------------+\n";
            cout << "|       User Login     |" << endl;
            cout << "+----------------------+\n";
            cout << "Username: ";
            getline(cin, username);
            cout << "Password: ";
            getline(cin, password);

            bool loginSuccess = false;
            for (auto& user : users) { // new correct
                if (user.getUsername() == username && user.getPassword() == password && !user.isAdmin()) {
                currentUser = &user; // Assign the global pointer to the found user
                    break;
                }
            }
            if (currentUser != nullptr) {
                cout << "User login successful!" << endl;
                staffDashboard(); // just add add to give access to user dashboard
            } else {
                cout << "Invalid username or password, or you are an admin. Please use the admin login." << endl;
            }
            break;

        }
        case 3:{
            system("cls");
            cout << "Returning to main menu..." << endl;
            break;
        }
        default:
            cout << "Invalid choice. Please try again." << endl;
            break;
    }
}

// Staff dashboard for regular users
void staffDashboard() { // just add add for allowing staff actions
    int choice;
    system("cls");
    do {
        cout << "+--------------------------+\n";
        cout << "|      User Dashboard     |" << endl;
        cout << "+--------------------------+\n";
        cout << "  Welcome, To Our shop pls enjoy buying what you want !!" << endl;
        cout << "[1] Display All Stocks" << endl;
        cout << "[2] Search Stock" << endl;
        // cout << "[3] Buy Stock" << endl;
        cout << "[3] Add Item to Cart" << endl; // Added Cart option
        cout << "[4] View Cart" << endl;       // Added Cart option
        cout << "[5] Checkout the cart" << endl;
        cout << "[6] Logout" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        switch(choice) {
            case 1:{
                system("cls");
                displayAllStocks();
                break;
            }
            case 2:{
                system("cls");
                searchStock();
                break;
            }
            // case 3:{
            //     system("cls");
            //     buyStock(); // just add add to allow purchasing items
            //     break;
            // }
            case 3:{
                system("cls");
                addItemToCart(); // Call the new function
                break;
            }
            case 4:{
                system("cls");
                viewCart(); // Call the new function
                break;
            }
            case 5:
                if (currentUser) {
                    checkoutCart(currentUser->getUsername()); // Pass current user's username
                } else {
                    cout << "Error: No user logged in for checkout." << endl;
                }
                break;
            case 6:
                cout << "Logging out..." << endl;
                currentUser = nullptr; // Clear current user on logout
                break;


            default:
                cout << "Invalid choice. Please try again." << endl;
                break;
        }
    } while (choice != 6);
}

// Buy Stock Function (for staff)
// Buy Stock Function (for staff)
void buyStock() {
    int id, quantity;
    cout << "\n--- Buy Stock ---" << endl;
    cout << "Enter stock ID to purchase: ";
    cin >> id;
    cout << "Enter quantity: ";
    cin >> quantity;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    auto it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s) {
        return s.getId() == id;
    });

    if (it != stocks.end()) {
        if (it->getQuantity() >= quantity) {
            // Create a temporary vector to hold the single item for the receipt
            vector<pair<Stock, int>> items;
            items.push_back({*it, quantity});
            
            // Create a receipt
            // Note: In a real-world app, receipt IDs would be persisted and tracked.
            // For now, we'll use a simple counter based on the global receipts vector size.
            //int receiptId = receipts.size() + 1;
            int receiptId = ExcelUtil::getNextReceiptId(receipts);  
            string username = (currentUser != nullptr) ? currentUser->getUsername() : "Guest";
            Receipt newReceipt(receiptId, items, username); // Pass username to Receipt constructor
            receipts.push_back(newReceipt);

            // Update the stock quantity
            it->setQuantity(it->getQuantity() - quantity);
            ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);

            cout << "\nPurchase successful!" << endl;
            cout << "Receipt ID: " << newReceipt.getReceiptId() << endl;
            cout << "Items purchased: " << it->getName() << " x " << quantity << endl;
            cout << "Total Price: $" << fixed << setprecision(2) << newReceipt.getTotalPrice() << endl;
        } else {
            cout << "Insufficient stock. Available quantity: " << it->getQuantity() << endl;
        }
    } else {
        cout << "Stock with ID " << id << " not found." << endl;
    }
}

// Function to add an item to the global cart
void addItemToCart() {
    int id, qty;
    cout << "+-----------------------+\n";
    cout << "|       Add to Cart     |\n"; 
    cout << "+-----------------------+\n";
    displayAllStocks(); // Show available stocks
    cout << "Enter Stock ID: "; 
    cin >> id;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer

    auto it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s){return s.getId()==id;});
    if (it==stocks.end()){ 
        cout << "Item not found.\n"; 
        return; 
    }
    cout << "Enter quantity: "; 
    cin >> qty;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer

    if (qty > 0 && qty <= it->getQuantity()){
        // Check if item already in cart, if so, update quantity
        auto cart_it = find_if(cart.begin(), cart.end(), [id](const pair<Stock, int>& p){
            return p.first.getId() == id;
        });

        if (cart_it != cart.end()) {
            // Item already in cart, update its quantity
            cart_it->second += qty;
            cout << qty << " more of " << it->getName() << " added. Total in cart: " << cart_it->second << ".\n";
        } else {
            // Item not in cart, add new entry
            cart.push_back({*it, qty});
            cout << qty << " x " << it->getName() << " added to cart.\n";
        }
    } else {
        cout << "Invalid quantity or insufficient stock. Available: " << it->getQuantity() << ".\n";
    }
}

// Function to view the contents of the global cart
void viewCart() {
    cout << "+-----------------------+\n";
    cout << "|        Your Cart      |\n";
    cout << "+-----------------------+\n";
    if (cart.empty()){ 
        cout << "Cart is empty.\n"; 
        return; 
    }
    double total = 0;
    cout << fixed << setprecision(2); // Set precision for currency
    for (auto &c : cart) {
        double itemTotal = c.first.getPrice() * c.second;
        cout << c.first.getName() << " x " << c.second << " = $" << itemTotal << endl;
        total += itemTotal;
    }
    cout << "-------------------\n";
    cout << "Total: $" << total << endl;
}


void printStockReport() {
    // ─── Current Date & Time ──────────────────────────────
    time_t now = time(nullptr);
    char dateBuf[100];
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    // ─── Report Header ────────────────────────────────────
    cout << "\n========== STOCK SUMMARY REPORT ==========\n";
    cout << "Generated on: " << dateBuf << "\n\n";

    // ─── Stats Setup ──────────────────────────────────────
    int totalItems = 0;
    int totalQuantity = 0;
    double totalValue = 0;

    const Stock* mostExpensive = nullptr;
    const Stock* leastExpensive = nullptr;
    const Stock* mostStocked = nullptr;
    const Stock* leastStocked = nullptr;

    for (const auto& s : stocks) {
        double itemValue = s.getQuantity() * s.getPrice();
        totalItems++;
        totalQuantity += s.getQuantity();
        totalValue += itemValue;

        if (!mostExpensive || s.getPrice() > mostExpensive->getPrice()) {
            mostExpensive = &s;
        }
        if (!leastExpensive || s.getPrice() < leastExpensive->getPrice()) {
            leastExpensive = &s;
        }
        if (!mostStocked || s.getQuantity() > mostStocked->getQuantity()) {
            mostStocked = &s;
        }
        if (!leastStocked || s.getQuantity() < leastStocked->getQuantity()) {
            leastStocked = &s;
        }
    }

    // ─── Summary ──────────────────────────────────────────
    cout << "     Total Items:     " << totalItems << "\n";
    cout << "    Total Quantity:  " << totalQuantity << "\n";
    cout << "    Inventory Value: $" << fixed << setprecision(2) << totalValue << "\n\n";

    if (mostExpensive) {
        cout << " Most Expensive: " << mostExpensive->getName()
             << " ($" << mostExpensive->getPrice() << ")\n";
    }

    if (leastExpensive) {
        cout << " Least Expensive: " << leastExpensive->getName()
             << " ($" << leastExpensive->getPrice() << ")\n";
    }

    if (mostStocked) {
        cout << " Most Stocked:   " << mostStocked->getName()
             << " (" << mostStocked->getQuantity() << " item)\n";
    }

    if (leastStocked) {
        cout << " Least Stocked:  " << leastStocked->getName()
             << " (" << leastStocked->getQuantity() << " item)\n";
    }

    cout << "==========================================\n\n";
        // Display recent transactions
    cout << "\n--- Recent Transactions ---" << endl;
    if (receipts.empty()) {
        cout << "No transactions recorded yet." << endl;
    } else {
        int count = 0;
        for (auto it = receipts.rbegin(); it != receipts.rend() && count < 5; ++it, ++count) {
            time_t transactionTime = it->getTransactionTime();
            tm ptm{};
            if (localtime_s(&ptm, &transactionTime) == 0) {
                cout << "Receipt ID: " << it->getReceiptId()
                          << ", User: " << it->getUsername()
                          << ", Total: $" << fixed << setprecision(2) << it->getTotalPrice()
                          << ", Time: " << put_time(&ptm, "%Y-%m-%d %H:%M:%S") << endl;
            } else {
                cout << "Receipt ID: " << it->getReceiptId()
                          << ", User: " << it->getUsername()
                          << ", Total: $" << fixed << setprecision(2) << it->getTotalPrice()
                          << ", Time: [Invalid Time]" << endl;
            }

            for (const auto& item : it->getItems()) {
                cout << "    - " << item.first.getName()
                          << " x " << item.second
                          << " @ $" << item.first.getPrice() << endl;
            }
        }

        if (receipts.size() > 5) {
            cout << "(Displaying last 5 transactions. Total transactions: " << receipts.size() << ")" << endl;
        }
    }
}
// Function to checkout the cart
void checkoutCart(const string& username) {
    cout << "\n--- Checking out Cart ---\n";
    if (cart.empty()) {
        cout << "Your cart is empty. Nothing to checkout.\n";
        return;
    }

    char confirm;
    cout << "Confirm purchase of items in cart? (y/n): ";
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (tolower(confirm) != 'y') {
        cout << "Checkout cancelled.\n";
        return;
    }

    int receiptId = ExcelUtil::getNextReceiptId(receipts);
    vector<pair<Stock, int>> purchasedItemsForReceipt;
    double totalCartPrice = 0.0;
    bool transactionSuccessful = true;

    for (auto& cart_item : cart) {
        int id = cart_item.first.getId();
        int quantityToBuy = cart_item.second;

        auto stock_it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s){
            return s.getId() == id;
        });

        if (stock_it != stocks.end()) {
            if (stock_it->getQuantity() >= quantityToBuy) {
                stock_it->setQuantity(stock_it->getQuantity() - quantityToBuy);
                purchasedItemsForReceipt.push_back({*stock_it, quantityToBuy});
                totalCartPrice += stock_it->getPrice() * quantityToBuy;
            } else {
                cout << "Insufficient stock for " << stock_it->getName()
                          << ". Available: " << stock_it->getQuantity() << ". Purchase failed for this item.\n";
                transactionSuccessful = false;
            }
        } else {
            cout << "Error: Item ID " << id << " not found in stock. Skipping.\n";
            transactionSuccessful = false;
        }
    }

    if (transactionSuccessful && !purchasedItemsForReceipt.empty()) {
        Receipt newReceipt(receiptId, purchasedItemsForReceipt, username);
        receipts.push_back(newReceipt);

        ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
        ExcelUtil::writeTransactionsToFile("data/transactions.xlsx", receipts);

        cout << "\nCheckout successful!\n";
        cout << "Receipt ID: " << newReceipt.getReceiptId() << endl;
        cout << "Total amount: $" << fixed << setprecision(2) << newReceipt.getTotalPrice() << endl;
        cout << "Thank you for your purchase, " << username << "!\n";
        cart.clear();
    } else if (!purchasedItemsForReceipt.empty()) {
        cout << "\nCheckout partially successful. Some items could not be purchased due to insufficient stock.\n";
        cart.clear();
    } else {
        cout << "\nCheckout failed. No items were successfully purchased.\n";
        cart.clear();
    }
}




