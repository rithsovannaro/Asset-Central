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
#include <windows.h>
#include <conio.h>   

using namespace std;
const int LOW_STOCK_THRESHOLD = 20; 


// Platform-specific includes for password masking
#ifdef _WIN32
    #include <conio.h>
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <sys/types.h>
#endif

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
string getPasswordInput(const string& prompt);

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

string getPasswordInput(const string& prompt) {
    string password;
    cout << prompt;
    cout.flush(); // Ensure prompt is displayed immediately

#ifdef _WIN32
    // Windows implementation
    char ch;
    while (true) {
        ch = _getch();

        if (ch == '\r') { // Enter key
            break;
        } else if (ch == '\b') { // Backspace
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b"; // Erase the asterisk
                cout.flush();
            }
        } else if (ch >= 32 && ch <= 126) { // Printable characters
            password += ch;
            cout << '*';
            cout.flush();
        }
        // Ignore other special characters
    }
#else
    // Unix/Linux implementation
    struct termios oldTermios, newTermios;

    // Get current terminal attributes
    if (tcgetattr(STDIN_FILENO, &oldTermios) != 0) {
        // Fallback
        cout << "\nWarning: Cannot hide password input on this terminal." << endl;
        cout << "Password: ";
        getline(cin, password);
        return password;
    }

    // Disable echo & canonical mode
    newTermios = oldTermios;
    newTermios.c_lflag &= ~(ECHO | ICANON);
    newTermios.c_cc[VMIN] = 1;
    newTermios.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newTermios) != 0) {
        cout << "\nWarning: Cannot hide password input on this terminal." << endl;
        cout << "Password: ";
        getline(cin, password);
        tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
        return password;
    }

    char ch;
    while (true) {
        if (read(STDIN_FILENO, &ch, 1) != 1) {
            break;
        }

        if (ch == '\n' || ch == '\r') {
            break;
        } else if (ch == 127 || ch == 8) {
            if (!password.empty()) {
                password.pop_back();
                cout << "\b \b";
                cout.flush();
            }
        } else if (ch >= 32 && ch <= 126) {
            password += ch;
            cout << '*';
            cout.flush();
        }
    }

    // Restore terminal
    tcsetattr(STDIN_FILENO, TCSANOW, &oldTermios);
#endif

    cout << endl;
    return password;
}


// ─── Main Function ──────────────────────────────────────────────
int main() {
    system("cls");
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
        // DisplayUtil::displayWelcome();

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
    cout << "            \033[94m                                                -------Select the role for your Requirement-------" << endl;
    cout << "                         \033[94m                                          [1] Admin\033[0m" << endl;
    cout << "                         \033[94m                                          [2] User\033[0m" << endl;
    cout << "                         \033[94m                                          [3] Exit\033[0m" << endl;
    cout << "                         \033[94m                                          Enter your choice: ";
        if (!(cin >> choice)) {  
            cin.clear(); // clear fail state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
            cout << "\033[31mInvalid input! Please enter a number between 1 and 3.\033[0m\n\n";
            continue; // skip to next loop iteration
        }

        switch (choice) {
            case 1:{
                // system("cls");
                adminLogin();
                break;
            }
            case 2:{
                // system("cls");
                userLogin(); // just add add to allow user login or registration
                break;
            }
            case 3:
                cout << "\033[31mExiting program. Goodbye!\033[0m" << endl;
                break;
            default:
                cout << "\033[31mInvalid input! Please enter a number between 1 and 3.\033[0m\n\n";
        }
    } while (choice != 3);
}

//add color laoding

void SetColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void loadingAnimation() {
     const int barWidth = 30;

    std::cout << "                                                                      Loading: [";

    // Print empty bar initially
    for (int i = 0; i < barWidth; ++i) std::cout << " ";
    std::cout << "\r                                                                    Loading: ";

    for (int i = 0; i <= barWidth; ++i) {
        // Change color depending on progress
        if (i < barWidth / 3)
            SetColor(7);      // Green
        else if (i < 2 * barWidth / 3)
            SetColor(14);      // Yellow
        else
            SetColor(10);    

        // Print the progress bar block
        for (int j = 0; j < i; ++j) std::cout << "\xDB";  // ▓ character (full block)

        // Reset color for the rest of the bar
        SetColor(7);

        // Print spaces for the rest of the bar
        for (int j = i; j < barWidth; ++j) std::cout << " ";

        std::cout << "\r                                                                Loading: ";
        std::cout.flush();

        Sleep(100);
    }

    // Reset color and clear the line
    SetColor(7);
    std::cout << std::string(50, ' ') << "\r";  // Clear current line

    // Print only Done!
    std::cout << "                                                                      login successfull !" << std::endl;
}

// ─── Admin Login ────────────────────────────────────────────────
void adminLogin() {
    system("cls");
    string username, password;
   cout << "\033[36m";
    cout << "                                                             ___      _           _          _                 _       \n";
    cout << "                                                            / _ \\    | |         (_)        | |               (_)      \n";
    cout << "                                                           / /_\\ \\ __| |_ __ ___  _ _ __    | |     ___   __ _ _ _ __  \n";
    cout << "                                                           |  _  |/ _` | '_ ` _ \\| | '_ \\   | |    / _ \\ / _` | | '_ \\ \n";
    cout << "                                                           | | | | (_| | | | | | | | | | |  | |___| (_) | (_| | | | | |\n";
    cout << "                                                           \\_| |_/\\__,_|_| |_| |_|_|_| |_|  \\_____/\\___/ \\__, |_|_| |_|\n";
    cout << "                                                                                                            __/ |        \n";
    cout << "                                                                                                           |____/         \n";
    cout << "\033[0m"; 

    SetColor(9);

   
    cout << "                                                                           Username: ";
    cin >> username;
    password = getPasswordInput("                                                                           Password: ");
    // cout << "                                                                           Password: ";
    // cin >> password;

   for (const auto& user : users) {
        if (user.getUsername() == username && user.getPassword() == password && user.isAdmin()) {
            loadingAnimation();  // Show loading animation after successful login
            adminDashboard();
            return;
        }
    }
    cout << "\033[31mInvalid credentials or not an admin.\033[0m\n";
}

// ─── Admin Dashboard ────────────────────────────────────────────
void adminDashboard() {
    system("cls");
    int choice;
    do {
        
        cout << "\033[34m"; // Light blue (cyan)
        cout << "                                                 ___      _           _        ______          _     _                         _  \n";
        cout << "                                                / _ \\    | |         (_)       |  _  \\        | |   | |                       | | \n";
        cout << "                                               / /_\\ \\ __| |_ __ ___  _ _ __   | | | |__ _ ___| |__ | |__   ___   __ _ _ __ __| | \n";
        cout << "                                               |  _  |/ _` | '_ ` _ \\| | '_ \\  | | | / _` / __| '_ \\| '_ \\ / _ \\ / _` | '__/ _` | \n";
        cout << "                                               | | | | (_| | | | | | | | | | | | |/ / (_| \\__ \\ | | | |_) | (_) | (_| | | | (_| | \n";
        cout << "                                               \\_| |_/\\__,_|_| |_| |_|_|_| |_| |___/ \\__,_|___/_| |_|_.__/ \\___/ \\__,_|_|  \\__,_| \n";
        cout << "                                                                                                                                  \n";
        cout << "                                                                                                                                      \n";
        cout << "\033[0m"; // Reset to default

        SetColor(9);
        cout << " " << endl;
        cout << "                                                                       [1] Add Stock\n";
        cout << "                                                                       [2] Update Stock\n";
        cout << "                                                                       [3] Delete Stock\n";
        cout << "                                                                       [4] Search Stock\n";
        cout << "                                                                       [5] Display All Stocks\n";
        cout << "                                                                       [6] Track Inventory\n";
        cout << "                                                                       [7] Generate Low-Stock Alerts\n";
        cout << "                                                                       [8] Print Reports\n";
        cout << "                                                                       [9] Logout\n";
        cout << "                                                                           Enter your choice: ";
        if (!(cin >> choice)) {  
            system("cls");
            cin.clear(); // clear fail state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
            cout << "\033[31mInvalid input! Please enter a number between 1 and 9.\n\n\033[0m" << endl;
            return adminDashboard(); // skip to next loop iteration
        }
        SetColor(9);
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
            system("cls");
                cout << "\033[31mInvalid input! Please enter a number between 1 and 9.\n\n\033[0m" << endl;
                break;
        }
    } while (choice != 9);
}

// ─── Add Stock ──────────────────────────────────────────────────
void addStock() {
    
    string name;
    int quantity;
    double price;

    cout << "\033[34m"; // Blue
    cout << "                                                   __    ____  ____     _  _  ____  _    _    ___  ____  _____  ___  _  _     \n";
    cout << "                                                  /__\\  (  _ \\(  _ \\   ( \\( )( ___)( \\/\\/ )  / __)(_  _)(  _  )/ __)( )/ )    \n";
    cout << "                                                 /(__)\\  )(_) ))(_) )   )  (  )__)  )    (   \\__ \\  )(   )_)(( (__  )  (     \n";
    cout << "                                                (__)(__)(____/(____/   (_)_/)(____)(__\\/__)  (___/ (__) (_____)(___)(_)\_)    \n";
    cout << "\033[0m"; // Reset

    SetColor(9);
    cout << "                                                                                                 "<< endl;
    cout << "                                                                               Enter stock name: ";

    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    getline(cin, name);
    cout << "                                                                           Quantity: ";
    while (!(cin >> quantity) || quantity < 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "\033[31mInvalid input! Quantity must be a positive number: \033[0m";
    }
    cout << "                                                                           Price: ";
    while (!(cin >> price) || price < 0) {
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "\033[31mInvalid input! Price must be a positive number: \033[0m";
    }

    int newId = ExcelUtil::getNextStockId(stocks);
    stocks.emplace_back(newId, name, quantity, price);
    ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
    cout << "                                                                           Stock added! ID: " << newId << endl;
}

// ─── Update Stock ───────────────────────────────────────────────
void updateStock() {
   
    int id;
    int choice;
    cout << "\033[34m"; // Blue
    cout << "                                                   __  __  ____  ____    __   ____  ____    ___  ____  _____  ___  _  _ \n";
    cout << "                                                  (  )(  )(  _ \\(  _ \\  /__\\ (_  _)( ___)  / __)(_  _)(  _  )/ __)( )/ )\n";
    cout << "                                                   )(__)(  )___/ )(_) )/(__)\\  )(   )__)   \\__ \\  )(   )_)(( (__  )  ( \n";
    cout << "                                                  (______)(__)  (____/(__)(__)(__) (____)  (___/ (__) (_____)(___)(_)\_)\n";
    cout << "\033[0m"; // Reset
    SetColor(9);

    cout << " " << endl;
    cout << "                                                                              Enter stock ID to update: ";
    cin >> id;

    // Find the stock item by ID
    auto it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s) {
        return s.getId() == id;
    });

    if (it != stocks.end()) {
        cout << "                                                                          Stock found: " << it->getName() << endl;
        cout << "                                                                          [1] Update Name" << endl;
        cout << "                                                                          [2] Update Quantity" << endl;
        cout << "                                                                          [3] Update Price" << endl;
        cout << "                                                                              Enter your choice: ";
        if (!(cin >> choice)) {  
            cin.clear(); // clear fail state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
            cout << "\033[31mInvalid input! Please enter a number between 1 and 9.\033[0m\n\n";
            return; 
        }

        string newName;
        int newQuantity;
        double newPrice;

        switch (choice) {
            case 1:{
                system("cls");
                cout << "                                                                  Enter new name: ";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, newName);
                it->setName(newName);
                break;
            }
            case 2:{
                system("cls");
                cout << "                                                                  Enter new quantity: ";
                cin >> newQuantity;
                it->setQuantity(newQuantity);
                break;
            }
            case 3:{
                system("cls");
                cout << "                                                                  Enter new price: ";
                cin >> newPrice;
                it->setPrice(newPrice);
                break;
            }
            default:
                cout << "\033[31mInvalid input! Please enter a number between 1 and 9.\033[0m\n\n";
                return;
        }

        ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
        cout << "Stock updated successfully!" << endl;
    } else {
        cout << "Stock with ID " << id << "\033[31m not found.\033[0m" << endl;
    }
}

// ─── Delete Stock ───────────────────────────────────────────────
void deleteStock() {
   
    int id;
    cout << "\033[34m"; // Blue
    cout << "                                                     ____  ____  __    ____  ____  ____    ___  ____  _____  ___  _  _   \n";
    cout << "                                                    (  _ \\( ___)(  )  ( ___)(_  _)( ___)  / __)(_  _)(  _  )/ __)( )/ )  \n";
    cout << "                                                     )(_) ))__)  )(__  )__)   )(   )__)   \\__ \\  )(   )_)(( (__  )  (   \n";
    cout << "                                                    (____/(____)(____)(____) (__) (____)  (___/ (__) (_____)(___)(_)\_)  \n";
    cout << "\033[0m"; // Reset

    SetColor(9);
    cout << "  " << endl;
    cout << "                                                                               Enter stock ID: ";
    cin >> id;

    auto it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s) {
        return s.getId() == id;
    });

    if (it != stocks.end()) {
        stocks.erase(it);
        ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
        cout << "\033[31mDeleted stock with ID \033[0m" << id << endl;
    } else {
        cout << "\033[31m Stock not found.\033[0m\n";
    }
}

// ─── Search Stock ───────────────────────────────────────────────
void searchStock() {
    
    cout << "\033[34m";  // 9 = Light Blue

    cout << "                                     ___  ____    __    ____   ___  _   _    ___  ____  _____  ___  _  _    ____  _  _    ____  ____  \n";
    cout << "                                    / __)( ___)  /__\\  (  _ \\ / __)( )_( )  / __)(_  _)(  _  )/ __)( )/ )  (  _ \\( \\/ )  (_  _)(  _ \\ \n";
    cout << "                                    \\__ \\ )__)  /(__)\\  )   /( (__  ) _ (   \\__ \\  )(   )(_)(( (__  )  (    ) _ < \\  /    _)(_  )(_) )\n";
    cout << "                                    (___/(____)(__)(__)(_) \_) \\___)(_) (_)  (___/ (__) (_____) \___)(_)\_)   (____/ (__)   (____)(____/ \n";

    cout << "\033[0m"; 

    SetColor(9);
    int id;
    cout << " " << endl;
    cout << "                                                   Enter ID: ";
    cin >> id;

    bool found = false;
    for (const auto& s : stocks) {
        if (s.getId() == id) {
            cout << "                                                   Stock Found:\n";
            cout << "          ID:  "<< s.getId()
                 << "                    | Name:  " << s.getName()
                 << "                    | Quantity:  " << s.getQuantity()
                 << "                    | Price:"<< s.getPrice() << endl;
            found = true;
            break; // Since ID is unique, break early
        }
    }

    if (!found) {
        cout << "\033[31mNo stock item found with ID: " << id << "\033[0m" << endl;
    }
}


// ─── Display All Stocks ─────────────────────────────────────────
void displayAllStocks() {
    SetColor(9);
    DisplayUtil::displayStocks(stocks);
}
void trackInventory() {
    
    cout << "\033[34m"; 
    cout << "--------------------Tracking inventory--------------------";
    cout << "\033[0m"; 

    SetColor(9);
    cout << " " << endl;
    if (stocks.empty()) {
        cout << "\033[31mNo stock items to track. Inventory is empty.\033[0m\n" << endl;
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
        cout << "\033[31mNo items are currently low in stock. Good job!\033[0m" << endl;
    } else {
        cout << "\033[31mThe following items are running low: \033[0m" << endl;
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
    SetColor(9);
    cout << "                                                                           +----------------------------+\n";
    cout << "                                                                           |       Low Stock Alerts     |" << endl;
    cout << "                                                                           +----------------------------+\n";
    vector<Stock> LowStockItems;
    
    for (const auto& stock : stocks){
        if (stock.getQuantity() < LOW_STOCK_THRESHOLD){
            LowStockItems.push_back(stock);
        }
    }
    if (LowStockItems.empty()){
        cout << "\033[31mNo items are currently Low in stock.\033[0m" << endl;
    }else{
        cout << "                                                                       The following items are low:" << endl;
        for (const auto& item : LowStockItems){
            cout << "             ID:  " << item.getId()
                 << "                       | Name:  " << item.getName()
                 << "                       | Quantity:  " << item.getQuantity()
                 << "                       | Price:  " << item.getPrice() << endl;
        }
    }
}


// Function to register a new user
void userRegister() { // just add add for allowing new user registration
    SetColor(9);
    string username, password;
    cout << "                                                                   +----------------------------+\n";
    cout << "                                                                   |     User Registration      |" << endl;
    cout << "                                                                   +----------------------------+\n";
    cout << "                                                                   Enter new username: ";
    getline(cin, username);
    cout << "                                                                   Enter new password: ";
    getline(cin, password);

    bool userExists = false;
    for (const auto& user : users) {
        if (user.getUsername() == username) {
            userExists = true;
            break;
        }
    }

    if (userExists) {
        cout << "\033[31mUsername already exists. Please choose a different one.\033[0m" << endl;
    } else {
        users.emplace_back(username, password, false);
        ExcelUtil::writeUsersToFile("data/users.xlsx", users);
        cout << "User registered successfully!" << endl;
    }
}

// Function for user login and registration menu
void userLogin() { // just add add to enable user to login or register
    system("cls");
    SetColor(9);
    int choice;
    cout << "                                                                           +----------------------+\n";
    cout << "                                                                           |       User Menu      |" << endl;
    cout << "                                                                           +----------------------+\n";
    cout << "                                                                           [1] Register" << endl;
    cout << "                                                                           [2] login" << endl;
    cout << "                                                                           [3] Back to Main Menu" << endl;
    cout << "                                                                               Enter your choice: ";
    if (!(cin >> choice)) {  
        cin.clear(); // clear fail state
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
        cout << "Invalid input! Please enter a number between 1 and 3.\n\n";
        return; 
    }

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
            cout << "                                                                   +----------------------+\n";
            cout << "                                                                   |       User Login     |" << endl;
            cout << "                                                                   +----------------------+\n";
            cout << "                                                                   Username: ";
            getline(cin, username);
            password = getPasswordInput("                                                                   Password: ");
            // getline(cin, password); 

            bool loginSuccess = false;
            for (auto& user : users) {
                if (user.getUsername() == username && user.getPassword() == password && !user.isAdmin()) {
                    loadingAnimation();  // Show loading animation after successful login
                    currentUser = &user;
                    loginSuccess = true;
                    break;
                }
            }

            if (loginSuccess) {
                loadingAnimation();  // <-- Call loading animation here!
                staffDashboard();
            } else {
                cout << "\033[31mInvalid username or password, or you are an admin. Please use the admin login.\033[0m" << endl;
            }
            break;

        }
        case 3:{
            system("cls");
            cout << "\033[31mReturning to main menu...\033[0m" << endl;
            break;
        }
        default:
            cout << "\033[31mInvalid input! Please enter a number between 1 and 3.\n\n\033[0m";
            break;
    }
}

// Staff dashboard for regular users
void staffDashboard() { // just add add for allowing staff actions
    int choice;
    // system("cls");

   
    do {
        cout << "\033[34m"; 
        cout << "                                            __  __  ___  ____  ____    ____    __    ___  _   _  ____  _____    __    ____  ____    " << endl;
        cout << "                                           (  )(  )/ __)( ___)(  _ \\  (  _ \\  /__\\  / __)( )_( )(  _ \\(  _  )  /__\\  (  _ \\(  _ \\   " << endl;
        cout << "                                            )(__)( \\__ \\ )__)  )   /   )(_) )/(__)\\ \\__ \\ ) _ (  ) _ < )(_)(  /(__)\\  )   / )(_) )  " << endl;
        cout << "                                           (______)(___/(____)(_)\\_)  (____/(__)(__)(___/(_) (_)(____/(_____)(__)(__)(_)\\_)(____/   " << endl;
        cout << "\033[0m"; 
         SetColor(9);
        cout << " " << endl;
        cout << "                                                              Welcome, To Our shop pls enjoy buying what you want !!" << endl;
        cout << "                                                                       [1] Display All Stocks" << endl;
        cout << "                                                                       [2] Search Stock" << endl;
        cout << "                                                                       [3] Add Item to Cart" << endl; // Added Cart option
        cout << "                                                                       [4] View Cart" << endl;       // Added Cart option
        cout << "                                                                       [5] Checkout the cart" << endl;
        cout << "                                                                       [6] Logout" << endl;
        cout << "                                                                           Enter your choice: ";
    if (!(cin >> choice)) {
        system("cls");  
        cin.clear(); // clear fail state
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
        cout << "\033[31mInvalid input! Please enter a number between 1 and 6.\033[0m\n\n";
        return staffDashboard(); 
    }
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
                    cout << "\033[31mError: No user logged in for checkout.\033[0m" << endl;
                }
                break;
            case 6:{
                system("cls");
                cout << "Logging out..." << endl;
                currentUser = nullptr; // Clear current user on logout
                break;
            }

            default:
            system("cls");
            // staffDashboard();
            cout << "\033[31mInvalid choice. Please try again.\033[0m" << endl;
            break;
        }
    } while (choice != 6);
}

// Buy Stock Function (for staff)
void buyStock() {
    SetColor(9);
    int id, quantity;
    cout << "--- Buy Stock ---" << endl;
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
    cout << "\033[34m"; 
    cout << "                                                      __    ____  ____     ____  _____     ___    __    ____  ____     " << endl;
    cout << "                                                     /__\\  (  _ \\(  _ \\   (_  _)(  _  )   / __)  /__\\  (  _ \\(_  _)    " << endl;
    cout << "                                                    /(__)\\  )(_) ))(_) )    )(   )(_)(   ( (__  /(__)\\  )   /  )(      " << endl;
    cout << "                                                   (__)(__)(____/(____/    (__) (_____)   \\___)(__)(__)(_)\\_) (__)     " << endl;
    cout << "\033[0m";

    SetColor(9);
    cout << " " << endl;
    displayAllStocks(); // Show available stocks
    cout << "Enter Stock ID: "; 
    cin >> id;
    cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear buffer

    auto it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s){return s.getId()==id;});
    if (it==stocks.end()){ 
        cout << "\033[31mItem not found.\033[0m\n"; 
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
        cout << "\033[31mInvalid quantity or insufficient stock. Available: 033[0m" << it->getQuantity() << ".\n";
    }
}

// Function to view the contents of the global cart
void viewCart() {
    SetColor(9);
    cout << "                                                                            +-----------------------+\n";
    cout << "                                                                            |        Your Cart      |\n";
    cout << "                                                                            +-----------------------+\n";
    if (cart.empty()){ 
        cout << "\033[31mCart is empty.033[0m\n"; 
        return; 
    }
    double total = 0;
    cout << "                                                                            \033[31m " << fixed << setprecision(2); // Set precision for currency
    for (auto &c : cart) {
        double itemTotal = c.first.getPrice() * c.second;
        cout << c.first.getName() << " x " << c.second << " = $" << itemTotal << endl;
        total +=itemTotal;
    }
    cout << "                                                                             -----------------------\n";
    cout << "                                                                              Total: $033[0m" << total << endl;
}


void printStockReport() {
    SetColor(4);
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
     cout << "\033[0m";
}
// Function to checkout the cart
void checkoutCart(const string& username) {
    system("cls");
    SetColor(9);
    cout << "\n\033[31m --- Checking out Cart ---\n";
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
                cout << "\033[31mInsufficient stock for " << stock_it->getName()
                          << ". Available: " << stock_it->getQuantity() << ". Purchase failed for this item.\n";
                transactionSuccessful = false;
            }
        } else {
            cout << "Error: Item ID " << id << " not found in stock. Skipping.033[0m\n";
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