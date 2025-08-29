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
#include <filesystem>  
#include <thread>
#include <chrono>
using namespace std;
const int LOW_STOCK_THRESHOLD = 20; 
namespace fs = filesystem;

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
void backupStockData();

// for User
void addItemToCart();
void viewCart(); 
void buyStock();
void checkoutCart(const string& username);
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}
void deleteUser();

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
    system("chcp 65001 > nul");
    string lightBlue = "\033[94m";
    string cyan = "\033[36m";
    string reset = "\033[0m";
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
    system("cls");
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
     vector<string> lines = {
        "                                                      _    _      _                            _____                    ",
        "                                                     | |  | |    | |                          |_   _|                   ",
        "                                                     | |  | | ___| | ___ ___  _ __ ___   ___    | | ___                 ",
        "                                                     | |/\\| |/ _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\   | |/ _ \\                ",
        "                                                     \\  /\\  /  __/ | (_| (_) | | | | | |  __/   | | (_) |               ",
        "                                                      \\/  \\/ \\___|_|\\___\\___/|_| |_| |_|\\___|   \\_/\\___/                ",
        "                                                                                                                        ",
        "                                            ___   _____ _____ _____ _____   _____  _____ _   _ ___________  ___   _     ",
        "                                           / _ \\ /  ___/  ___|  ___|_   _| /  __ \\|  ___| \\ | |_   _| ___ \\/ _ \\ | |    ",
        "                                          / /_\\ \\\\ `--.\\ `--.| |__   | |   | /  \\/| |__ |  \\| | | | | |_/ / /_\\ \\| |    ",
        "                                          |  _  | `--.\\\\`--. \\  __|  | |   | |    |  __|| . ` | | | |    /|  _  || |    ",
        "                                          | | | |/\\__/ /\\__/ / |___  | |   | \\__/\\| |___| |\\  | | | | |\\ \\| | | || |____",
        "                                          \\_| |_/\\____/\\____/\\____/  \\_/    \\____/\\____/\\_| \\_/ \\_/ \\_| \\_\\_| |_/\\_____/ "
    };
    
    // Clear screen
    cout << "\033[2J\033[H";
    
    // Set light blue color
    cout << "\033[94m";
    
    // Animate each line appearing from top to bottom
    for (size_t i = 0; i < lines.size(); ++i) {
        // Clear screen and position cursor at top
        cout << "\033[H";
        
        // Print all lines up to current line
        for (size_t j = 0; j <= i; ++j) {
            cout << lines[j] << "\n";
        }
        
        // Flush output to ensure immediate display
        cout.flush(); 
        // Wait before showing next line (adjust timing as needed)
        this_thread::sleep_for(chrono::milliseconds(70));
    }
    int choice;
    do {
        // Enhanced menu design - keeping your exact logic
        cout << "\033[96m\033[1m";
        cout << "\n                                       ╔══════════════════════════════════════════════════════════════════════════════════╗\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║" << "\033[93m" << "                        🎯 SELECT YOUR ROLE & REQUIREMENT 🎯                      " << "\033[96m" << "║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ╠══════════════════════════════════════════════════════════════════════════════════╣\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║         " << "\033[94m" << "[1] 🔐 Admin" << "\033[96m" << "                                                             ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Access administrative functions and system management" << "\033[96m" << "             ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║         " << "\033[94m" << "[2] 👤 User" << "\033[96m" << "                                                              ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Browse products, manage cart, and make purchases" << "\033[96m" << "                  ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║         " << "\033[94m" << "[3] 🚪 Exit" << "\033[96m" << "                                                              ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Close the application safely" << "\033[96m" << "                                      ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ╚══════════════════════════════════════════════════════════════════════════════════╝\n";
        cout << "\033[0m";
        
        // Your original input prompt with styling
        cout << "\n" << "\033[95m\033[1m" << "                                       💭 Enter your choice ▶ " << "\033[0m";
        cin >> choice;  
        cin.clear(); // clear fail state
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input            
        // Enhanced error message
        
        
        switch (choice) {
            case 1:{
                // Loading message for admin
                cout << "\n    " << "\033[96m" << "                                   🔐 Accessing Admin Panel";
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50); // Use usleep(300000) for Linux/Mac
                } 
                
                // system("cls"); // Uncomment if you want to clear
                adminLogin();
                break;
            }
            case 2:{
                // Loading message for user
                cout << "\n    " << "\033[96m" << "                                  👤 Loading User Portal";
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50); // Use usleep(300000) for Linux/Mac
                }
                
                // system("cls"); // Uncomment if you want to clear
                userLogin(); // just add add to allow user login or registration
                break;
            }
            case 3:
                // Enhanced exit message
                cout << "\n" << "\033[93m" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
                cout << padLeft("║") << "\033[91m" << centerText("👋 GOODBYE!") << "\033[93m" << "  ║" << "\n";
                cout << padLeft("║") << "\033[92m" << centerText("Thank you for using our system!") << "\033[93m" << "║" << "\n";
                cout << padLeft("║") << "\033[96m" << centerText("System shutting down safely...") << "\033[93m" << "║" << "\n";
                cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n\n";
                break;
            default:
                cout << "\n    " << "\033[91m\033[1m" << "                                   ⚠️  INVALID CHOICE!" << "\033[0m" << "\n";
                cout << "    " << "\033[93m" << "                                   Please select a number between 1 and 3 only." << "\033[0m" << "\n";
                break;
        }
    } while (choice != 3);
}

//add color laoding

void SetColor(int color); // Function prototype

void SetColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void loadingAnimation() {
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
    const int barWidth = 30;
    // Show starting message
    // Animation loop
    for (int i = 0; i <= barWidth; ++i) {
        // Set color based on progress
        if (i < barWidth / 3)
            SetColor(9);  // Light Blue
        else if (i < 2 * barWidth / 3)
            SetColor(14); // Yellow
        else
            SetColor(10); // Green

        // Draw progress bar
        cout << padLeft("\r                                       Loading: ");
        for (int j = 0; j < i; ++j) cout << "▓";  // Filled block
        for (int j = i; j < barWidth; ++j) cout << " ";
        cout << " ";

        // Force output to update instantly
        cout.flush();
        Sleep(30);
    }

    // Reset color
    SetColor(7);
}

void backupStockDataWithCSV() {
    system("cls");
    
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

    // cout << "\033[35m\033[1m"; // Magenta and bold
    // cout << padLeft(" ____             _                 ____        _        ") << "\n";
    // cout << padLeft("|  _ \\           | |               |  _ \\      | |       ") << "\n";
    // cout << padLeft("| |_) | __ _  ___| | ___   _ _ __   | | | | __ _| |_ __ _ ") << "\n";
    // cout << padLeft("|  _ < / _` |/ __| |/ / | | | '_ \\  | | | |/ _` | __/ _` |") << "\n";
    // cout << padLeft("| |_) | (_| | (__|   <| |_| | |_) | | |_| | (_| | || (_| |") << "\n";
    // cout << padLeft("|____/ \\__,_|\\___|_|\\_\\\\__,_| .__/  |____/ \\__,_|\\__\\__,_|") << "\n";
    // cout << padLeft("                            | |                         ") << "\n";
    // cout << padLeft("                            |_|                         ") << "\n";
    // cout << "\033[0m\n"; // Reset colors and add spacing


    SetColor(14); // Bright Yellow

    // Enhanced backup design with dynamic centering
    cout << "\033[93m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("💾 BACKUP STOCK DATA TO FILES 💾") << "\033[93m" << "    ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[94m" << centerText("Creating secure backup of your inventory data...") << "\033[93m" << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[92m" << "📂 Excel Format (.xlsx) - For system compatibility" << "\033[93m" << string(22, ' ') << " ║\n";
    cout << padLeft("║         ") << "\033[92m" << "📊 CSV Format (.csv) - For easy data access" << "\033[93m" << string(28, ' ') << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    try {
        // Step 1: Create backup directory
        cout << "\n" << padLeft("\033[96m🔧 Step 1: Setting up backup directory");
        for(int i = 0; i < 3; i++) {
            cout << " ▓";
            cout.flush();
            Sleep(200);
        }
        cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
        cout << "        ";          // Print spaces to erase
        cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start    
        cout << "\n";
        string backupDir = "backup data";
        bool dirCreated = false;
        if (!fs::exists(backupDir)) {
            fs::create_directories(backupDir);
            dirCreated = true;
        }

        if (dirCreated) {
            cout << padLeft("\033[92m✅ Backup directory created: ") << backupDir << "\033[0m\n";
        } else {
            cout << padLeft("\033[94m📁 Using existing backup directory: ") << backupDir << "\033[0m\n";
        }

        // Step 2: Generate timestamp
        cout << "\n" << padLeft("\033[96m🕒 Step 2: Generating timestamp");
        for(int i = 0; i < 2; i++) {
            cout << " ▓";
            cout.flush();
            Sleep(200);
        }
        cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
        cout << "        ";          // Print spaces to erase
        cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start  
        cout << "\n";
        time_t now = time(nullptr);
        char buf[50];
        strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", localtime(&now));
        
        // Display timestamp info
        char readableBuf[100];
        strftime(readableBuf, sizeof(readableBuf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        cout << padLeft("\033[94m📅 Backup timestamp: ") << readableBuf << "\033[0m\n";

        // Step 3: Create Excel backup
        cout << "\n" << padLeft("\033[96m📊 Step 3: Creating Excel backup (.xlsx)");
        for(int i = 0; i < 4; i++) {
            cout << " ▓";
            cout.flush();
            Sleep(250);
        }
        cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
        cout << "        ";          // Print spaces to erase
        cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start  
        cout << "\n";
        string backupFile = backupDir + "/stock_backup_" + buf + ".xlsx";
        ExcelUtil::writeStockToFile(backupFile, stocks);
        
        cout << padLeft("\033[92m✅ Excel backup created: ") << "stock_backup_" << buf << ".xlsx\033[0m\n";

        // Step 4: Create CSV backup
        cout << "\n" << padLeft("\033[96m📋 Step 4: Creating CSV backup (.csv)");
        for(int i = 0; i < 4; i++) {
            cout << " ▓";
            cout.flush();
            Sleep(250);
        }
        cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
        cout << "        ";          // Print spaces to erase
        cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start  
        cout << "\n";
        string csvBackupFile = backupDir + "/stock_backup_" + buf + ".csv";
        ofstream csvFile(csvBackupFile);
        
        if (csvFile.is_open()) {
            // Write CSV header
            csvFile << "ID,Name,Quantity,Price\n";
            
            // Write stock data with progress indication
            int totalItems = stocks.size();
            int processed = 0;
            
            for (const auto& stock : stocks) {
                csvFile << stock.getId() << ","
                       << "\"" << stock.getName() << "\"," 
                       << stock.getQuantity() << ","
                       << fixed << setprecision(2) << stock.getPrice() << "\n";
                processed++;
            }
            csvFile.close();
            
            cout << padLeft("\033[92m✅ CSV backup created: ") << "stock_backup_" << buf << ".csv\033[0m\n";
            cout << padLeft("\033[94m📈 Records processed: ") << totalItems << " items\033[0m\n";
        } else {
            throw runtime_error("Could not create CSV backup file");
        }

        // Final processing
        cout << "\n" << padLeft("\033[96m🔄 Finalizing backup process");
        for(int i = 0; i < 3; i++) {
            cout << " ▓";
            cout.flush();
            Sleep(200);
        }
        cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
        cout << "        ";          // Print spaces to erase
        cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start  
        cout << "\n";
        // Success message with detailed backup summary
        cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("🎉 BACKUP OPERATION COMPLETED SUCCESSFULLY! 🎉") << "\033[92m" << "    ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[95m" << centerText("📋 BACKUP SUMMARY REPORT") << "\033[92m" << "  ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[96m" << "📂 Backup Directory:" << "\033[97m" << string(53, ' ') << "\033[92m║\n";
        cout << padLeft("\033[92m║          ") << "\033[94m" << "└─ " + backupDir + "/" << "\033[92m" << string(tableWidth - 13 - backupDir.length() - 1, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[96m" << "📊 Excel File (.xlsx):" << "\033[97m" << string(51, ' ') << "\033[92m║\n";
        cout << padLeft("\033[92m║          ") << "\033[94m" << "└─ stock_backup_" + string(buf) + ".xlsx" << "\033[92m" << string(tableWidth - 18 - 13 - strlen(buf), ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[96m" << "📋 CSV File (.csv):" << "\033[97m" << string(54, ' ') << "\033[92m║\n";
        cout << padLeft("\033[92m║          ") << "\033[94m" << "└─ stock_backup_" + string(buf) + ".csv" << "\033[92m" << string(tableWidth - 18 - 12 - strlen(buf), ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║          ") << "\033[96m" << "📈 Backup Statistics:" << "\033[97m" << string(51, ' ') << "\033[92m║\n";
        cout << padLeft("\033[92m║           ") << "\033[94m" << "└─ Total Items: " + to_string(stocks.size()) + " products" << "\033[92m" << string(tableWidth - 18 - 15 - to_string(stocks.size()).length() - 9, ' ') << "      ║\n";
        
        // Calculate total inventory value for summary
        double totalValue = 0;
        int totalQuantity = 0;
        for (const auto& stock : stocks) {
            totalValue += stock.getPrice() * stock.getQuantity();
            totalQuantity += stock.getQuantity();
        }
        
        cout << padLeft("║           ") << "\033[94m" << "└─ Total Quantity: " + to_string(totalQuantity) + " units" << "\033[92m" << string(tableWidth - 18 - 18 - to_string(totalQuantity).length() - 6, ' ') << "      ║\n";
        cout << padLeft("║           ") << "\033[94m" << "└─ Total Value: $" + to_string((int)(totalValue * 100) / 100.0) << "\033[92m" << string(tableWidth - 18 - 15 - to_string((int)(totalValue * 100) / 100.0).length(), ' ') << "     ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[96m" << "⏰ Backup Timestamp:" << "\033[97m" << string(53, ' ') << "\033[92m║\n";
        cout << padLeft("\033[92m║           ") << "\033[94m" << "└─ " + string(readableBuf) << "\033[92m" << string(tableWidth - 14 - strlen(readableBuf), ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("✅ Your inventory data has been safely backed up!") << "\033[92m" << " ║\n";
        cout << padLeft("║") << "\033[97m" << centerText("Both Excel and CSV formats are available for data recovery") << "\033[92m" << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        
    }
    catch (const exception& e) {
        // Enhanced error handling with styled error message
        cout << "\n" << padLeft("\033[91m╔═══════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << "                  ❌ BACKUP FAILED!                       " << "\033[91m" << "║" << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        
        string errorMsg = string(e.what());
        if (errorMsg.length() > 50) {
            errorMsg = errorMsg.substr(0, 47) + "...";
        }
        cout << padLeft("║") << "\033[96m" << centerText("Error: " + errorMsg) << "\033[91m" << "║" << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[97m" << centerText("Please check write permissions") << "\033[91m" << "║" << "\n";
        cout << padLeft("║") << "\033[97m" << centerText("in the backup directory") << "\033[91m" << "║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
    }

    // Footer prompt
    cout << "\n" << padLeft("\033[94m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << "\033[96m" << centerText("Press any key to continue...") << "\033[94m" << "║" << "\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
    
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get(); // Wait for user input
}

// ─── Admin Login ────────────────────────────────────────────────
void adminLogin() {
    system("cls");
    string username, password;
    SetColor(9);

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

    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[93m" << centerText("🔑 ADMIN LOGIN  🔑") << "\033[96m" << "    ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║   ") << "\033[94m" << "👤 Username:" << "\033[96m" << string(67, ' ') << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║   ") << "\033[94m" << "🔒 Password:" << "\033[96m" << string(67, ' ') << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\n";
    cout << "\033[0m";

    // Move cursor into the table (row, col positions will depend on your console size)
    gotoxy(55,7);  // Adjust Y for Username line
    getline(cin, username);

    gotoxy(55, 10); // Adjust Y for Password line
    password = getPasswordInput("");

    for (const auto& user : users) {
        if (user.getUsername() == username && user.getPassword() == password && user.isAdmin()) {
            gotoxy(55, 14);
            cout << "\n" << padLeft("🔐 Authenticating credentials");
            loadingAnimation();  // Show loading animation after successful login
            adminDashboard();
            return;
        }
    }
    
    // Error message with enhanced styling
    gotoxy(55, 13);
    cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << "\033[93m" << centerText("❌ ACCESS DENIED!") << "\033[91m" << " ║\n";
    cout << padLeft("║") << "\033[96m" << centerText("Invalid credentials or not an admin.") << "\033[91m" << "║\n";
    cout << padLeft("║") << "\033[94m" << centerText("Please check your login details.") << "\033[91m" << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << padLeft("\033[93m⚠️  Press any key to try again...\033[0m") << "\n";
    gotoxy(72, 19); // Move cursor to a specific position 
    // cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
    return;
}

// ─── Admin Dashboard ────────────────────────────────────────────
void adminDashboard() {
    system("cls");
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
    int choice;
    do {
        system("cls");
        // Enhanced admin dashboard design with consistent styling
        cout << "\033[96m\033[1m";
        cout << "\n                                       ╔══════════════════════════════════════════════════════════════════════════════════╗\n";
        cout << "                                       ║" << "\033[93m" << "                         🎯 ADMIN DASHBOARD PANEL 🎯                              " << "\033[96m" << "║\n";
        cout << "                                       ╠══════════════════════════════════════════════════════════════════════════════════╣\n";
        cout << "                                       ║         " << "\033[94m" << "[1] 📦 Add Stock" << "\033[96m" << "                                                         ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Add new products to inventory" << "\033[96m" << "                                     ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║         " << "\033[94m" << "[2] 🔄 Update Stock" << "\033[96m" << "                                                      ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Modify existing product details" << "\033[96m" << "                                   ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║         " << "\033[94m" << "[3] 🗑️  Delete Stock" << "\033[96m" << "                                                      ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Remove products from inventory" << "\033[96m" << "                                    ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║         " << "\033[94m" << "[4] 🔍 Search Stock" << "\033[96m" << "                                                      ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Find specific products quickly" << "\033[96m" << "                                    ║\n";
        cout << "                                       ║                                                                                  ║\n";      
        cout << "                                       ║         " << "\033[94m" << "[5] 📋 Display All Stocks" << "\033[96m" << "                                                ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ View complete inventory list" << "\033[96m" << "                                      ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║         " << "\033[94m" << "[6] 📊 Track Inventory" << "\033[96m" << "                                                   ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Monitor stock levels and trends" << "\033[96m" << "                                   ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║         " << "\033[94m" << "[7] ⚠️  Generate Low-Stock Alerts" << "\033[96m" << "                                         ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Check products running low" << "\033[96m" << "                                        ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║         " << "\033[94m" << "[8] 📄 Print Reports" << "\033[96m" << "                                                     ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Generate detailed inventory reports" << "\033[96m" << "                               ║\n";
        cout << "                                       ║                                                                                  ║\n"; 
        cout << "                                       ║         " << "\033[94m" << "[9] 💾 Backup All Data" << "\033[96m" << "                                                   ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Save inventory data to backup file" << "\033[96m" << "                                ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ║         " << "\033[94m" << "[10] 👤❌ Delete User" << "\033[96m" << "                                                    ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Remove user accounts from system" << "\033[96m" << "                                  ║\n";
        cout << "                                       ║                                                                                  ║\n";                                       
        cout << "                                       ║         " << "\033[94m" << "[11] 🚪 Logout" << "\033[96m" << "                                                           ║\n";
        cout << "                                       ║             " << "\033[92m" << "└─ Exit admin panel safely" << "\033[96m" << "                                           ║\n";
        cout << "                                       ║                                                                                  ║\n";
        cout << "                                       ╚══════════════════════════════════════════════════════════════════════════════════╝\n";
        cout << "\033[0m";
        
        // Enhanced input prompt with styling
        cout << "\n" << "\033[95m\033[1m" << "                                       💭 Enter your choice ▶ " << "\033[0m";
        
        if (!(cin >> choice)) {  
            cin.clear(); // clear fail state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input            
            // Enhanced error message
            cout << "\n    " << "\033[91m" << "                                   ❌ Invalid input! Please enter a number between 1 and 11." << "\033[0m" << "\n";
            _getch(); // wait for user to acknowledge
            continue; // skip to next loop iteration
        }

        switch(choice) {
            case 1:{
                // Loading message for add stock
                cout << "\n    " << "\033[96m" << "                                   📦 Loading Add Stock Module";
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50); // Use usleep(300000) for Linux/Mac
                }
                system("cls");
                addStock();
                cout << "    " << "\033[93m" << "                                   📋 Press any key to go back to the menu..." << "\033[0m" << "\n";
                gotoxy(81, 14); // Move cursor to a specific position 
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
                break;
            }
            case 2:{
                // Loading message for update stock
                cout << "\n    " << "\033[96m" << "                                   🔄 Loading Update Stock Module";
                for(int i = 0; i < 20; i++) {
                    cout << " ▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start            
                system("cls");
                updateStock();
                break;
            }
            case 3:{
                // Loading message for delete stock
                cout << "\n    " << "\033[96m" << "                                   🗑️ Loading Delete Stock Module";
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start                
                system("cls");
                deleteStock();
                break;
            }
            case 4:{
                // Loading message for search stock
                cout << "\n    " << "\033[96m" << "                                   🔍 Loading Search Stock Module";
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start
                
                system("cls");
                searchStock();
                // gotoxy(81, 14); // Move cursor to a specific position 
                // cin.ignore(numeric_limits<streamsize>::max(), '\n');
                // cin.get();
                break;
            }
            case 5:{
                // Loading message for display all stocks
                cout << "\n    " << "\033[96m" << "                                   📋 Loading Stock Display Module";
                for(int i = 0; i < 20; i++) {
                    cout << " ▓";
                    cout.flush();
                    Sleep(50);
                } 
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start
               
                system("cls");
                displayAllStocks();
                    // Interactive prompt
                cout << "    " << "\033[93m" << "                                   📋 Press any key to go back to the menu..." << "\033[0m" << "\n";
                gotoxy(81, 13); // Move cursor to a specific position 
                _getch();
                break;
            }
            case 6:{
                // Loading message for track inventory
                cout << "\n    " << "\033[96m" << "                                   📊 Loading Inventory Tracker";
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start
                system("cls");
                trackInventory();
                break;
            }
            case 7:{
                // Loading message for low stock alerts
                cout << "\n    " << "\033[96m" << "                                   ⚠️ Generating Low-Stock Alerts";
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start

                system("cls");
                generateLowStockAlerts(); 
                break;
            }
            case 8:{
                // Loading message for print reports
                cout << "\n    " << "\033[96m" << "                                   📄 Preparing Reports Module";
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start

                system("cls");
                printStockReport();
                break;  
            }
            case 9: {
                // Loading message for backup
                cout << "\n    " << "\033[96m" << "                                   💾 Initiating Data Backup";
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\033[0m" << "\n";
                backupStockDataWithCSV();
                break;
            }
            case 10:{
                // Loading message for delete user
                cout << "\n    " << "\033[96m" << "                                   👤 Loading User Management";
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start

                system("cls");
                deleteUser(); // Call the new function
                break;
            }
            case 11:{
                system("cls");
                currentUser = nullptr;
                break;
            }
            default:
                // Enhanced error for invalid choice
                cout << "\n    " << "\033[91m\033[1m" << "                                   ⚠️  INVALID CHOICE!" << "\033[0m" << "\n";
                cout << "    " << "\033[93m" << "                                   Please select a number between 1 and 11 only." << "\033[0m" << "\n";
                _getch(); // Wait for user input
                break;
        }
    } while (choice != 11);
}

// ─── Add Stock ──────────────────────────────────────────────────
void addStock() {
    system("cls");
    string name;
    int quantity;
    double price;

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

    // ASCII Art Header
    cout << "\033[34m"; // Blue
    cout << "                                                   __    ____  ____     _  _  ____  _    _    ___  ____  _____  ___  _  _     \n";
    cout << "                                                  /__\\  (  _ \\(  _ \\   ( \\( )( ___)( \\/\\/ )  / __)(_  _)(  _  )/ __)( )/ )    \n";
    cout << "                                                 /(__)\\  )(_) ))(_) )   )  (  )__)  )    (   \\__ \\  )(   )_)(( (__  )  (     \n";
    cout << "                                                (__)(__)(____/(____/   (_)_/)(____)(__\\/__)  (___/ (__) (_____)(___)(_)\_)    \n";
    cout << "\033[0m"; // Reset

    SetColor(9);

    // Enhanced add stock design with dynamic centering
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("Add new products to your inventory 📦") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[94m" << centerText("Please fill in the product details below:") << "\033[96m" << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[92m" << "📝 Product Name:" << "\033[96m" << string(54, ' ') << "   ║\n";
    cout << padLeft("║           ") << "\033[97m" << "└─ Enter the name of the product" << "\033[96m" << string(33, ' ') << "      ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[92m" << "📊 Quantity:" << "\033[96m" << string(60, ' ') << " ║\n";
    cout << padLeft("║           ") << "\033[97m" << "└─ Enter number of items to add" << "\033[96m" << string(33, ' ') << "       ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[92m" << "💰 Price:" << "\033[96m" << string(63, ' ') << " ║\n";
    cout << padLeft("║           ") << "\033[97m" << "└─ Enter price per unit ($)" << "\033[96m" << string(37, ' ') << "       ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";
     // Use gotoxy for input fields (adjust X and Y as needed for your box)
     gotoxy(66, 13);
    while (true) {
        getline(cin >> ws, name); // eat whitespace then read full line
        if (!name.empty()) break;
        cout << "\n" << padLeft("\033[91m❌ Invalid input! Product name cannot be empty ▶ \033[0m");
    }

        gotoxy(62, 16);
    while (!(cin >> quantity) || quantity < 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        gotoxy(39, 23);
        cout <<"\033[91m❌ Invalid input! Quantity must be a positive number ▶ \033[0m";
    }
        gotoxy(58, 19);
    while (!(cin >> price) || price < 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        gotoxy(39, 25);
        cout <<"\033[91m❌ Invalid input! Price must be a positive number ▶ \033[0m";
    }


    // Processing animation
    cout << "\n ";
            gotoxy(39, 23);
    cout << "\n" << padLeft("\033[96m📦 Adding product to inventory");
    for(int i = 0; i < 3; i++) {
        cout << " ▓";
        cout.flush();
        Sleep(300); // Use usleep(300000) for Linux/Mac
    }
    cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
    cout << "        ";          // Print spaces to erase
    cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start

    int newId = ExcelUtil::getNextStockId(stocks);
    stocks.emplace_back(newId, name, quantity, price);
    ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);

    // Success message
    cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║")  << "\033[93m" << centerText("✅ STOCK ADDED SUCCESSFULLY!") << "\033[92m" << " ║\n";
    cout << padLeft("║")  << "\033[96m" << centerText("Product has been added to inventory")  << "\033[92m" << "║\n";
    {
        string productText = "Product ID: " + to_string(newId);
        cout << padLeft("║")  << "\033[94m" << centerText(productText) << "\033[92m" << "║\n";
    }
    cout << padLeft("\033[92m╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
}
// ─── Update Stock ───────────────────────────────────────────────
void updateStock() {
    system("cls");
    int id;
    int choice;
    
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

    string asciiArt[] = {
        " _    _           _       _          _____ _             _    ",
        "| |  | |         | |     | |        / ____| |           | |   ",
        "| |  | |_ __   __| | __ _| |_ ___  | (___ | |_ ___   ___| | __",
        "| |  | | '_ \\ / _` |/ _` | __/ _ \\  \\___ \\| __/ _ \\ / __| |/ /",
        "| |__| | |_) | (_| | (_| | ||  __/  ____) | || (_) | (__|   < ",
        " \\____/| .__/ \\__,_|\\__,_|\\__\\___| |_____/ \\__\\___/ \\___|_|\\_\\",
        "       | |                                                   ",
        "       |_|                                                   "
    };

    cout << "\n";
    for (const string& line : asciiArt) {
        cout << padLeft(" " + centerText(line) + " ") << endl;
    }
    cout << "\n";
    // Enhanced update stock design with dynamic centering
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("Modify existing products in your inventory 📝") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[94m" << centerText("Enter the Product ID to update:") << "\033[96m" << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[92m" << "🔍 Product ID:" << "\033[96m" << string(57, ' ') << "  ║\n";
    cout << padLeft("║           ") << "\033[97m" << "└─ Enter the ID number of the product" << "\033[96m" << string(28, ' ') << "      ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << " \n ";
    cout << "\033[0m";

    // Enhanced input prompt
    // gotoxy(64,13); 
    // cin >> id;
    gotoxy(64,19); 
    if (!(cin >> id)) {
        cin.clear(); // clear error flag
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard wrong input
        gotoxy(64,25); 
        cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << centerText("❌ NOT FOUND!") << "\033[91m" << " ║" << "\n";
        cout << padLeft("║") << "\033[94m" << centerText("Invalid input! Please enter a numeric ID.") << "\033[91m" << "║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
        cout << "\n" << padLeft("\033[93m⚠️  Press any key to try again...\033[0m") << "\n";
        gotoxy(72, 30); // Move cursor to a specific position
        _getch(); // Wait for user input
        return;
    }
    // Find the stock item by ID
    auto it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s) {
        return s.getId() == id;
    });

    if (it != stocks.end()) {
        // Product found - show options menu
        gotoxy(64, 24); // Move cursor to a specific position
        cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << centerText("✅ PRODUCT FOUND!") << "\033[92m" << " ║\n";
        printf("%s║%s              Product: %-25s    %s                              ║%s\n", 
            string(leftPadding, ' ').c_str(), "\033[96m", 
            it->getName().length() > 25 ? (it->getName().substr(0, 22) + "...").c_str() : it->getName().c_str(), 
            "\033[92m", "\033[0m");
        cout << padLeft("\033[92m╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        // Options menu
        cout << "\033[96m\033[1m";
        cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("📝 UPDATE OPTIONS") << "\033[96m" << "  ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[94m" << "[1] 📝 Update Name" << "\033[96m" << string(55, ' ') << "║\n";
        cout << padLeft("║             ") << "\033[92m" << "└─ Change the product name" << "\033[96m" << string(43, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[94m" << "[2] 📊 Update Quantity" << "\033[96m" << string(51, ' ') << "║\n";
        cout << padLeft("║             ") << "\033[92m" << "└─ Modify stock quantity" << "\033[96m" << string(45, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[94m" << "[3] 💰 Update Price" << "\033[96m" << string(54, ' ') << "║\n";
        cout << padLeft("║             ") << "\033[92m" << "└─ Change the unit price" << "\033[96m" << string(45, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
        cout << "\033[0m";

        cout << "\n" << padLeft("\033[95m\033[1m💭 Enter your choice ▶ \033[0m");
        
        if (!(cin >> choice)) {  
            cin.clear(); // clear fail state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input            
        }

        string newName;
        int newQuantity;
        double newPrice;

        bool updateName = false, updateQuantity = false, updatePrice = false;

        switch (choice) {
            case 1: {
                system("cls");
                cout << "\n" << padLeft("\033[96m\033[1m") << "\n";
                cout << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
                cout << padLeft("║") << "\033[93m" << centerText("📝 UPDATE PRODUCT NAME") << "\033[96m" << "  ║\n";
                cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";

                cout << "\n" << padLeft("\033[92m\033[1m📝 Enter new name ▶ \033[0m");
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                getline(cin, newName);
                it->setName(newName);
                updateName = true;

                // Ask for quantity
                char yn;
                cout << "\n" << padLeft("\033[95mDo you want to update quantity? (Y/N) ▶ \033[0m");
                cin >> yn;
                if (yn == 'Y' || yn == 'y') {
                    choice = 2; // force go to case 2
                }else {
                    break;
                }
            }
            case 2: {
                system("cls");
                cout << "\n" << padLeft("\033[96m\033[1m") << "\n";
                cout << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
                cout << padLeft("║") << "\033[93m" << centerText("📊 UPDATE QUANTITY") << "\033[96m" << "  ║\n";
                cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";

                cout << "\n" << padLeft("\033[92m\033[1m📊 Enter new quantity ▶ \033[0m");
                while (!(cin >> newQuantity) || newQuantity < 0) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << padLeft("\033[91m❌ Invalid input! Quantity must be a positive number ▶ \033[0m");
                }
                it->setQuantity(newQuantity);
                updateQuantity = true;

                // Ask for price
                char yn;
                cout << "\n" << padLeft("\033[95mDo you want to update price? (Y/N) ▶ \033[0m");
                cin >> yn;
                if (yn == 'Y' || yn == 'y') {
                    choice = 3; // force go to case 3
                } else {
                    break; // exit updateStock
                }
            }
            case 3: {
                system("cls");
                cout << "\n" << padLeft("\033[96m\033[1m") << "\n";
                cout << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
                cout << padLeft("║") << "\033[93m" << centerText("💰 UPDATE PRICE") << "\033[96m" << "  ║\n";
                cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";

                cout << "\n" << padLeft("\033[92m\033[1m💰 Enter new price ▶ \033[0m");
                while (!(cin >> newPrice) || newPrice < 0) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << padLeft("\033[91m❌ Invalid input! Price must be a positive number ▶ \033[0m");
                }
                it->setPrice(newPrice);
                updatePrice = true;
                break;
            }
            default: {
                gotoxy(0, 39); // Move cursor to a specific position
                cout << "\n" << padLeft("\033[91m❌ Invalid choice! Please enter a number between 1 and 3.\033[0m") << "\n";
                cout << padLeft("\033[93m⚠️  Press any key to go back to the menu...\033[0m") << "\n";
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cin.get();
                return adminDashboard();
            }
        }


        // Processing animation        
        ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);      
        // Success message
        cout << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << centerText("✅ UPDATE SUCCESSFUL!") << "\033[92m" << " ║" << "\n";
        cout << padLeft("║") << "\033[96m" <<  centerText("Product has been updated successfully") << "\033[92m" << "║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        cout << "\n" << padLeft("\033[93m⚠️  Press any key to go back to the menu...\033[0m") << "\n";
        gotoxy(72, 19); // Move cursor to a specific position
        _getch(); // wait for user input
        return; // exit updateStock and go back to menu
    } else {
        // Product not found
        gotoxy(0, 17); 
        cout << padLeft("\033[93m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║\033[93m") << centerText("❌ NOT FOUND!") << "\033[93m ║\n"; 
        printf("%s║%s                            Product ID %3d not found                %s              \033[93m║%s\n",
            string(leftPadding, ' ').c_str(), "\033[96m", id, "\033[91m", "\033[0m");
        cout << padLeft("\033[93m║") << "\033[94m" << centerText("Please check the ID and try again") << "\033[93m║\n";
        cout << padLeft("\033[93m╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        cout << "\n" << padLeft("\033[93m⚠️  Press any key to go back to the menu...\033[0m") << "\n";
        gotoxy(82, 23); // Move cursor to a specific position
        _getch(); // wait for user input
    }
}

// ─── Delete Stock ───────────────────────────────────────────────
void deleteStock() {
    system("cls");
    int id;
    char confirmation;
    
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

    // ASCII Art Header
    cout << "\033[34m"; // Blue
    cout << "                                                 ____  ____  __    ____  ____  ____    ___  ____  _____  ___  _  _   \n";
    cout << "                                                (  _ \\( ___)(  )  ( ___)(_  _)( ___)  / __)(_  _)(  _  )/ __)( )/ )  \n";
    cout << "                                                 )(_) ))__)  )(__  )__)   )(   )__)   \\__ \\  )(   )_)(( (__  )  (   \n";
    cout << "                                                (____/(____)(____)(____) (__) (____)  (___/ (__) (_____)(___)(_)\_)  \n";
    cout << "\033[0m"; // Reset

    SetColor(9);

    displayAllStocks(); 
    // Enhanced delete stock design with dynamic centering
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("Remove products from your inventory 🗑️") << "\033[96m" << "      ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[91m" << centerText("⚠️  WARNING: This action cannot be undone!") << "\033[96m" << "     ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[92m" << centerText("🔍 Delete Product By ID:") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << "\033[97m" << "                              └─ Enter the ID of the product to delete" << "\033[96m" << string(12, ' ') << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    // Enhanced input prompt2
    // gotoxy(63,30); 
    // cout << "\n" << padLeft("\033[95m\033[1m💭 Enter Product ID ▶ \033[0m");
    gotoxy(92,27);
    cin >> id;

    if(cin.fail()) {
        cin.clear(); // clear fail state
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
        gotoxy(55, 31); // Move cursor to a specific position
        cout << "\n" << padLeft("\033[91m❌ Invalid input! Please enter a numeric ID.\033[0m") << "\n";
        cout << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
        _getch(); // wait for user input
        cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
        cout << "        ";          // Print spaces to erase
        cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start  
        return; // exit deleteStock and go back to menu
    }
    // Search animation 
    gotoxy(0, 30);
    cout << "\n" << padLeft("\033[96m🔍 Searching for product");
    for(int i = 0; i < 3; i++) {
        cout << " ▓";
        cout.flush();
        Sleep(200);
    }
    cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
    cout << "        ";          // Print spaces to erase
    cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start  

    // Find the stock item by ID
    auto it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s) {
        return s.getId() == id;
    });

    if (it != stocks.end()) {
        // Product found - show details and confirmation
        cout << "\n" << padLeft("\033[93m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[91m" << centerText("⚠️  PRODUCT FOUND!") << "\033[93m" << "     ║" << "\n";
        cout << padLeft("║") << "\033[96m" << centerText("This product will be deleted:") << "\033[93m" << "║" << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        printf("%s\033[93m║%s                             ID: %3d  │  Name: %-25s      %s    ║%s\n", 
               string(leftPadding, ' ').c_str(), "\033[94m", it->getId(),
               it->getName().length() > 25 ? (it->getName().substr(0, 22) + "...").c_str() : it->getName().c_str(),
               "\033[93m", "\033[0m");
        printf("%s\033[93m║%s                       Quantity: %4d │  Price: $%-8.2f                   %s      ║%s\n", 
               string(leftPadding, ' ').c_str(), "\033[94m", it->getQuantity(), it->getPrice(),
               "\033[93m", "\033[0m");
        cout << padLeft("\033[93m╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";

        // Confirmation prompt
        cout << "\033[96m\033[1m";
        cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[91m" << centerText("🗑️ CONFIRM DELETION") << "\033[96m" << "      ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("Are you sure you want to delete this product?") << "\033[96m" << "║\n";
        cout << padLeft("║") << "\033[94m" << centerText("This action is permanent and cannot be undone.") << "\033[96m" << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[92m" << "[Y] Yes, delete this product" << "\033[96m" << string(45, ' ') << "║\n";
        cout << padLeft("║         ") << "\033[91m" << "[N] No, keep this product" << "\033[96m" << string(48, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
        cout << "\033[0m";

        cout << "\n" << padLeft("\033[95m\033[1m💭 Enter your choice (Y/N) ▶ \033[0m");
        cin >> confirmation;
        
        if (confirmation == 'Y' || confirmation == 'y') {
            // Processing deletion animation
            cout << "\n" << padLeft("\033[91m🗑️ Deleting product from inventory");
            for(int i = 0; i < 4; i++) {
                cout << " ▓";
                cout.flush();
                Sleep(400);
            }
            cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
            cout << "        ";          // Print spaces to erase
            cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start
            stocks.erase(it);
            ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
        
            cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << "\033[93m" << centerText ( "✅ DELETION SUCCESSFUL!") << "\033[91m" << " ║" << "\n";
            cout << padLeft("║") << "\033[96m" << centerText ("Product has been removed from inventory") << "\033[91m" << "║" << "\n";
            printf("%s║%s                                 Product ID: %3d                    %s              ║%s\n", 
                   string(leftPadding, ' ').c_str(), "\033[94m", id, "\033[91m", "\033[0m");
            cout << padLeft("\033[91m╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
            // cout << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
            // cin.get();
        } else {
            // Cancellation message
            cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << "\033[93m" << centerText ("❌ DELETION CANCELLED") << "\033[92m" << " ║" << "\n";
            cout << padLeft("║") << "\033[96m" << centerText ( "Product has been kept safely in inventory") << "\033[92m" << "║" << "\n";
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        }

    } else {
        // Product not found
        cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" <<centerText ( "❌ NOT FOUND!") << "\033[91m" << " ║" << "\n";
        printf("%s║%s                              Product ID %3d not found                %s            ║%s\n", 
               string(leftPadding, ' ').c_str(), "\033[96m", id, "\033[91m", "\033[0m");
        cout << padLeft("\033[91m║") << "\033[94m" << centerText ("Please check the ID and try again") << "\033[91m" << "║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
    }
    cout << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
    _getch(); // wait for user input 

}
// ─── Search Stock ───────────────────────────────────────────────
void searchStock() {
    system("cls");
    int id;
    
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

    // ASCII Art Header
    cout << "\033[34m";  // Blue
    cout << "                                 ___  ____    __    ____   ___  _   _    ___  ____  _____  ___  _  _    ____  _  _    ____  ____  \n";
    cout << "                                / __)( ___)  /__\\  (  _ \\ / __)( )_( )  / __)(_  _)(  _  )/ __)( )/ )  (  _ \\( \\/ )  (_  _)(  _ \\ \n";
    cout << "                                \\__ \\ )__)  /(__)\\  )   /( (__  ) _ (   \\__ \\  )(   )(_)(( (__  )  (    ) _ < \\  /    _)(_  )(_) )\n";
    cout << "                                (___/(____)(__)(__)(_) \_) \\___)(_) (_)  (___/ (__) (_____) \___)(_)\_)   (____/ (__)   (____)(____/ \n";
    cout << "\033[0m"; 

    SetColor(9);

    // Enhanced search stock design with dynamic centering
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("Find specific products in your inventory 🔍") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[94m" << centerText("Enter the Product ID to search for:") << "\033[96m" << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[92m" << "🔍 Product ID:" << "\033[96m" << string(59, ' ') << "║\n";
    cout << padLeft("║           ") << "\033[97m" << "└─ Enter the ID number of the product" << "\033[96m" << string(34, ' ') << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    // Enhanced input prompt

    gotoxy(64,13); 
    cin >> id;
    if (cin.fail()) {
        cin.clear(); // clear fail state
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
    }
    // Search animation
    gotoxy(64,17);
    cout << "\n" << padLeft("\033[96m🔍 Searching inventory");
    for(int i = 0; i < 4; i++) {
        cout << " ▓";
        cout.flush();
        Sleep(250);
    }
    cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
    cout << "        ";          // Print spaces to erase
    cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start


    
    bool found = false;
    for (const auto& s : stocks) {
        if (s.getId() == id) {
            // Product found - display detailed information
            cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << "\033[93m" << centerText("✅ PRODUCT FOUND!") << "\033[92m" << " ║" << "\n";
            cout << padLeft("║") << "\033[96m" << centerText("Product details displayed below:") << "\033[92m" << "║" << "\n";
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";

            // Product details table
            cout << "\033[96m\033[1m";
            cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << "\033[93m" << centerText("📋 PRODUCT DETAILS") << "\033[96m" << "  ║\n";
            cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
            cout << padLeft("║") << centerText("") << "║\n";
            
            printf("%s║%s         🆔 Product ID: %-3d                                    %s                   ║%s\n", 
                   string(leftPadding, ' ').c_str(), "\033[94m", s.getId(), "\033[96m", "\033[0m");
            cout << padLeft("\033[96m\033[1m║") << centerText("") << "║\n";
            
            printf("%s║%s         📦 Product Name: %-35s    %s                 ║%s\n", 
                   string(leftPadding, ' ').c_str(), "\033[94m", 
                   s.getName().length() > 35 ? (s.getName().substr(0, 32) + "...").c_str() : s.getName().c_str(), 
                   "\033[96m", "\033[0m");
            cout << padLeft("\033[96m\033[1m║") << centerText("") << "║\n";
            
            printf("%s\033[96m\033[1m║%s         📊 Stock Quantity: %-6d                            %s                    ║%s\n", 
                   string(leftPadding, ' ').c_str(), "\033[94m", s.getQuantity(), "\033[96m", "\033[0m");
            cout << padLeft("\033[96m\033[1m║") << centerText("") << "║\n";
            
            printf("%s\033[96m\033[1m║%s         💰 Unit Price: $%-8.2f                            %s                     ║%s\n", 
                   string(leftPadding, ' ').c_str(), "\033[94m", s.getPrice(), "\033[96m", "\033[0m");
            cout << padLeft("\033[96m\033[1m║") << centerText("") << "║\n";
            
            printf("%s\033[96m\033[1m║%s         💵 Total Value: $%-8.2f                           %s                     ║%s\n", 
                   string(leftPadding, ' ').c_str(), "\033[94m", (s.getPrice() * s.getQuantity()), "\033[96m", "\033[0m");
            cout << padLeft("\033[96m\033[1m║") << centerText("") << "║\n";
            
            // Stock status indicator
            string stockStatus;
            string statusColor;
            if (s.getQuantity() == 0) {
                stockStatus = "⚠️  OUT OF STOCK";
                statusColor = "\033[91m"; // Red
            } else if (s.getQuantity() < 10) {
                stockStatus = "⚠️  LOW STOCK";
                statusColor = "\033[93m"; // Yellow
            } else {
                stockStatus = "✅ IN STOCK";
                statusColor = "\033[92m"; // Green
            }
            
            printf("%s\033[96m\033[1m║%s         📈 Stock Status: %s%-15s%s                 %s                         ║%s\n", 
                   string(leftPadding, ' ').c_str(), "\033[94m", statusColor.c_str(), stockStatus.c_str(), 
                   "\033[94m", "\033[96m", "\033[0m");
            cout << padLeft("\033[96m\033[1m║") << centerText("") << "║\n";
            cout << padLeft("\033[96m\033[1m╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
            cout << "\033[0m";

            found = true;
            break; // Since ID is unique, break early
        }
    }

    if (!found) {
            // Product not found - Organized table
        cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("❌ PRODUCT NOT FOUND!") << "\033[91m" << " ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[96m" << centerText("🔍 Search Results:") << "\033[91m" << "  ║\n";
        cout << padLeft("║                            ") << "\033[94m" << "• Product ID: " << "\033[97m" << id << "\033[91m" << string(40 - to_string(id).length(), ' ') << "║\n";
        cout << padLeft("║                            ") << "\033[94m" << "• Status: " << "\033[97m" << "NOT FOUND IN DATABASE" << "\033[91m" << string(23, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[95m" << centerText("💡 Please verify the Product ID and try again") << "\033[91m" << "  ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        // Suggestion box
        cout << "\n" << padLeft("\033[93m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[96m" << centerText("💡 SUGGESTIONS:") << "\033[93m" << "  ║" << "\n";
        cout << padLeft("║") << "\033[94m" << centerText("• Check if the Product ID is correct") << "\033[93m" << "  ║" << "\n";
        cout << padLeft("║") << "\033[94m" << centerText("• Use 'Display All Stocks' to see all IDs") << "\033[93m" << "  ║" << "\n";
        cout << padLeft("║") << "\033[94m" <<  centerText("• Make sure the product exists in inventory") << "\033[93m" << "  ║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
    }
    cout << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
    gotoxy(83, 39); // Move cursor to a specific position
    _getch();
}

// ─── Display All Stocks ─────────────────────────────────────────
void displayAllStocks() {
    SetColor(9);
    DisplayUtil::displayStocks(stocks);
}
void trackInventory() {
    system("cls");
    
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

    cout << "\033[34m\033[1m"; // Blue and bold
    cout << padLeft("  _______             _      _____                      _                   ") << "\n";
    cout << padLeft(" |__   __|           | |    |_   _|                    | |                  ") << "\n";
    cout << padLeft("    | |_ __ __ _  ___| | __   | |  _ ____   _____ _ __ | |_ ___  _ __ _   _ ") << "\n";
    cout << padLeft("    | | '__/ _` |/ __| |/ /   | | | '_ \\ \\ / / _ \\ '_ \\| __/ _ \\| '__| | | |") << "\n";
    cout << padLeft("    | | | | (_| | (__|   <   _| |_| | | \\ V /  __/ | | | || (_) | |  | |_| |") << "\n";
    cout << padLeft("    |_|_|  \\__,_|\\___|_|\\_\\ |_____|_| |_|\\_/ \\___|_| |_|\\__\\___/|_|   \\__, |") << "\n";
    cout << padLeft("                                                                       __/ |") << "\n";
    cout << padLeft("                                                                      |___/ ") << "\n";
    cout << "\033[0m\n"; // Reset colors and add spacing

    SetColor(9);

    // Enhanced track inventory design with dynamic centering
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("Monitor your inventory status and stock levels 📊") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    // Loading animation
    cout << "\n" << padLeft("\033[96m📊 Analyzing inventory data");
    for(int i = 0; i < 4; i++) {
        cout << " ▓";
        cout.flush();
        Sleep(300);
    }
    cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
    cout << "        ";          // Print spaces to erase
    cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start 

    if (stocks.empty()) {
        // Empty inventory message
        cout << "\n" << padLeft("\033[91m╔═══════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << "                  📦 INVENTORY EMPTY                   " << "\033[91m" << "  ║" << "\n";
        cout << padLeft("║") << "\033[96m" << "            No stock items to track currently.           " << "\033[91m" << "     ║" << "\n";
        cout << padLeft("║") << "\033[94m" << "              Please add products first.                 " << "\033[91m" << "     ║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        
        cout << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
        cin.get();
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

    // TABLE 1: Inventory Statistics
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << "\033[93m" << centerText("📊 INVENTORY STATISTICS") << "\033[96m" << "  ║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    printf("%s║  📦 Total Unique Products: %-6d      │   📊 Total Items: %-8lld              ║%s\n", 
           string(leftPadding, ' ').c_str(), totalUniqueItems, totalQuantity, "\033[0m");
    printf("%s\033[96m║  💰 \033[96mTotal Inventory Value: $%-10.2f │   ⚠️  Low Stock Threshold: %-3d           \033[96m║\033[0m%s\n", 
           string(leftPadding, ' ').c_str(), totalPrice, LOW_STOCK_THRESHOLD, "\033[0m");
    cout << "\033[96m";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";
    // TABLE 2: Low Stock Items
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    printf("%s║%s                      🚨 LOW STOCK ALERTS (< %d items)                      %s      ║%s\n", 
           string(leftPadding, ' ').c_str(), "\033[91m", LOW_STOCK_THRESHOLD, "\033[96m", "\033[0m");
    cout << "\033[96m";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    
    if (lowStockItems.empty()) {
        cout << padLeft("║") << "\033[92m" << centerText("✅ ALL STOCK LEVELS OK! No items running low. 🎉") << "\033[96m" << "   ║\n";
    } else {
        cout << padLeft("║  ID  │         Product Name        │  Stock  │   Price   │   Total Value         ║\n");
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << "\033[0m";

        SetColor(11); // Light red for low stock items
        for (const auto& item : lowStockItems) {
            double itemValue = item.getPrice() * item.getQuantity();
            printf("%s\033[96m║ \033[91m%3d  │ %-28s│   %3d   │  $%7.2f │    $%8.2f          \033[96m║%s\n",
                   string(leftPadding, ' ').c_str(),
                   item.getId(),
                   item.getName().length() > 28 ? (item.getName().substr(0, 25) + "...").c_str() : item.getName().c_str(),
                   item.getQuantity(),
                   item.getPrice(),
                   itemValue,
                   "\033[0m");
        }
        cout << "\033[96m";
    }
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";
    
    cout << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}

// ─── Stub Functions ─────────────────────────────────────────────

//Function Generate Low Stock Alerts
void generateLowStockAlerts() {
    system("cls");
    
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

    SetColor(9);

    // Enhanced low stock alerts design with dynamic centering
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[93m" << centerText("⚠️ LOW STOCK ALERTS ⚠️") << "\033[96m" << "          ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("Monitor items that need immediate restocking 📦") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    // Scanning animation
    cout << "\n" << padLeft("\033[96m🔍 Scanning inventory for low stock items");
    for(int i = 0; i < 4; i++) {
        cout << " ▓";
        cout.flush();
        Sleep(300);
    }
    cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
    cout << "        ";          // Print spaces to erase
    cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start 
    vector<Stock> LowStockItems;
    
    for (const auto& stock : stocks){
        if (stock.getQuantity() < LOW_STOCK_THRESHOLD){
            LowStockItems.push_back(stock);
        }
    }

    // TABLE 1: Alert Summary
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << "\033[93m" << centerText("📊 ALERT SUMMARY") << "\033[96m" << "  ║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    printf("%s║  📦 Total Products Scanned: %-6d  │  ⚠️  Low Stock Threshold: %-3d               ║%s\n", 
           string(leftPadding, ' ').c_str(), static_cast<int>(stocks.size()), LOW_STOCK_THRESHOLD, "\033[0m");
    printf("%s\033[96m║  🚨 Items Below Threshold: %-7d  │  📈 Alert Status: %-15s          ║%s\n", 
           string(leftPadding, ' ').c_str(), static_cast<int>(LowStockItems.size()),
           LowStockItems.empty() ? "ALL CLEAR" : "ACTION NEEDED", "\033[0m");
    cout << "\033[96m";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    // TABLE 2: Low Stock Items
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    printf("%s║%s                         🚨 LOW STOCK ITEMS DETAILS                         %s      ║%s\n", 
           string(leftPadding, ' ').c_str(), "\033[91m", "\033[96m", "\033[0m");
    cout << "\033[96m";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    
    if (LowStockItems.empty()){
        cout << padLeft("║") << "\033[92m" << centerText("✅ NO LOW STOCK ITEMS FOUND!") << "\033[96m" << " ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[94m" << centerText("All products are well-stocked above the threshold.") << "\033[96m" << "║\n";
        cout << padLeft("║") << "\033[95m" << centerText("Great inventory management! 🎉") << "\033[96m" << "  ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
    } else {
        cout << padLeft("║  ID  │         Product Name         │  Stock  │   Price   │   Status             ║\n");
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << "\033[0m";

        SetColor(1); // Light red for low stock items
        for (const auto& item : LowStockItems){
            string status;
            if (item.getQuantity() == 0) {
                status = "OUT OF STOCK";
            } else if (item.getQuantity() <= LOW_STOCK_THRESHOLD / 2) {
                status = "CRITICAL LOW";
            } else {
                status = "LOW STOCK";
            }
            
            printf("%s\033[96m║ \033[91m%3d  │ %-28s │   %3d   │ $%7.2f  │ %-13s        \033[96m║%s\n",
                   string(leftPadding, ' ').c_str(),
                   item.getId(),
                   item.getName().length() > 28 ? (item.getName().substr(0, 25) + "...").c_str() : item.getName().c_str(),
                   item.getQuantity(),
                   item.getPrice(),
                   status.c_str(),
                   "\033[0m");
        }
        cout << "\033[96m";
    }
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";
    
    cout << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cin.get();
}
void userRegister() {
    system("cls");
    string username, password;

    // Draw registration table
    cout << "\033[96m\033[1m";
    cout << "\n                                       ╔══════════════════════════════════════════════════════════════════════════════════╗\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ║" << "\033[93m" << "                        📝 USER REGISTRATION PORTAL 📝                           "
         << "\033[96m" << " ║\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ╠══════════════════════════════════════════════════════════════════════════════════╣\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ║   " << "\033[94m" << "👤 New Username:" << "\033[96m" << "                                                               ║\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ║   " << "\033[94m" << "🔑 New Password:" << "\033[96m" << "                                                               ║\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ╚══════════════════════════════════════════════════════════════════════════════════╝\n";
    cout << "      ";
    cout << "\033[0m";

    // Move cursor inside table to Username input position
    gotoxy(59, 7); // adjust (x,y) so it's aligned inside the Username field
    getline(cin, username);

    // Move cursor inside table to Password input position
    gotoxy(59, 9); // adjust (x,y) so it's aligned inside the Password field
    getline(cin, password);

    // Check if username already exists
    bool userExists = false;
    for (const auto& user : users) {
        if (user.getUsername() == username) {
            userExists = true;
            break;
        }
    }

    if (userExists) {
        gotoxy(5, 13);
        cout << "\033[31m                                  ❌ Username already exists. Please choose a different one.\033[0m" << endl;
    } else {
        users.emplace_back(username, password, false);
        ExcelUtil::writeUsersToFile("data/users.xlsx", users);
        gotoxy(5, 13);
        cout << "\033[32m                                  ✅User registered successfully!\033[0m" << endl;
    }
}
// Function for user login and registration menu
void userLogin() { // just add add to enable user to login or register
    system("cls");
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
    int choice;
    
    // Enhanced user menu design with consistent styling
    cout << "\033[96m\033[1m";
    cout << "\n                                       ╔══════════════════════════════════════════════════════════════════════════════════╗\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ║" << "\033[93m" << "                           🎯 USER ACCESS PORTAL 🎯                               " << "\033[96m" << "║\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ╠══════════════════════════════════════════════════════════════════════════════════╣\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ║         " << "\033[94m" << "[1] 📝 Register" << "\033[96m" << "                                                          ║\n";
    cout << "                                       ║             " << "\033[92m" << "└─ Create a new user account" << "\033[96m" << "                                         ║\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ║         " << "\033[94m" << "[2] 🔑 Login" << "\033[96m" << "                                                             ║\n";
    cout << "                                       ║             " << "\033[92m" << "└─ Access your existing account" << "\033[96m" << "                                      ║\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ║         " << "\033[94m" << "[3] ⬅️  Back to Main Menu" << "\033[96m" << "                                                 ║\n";
    cout << "                                       ║             " << "\033[92m" << "└─ Return to previous menu" << "\033[96m" << "                                           ║\n";
    cout << "                                       ║                                                                                  ║\n";
    cout << "                                       ╚══════════════════════════════════════════════════════════════════════════════════╝\n";
    cout << "\033[0m";
    
    // Enhanced input prompt with styling
    cout << "\n" << "\033[95m\033[1m" << "                                       💭 Enter your choice ▶ " << "\033[0m";
    
        if (!(cin >> choice)) {  
        cin.clear(); // clear fail state
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
        
        // Enhanced error message
        cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << centerText("⚠️  INVALID CHOICE!") << "\033[91m" << "     ║\n";
        cout << padLeft("║") << "\033[96m" << centerText("Please select a number between 1 and 3 only.") << "\033[91m" << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        return; 
    }
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    switch (choice) {
        case 1: {
            // Loading message for registration
            cout << "\n" << padLeft("\033[96m📝 Loading Registration Portal");
            for(int i = 0; i < 3; i++) {
                cout << "▓";
                cout.flush();
                Sleep(200);
            }
            
            system("cls");
            userRegister();
            break;
        }
        case 2:{
            // Loading message for login
            cout << "\n" << padLeft("\033[96m🔑 Accessing Login Portal");
            for(int i = 0; i < 7; i++) {
                cout << "▓";
                cout.flush();
                Sleep(250);
            }
            
            system("cls");
            string username, password;
            
            cout << "\033[96m\033[1m";
            cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << centerText("") << "║\n";
            cout << padLeft("║") << "\033[93m" << centerText("🔑 USER LOGIN PORTAL 🔑") << "\033[96m" << "    ║\n";
            cout << padLeft("║") << centerText("") << "║\n";
            cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
            cout << padLeft("║") << centerText("") << "║\n";
            cout << padLeft("║   ") << "\033[94m" << "👤 Username:" << "\033[96m" << string(67, ' ') << "║\n";
            cout << padLeft("║") << centerText("") << "║\n";
            cout << padLeft("║") << centerText("") << "║\n";
            cout << padLeft("║   ") << "\033[94m" << "🔒 Password:" << "\033[96m" << string(67, ' ') << "║\n";
            cout << padLeft("║") << centerText("") << "║\n";
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
            cout << "\033[0m";

            // Move cursor into the table
            gotoxy(55, 7);  // Username line
            getline(cin, username);

            gotoxy(55, 10); // Password line
            password = getPasswordInput("");

            bool loginSuccess = false;
            for (auto& user : users) {
                if (user.getUsername() == username && user.getPassword() == password && !user.isAdmin()) {
                    currentUser = &user;
                    loginSuccess = true;
                    break;
                }
            }
            
            if (loginSuccess) {
                // Enhanced success message
                gotoxy(5, 13);
                cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
                cout << padLeft("║") << "\033[93m" << centerText("✅ LOGIN SUCCESSFUL!") << "\033[92m" << " ║\n"; 
                cout << padLeft("║") << "\033[96m" << centerText("Welcome back, " + username + "!") << "\033[92m" << "║\n";
                cout << padLeft("║") << "\033[94m" << centerText("Redirecting to dashboard...") << "\033[92m" << "║\n";
                cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
                
                loadingAnimation();  // Show loading animation after successful login
                staffDashboard();
            } else {
                // Enhanced error message
                gotoxy(0, 15); // Move cursor below the table
                cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
                cout << padLeft("║") << "\033[93m" << centerText("❌ LOGIN FAILED!") << "\033[91m" << " ║\n";
                cout << padLeft("║") << "\033[96m" << centerText("Invalid username or password detected.") << "\033[91m" << "║\n";
                cout << padLeft("║") << "\033[94m" << centerText("Please check credentials or use admin login.") << "\033[91m" << "║\n";
                cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
                cout << padLeft("\033[93m⚠️  Press any key to go back to the menu...\033[0m") << "\n";
                _getch();
            }
            break;
        }
        case 3:{
            // Enhanced back to menu message
            cout << "\n" << padLeft("\033[93m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << "\033[96m" << centerText("⬅️  RETURNING TO MENU") << "\033[93m" << "     ║\n";
            cout << padLeft("║") << "\033[92m" << centerText("Navigating back to main menu...") << "\033[93m" << "║\n";
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
            
            // Loading animation for navigation
            cout << "\n" << padLeft("\033[96m🔄 Loading Main Menu");
            for(int i = 0; i < 3; i++) {
                cout << "▓";
                cout.flush();
                Sleep(250);
            }
            
            system("cls");
            break;
        }
        default:
            // Enhanced error for invalid choice
            cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << "\033[93m" << centerText("⚠️  INVALID CHOICE!") << "\033[91m" << "║\n";
            cout << padLeft("║") << "\033[96m" << centerText("Please select a number between 1 and 3 only.") << "\033[91m" << "║\n";
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
            break;
    }
}
void staffDashboard() { // just add add for allowing staff actions
    system("cls");
    int choice;    
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

    do {
        system("cls");
        
        // Enhanced staff dashboard design with dynamic centering
        cout << "\033[96m\033[1m";
        cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("🏪 USER SHOPPING DASHBOARD 🏪") << "\033[96m" << "    ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[95m" << centerText("Welcome! Enjoy browsing and shopping with us! 🛍️") << "\033[96m" << "      ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[94m" << "[1] 📋 Display All Stocks" << "\033[96m" << string(48, ' ') << "║\n";
        cout << padLeft("║             ") << "\033[92m" << "└─ Browse all available products" << "\033[96m" << string(37, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[94m" << "[2] 🔍 Search Stock" << "\033[96m" << string(54, ' ') << "║\n";
        cout << padLeft("║             ") << "\033[92m" << "└─ Find specific products quickly" << "\033[96m" << string(36, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[94m" << "[3] 🛒 Add Item to Cart" << "\033[96m" << string(50, ' ') << "║\n";
        cout << padLeft("║             ") << "\033[92m" << "└─ Add products to your shopping cart" << "\033[96m" << string(32, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[94m" << "[4] 👀 View Cart" << "\033[96m" << string(57, ' ') << "║\n";
        cout << padLeft("║             ") << "\033[92m" << "└─ Review items in your cart" << "\033[96m" << string(41, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[94m" << "[5] 💳 Checkout Cart" << "\033[96m" << string(53, ' ') << "║\n";
        cout << padLeft("║             ") << "\033[92m" << "└─ Complete your purchase" << "\033[96m" << string(44, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[94m" << "[6] 🚪 Logout" << "\033[96m" << string(60, ' ') << "║\n";
        cout << padLeft("║             ") << "\033[92m" << "└─ Exit shopping session safely" << "\033[96m" << string(38, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
        cout << "\033[0m";
        
        // Enhanced input prompt with dynamic centering
        cout << "\n" << padLeft("\033[95m\033[1m💭 Enter your choice ▶ \033[0m");
        
        if (!(cin >> choice)) {
            cin.clear(); // clear fail state
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); // discard bad input
            
            // Enhanced error message
            cout << "\n" << padLeft("\033[91m\033[1m⚠️  INVALID CHOICE!\033[0m") << "\n";
            cout << padLeft("\033[93mPlease select a number between 1 and 6 only.\033[0m") << "\n";
            return staffDashboard(); // Skip to next iteration for valid input 
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n'); // Clear the input buffer
        switch(choice) {
            case 1:{
                // Loading message for display stocks
                cout << "\n" << padLeft("\033[96m📋 Loading Product Catalog");
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50); // Use usleep(300000) for Linux/Mac
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start       
                system("cls");              
                displayAllStocks();
                // Interactive prompt
                cout << "    " << "\033[93m" << padLeft("⚠️  Press any key to go back to the menu...") << "\033[0m" << "\n";
                _getch(); // Wait for any key press
                break;
            }
            case 2:{
                // Loading message for search stock
                cout << "\n" << padLeft("\033[96m🔍 Loading Search Module");
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start           
                system("cls");
                searchStock();
                break;
            }
            case 3:{
                // Loading message for add to cart
                cout << "\n" << padLeft("\033[96m🛒 Loading Shopping Cart");
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start               
                system("cls");
                addItemToCart(); // Call the new function
                break;
            }
            case 4:{
                // Loading message for view cart
                cout << "\n" << padLeft("\033[96m👀 Loading Cart Viewer");
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start       
                system("cls");
                viewCart(); // Call the new function
                break;
            }
            case 5:{
                // Loading message for checkout
                cout << "\n" << padLeft("\033[96m💳 Processing Checkout");
                for(int i = 0; i < 20; i++) {
                    cout << "▓";
                    cout.flush();
                    Sleep(50);
                }
                cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
                cout << "        ";          // Print spaces to erase
                cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start       
                if (currentUser) {
                    checkoutCart(currentUser->getUsername()); // Pass current user's username
                } else {
                    // Enhanced error message for no user
                    cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
                    cout << padLeft("║") << "\033[93m" << centerText("❌ ERROR!") << "\033[91m" << "║" << "\n";
                    cout << padLeft("║") << "\033[96m" << centerText("No user logged in for checkout.") << "\033[91m" << "║" << "\n";
                    cout << padLeft("║") << "\033[94m" << centerText("Please login first to continue.") << "\033[91m" << "║" << "\n";
                    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
                }
                break;
            }
            case 6:{
                // Enhanced logout message
                cout << "\n" << padLeft("\033[93m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
                cout << padLeft("║") << "\033[91m" << centerText("👋 LOGGING OUT...") << "\033[93m" << "║" << "\n";
                cout << padLeft("║") << "\033[92m" << centerText("Thank you for shopping with us!") << "\033[93m" << "║" << "\n";
                cout << padLeft("║") << "\033[96m" << centerText("Session ended successfully...") << "\033[93m" << "║" << "\n";
                cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n\n";
                
                system("cls");
                currentUser = nullptr; // Clear current user on logout
                break;
            }
            default:{
                // Enhanced error for invalid choice
                cout << "\n" << padLeft("\033[91m\033[1m⚠️  INVALID CHOICE!\033[0m") << "\n";
                cout << padLeft("\033[93mPlease select a number between 1 and 6 only.\033[0m") << "\n";
                break;
            }
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
    system("cls");
    
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


    SetColor(9);

    // Enhanced add to cart design with dynamic centering
    // cout << "\033[96m\033[1m";
    // cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    // cout << padLeft("║") << centerText("") << "║\n";
    // cout << padLeft("║") << "\033[95m" << centerText("🛒 Add Items to Your Shopping Cart 🛒") << "\033[96m" << "    ║\n";
    // cout << padLeft("║") << centerText("") << "║\n";
    // cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    // cout << padLeft("║") << centerText("") << "║\n";
    // cout << padLeft("║") << "\033[94m" << centerText("Available Products:") << "\033[96m" << "║\n";
    // cout << padLeft("║") << centerText("") << "║\n";
    // cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    // cout << "\033[0m";

    // Display available stocks with enhanced styling
    cout << "\n";
    displayAllStocks(); // Show available stocks
    cout << "\n";

    // Input section with enhanced styling
    cout << "\033[96m\033[1m";
    cout << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[94m" << centerText("Please select the product and quantity:") << "\033[96m" << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[92m" << "🏷️  Product ID:" << "\033[96m" << string(56, ' ') << "   ║\n";
    cout << padLeft("║           ") << "\033[97m" << "└─ Enter the ID of the product to add" << "\033[96m" << string(31, ' ') << "   ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[92m" << "📦 Quantity:" << "\033[96m" << string(60, ' ') << " ║\n";
    cout << padLeft("║           ") << "\033[97m" << "└─ Enter desired quantity" << "\033[96m" << string(40, ' ') << "      ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    int id, qty;

    // Get Product ID with input validation and positioning
    gotoxy(64, 19); // Adjust Y coordinate based on your display
    while (!(cin >> id) || id <= 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        gotoxy(39, 27);
        cout << "\033[91m❌ Invalid input! Please enter a valid Product ID ▶ \033[0m";
    }
    // cin.ignore(numeric_limits<streamsize>::max(), '\n');

    // Find the stock item
    auto it = find_if(stocks.begin(), stocks.end(), [id](const Stock& s){return s.getId()==id;});
    if (it == stocks.end()) {
        gotoxy(20, 27);
        cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << "                             ❌ PRODUCT NOT FOUND!                           " << "\033[91m" << "     ║" << "\n";
        cout << padLeft("║") << "\033[96m" <<  centerText("The specified Product ID does not exist") << "\033[91m" << "║" << "\n";
        cout << padLeft("║") << "\033[97m" <<  centerText("Please check and try again") << "\033[91m" << "║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        return;
    }

    // Display selected product info
        gotoxy(20,27);
    cout << "\n" << padLeft("\033[93m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << "\033[96m" << "                             📦 PRODUCT SELECTED                   " << "\033[93m" << "               ║" << "\n";
    string productInfo = "Product: " + it->getName() + " | Price: $" + to_string((int)(it->getPrice() * 100) / 100.0);
    string stockInfo = "Available Stock: " + to_string(it->getQuantity()) + " items";
    cout << padLeft("║") << "\033[97m" << centerText(productInfo) << "\033[93m" << "║" << "\n";
    cout << padLeft("║") << "\033[97m" << centerText(stockInfo) << "\033[93m" << "║" << "\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";

    // Get quantity with positioning
    gotoxy(62, 22); // Adjust Y coordinate based on your display
    while (!(cin >> qty) || qty <= 0) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        gotoxy(39, 32);
        cout << "\033[91m❌ Invalid input! Quantity must be a positive number ▶ \033[0m";
    }
    
    // Validate quantity and process
    if (qty > 0 && qty <= it->getQuantity()) {
        // Check if item already in cart, if so, update quantity
        auto cart_it = find_if(cart.begin(), cart.end(), [id](const pair<Stock, int>& p){
            return p.first.getId() == id;
        });

        if (cart_it != cart.end()) {
            // Item already in cart, update its quantity
            int oldQty = cart_it->second;
            cart_it->second += qty;
            
            // Success message for updated quantity
                gotoxy(20, 32);
            cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << "\033[93m" << centerText("✅ CART UPDATED SUCCESSFULLY!") << "\033[92m" << "║" << "\n";
            cout << padLeft("║") << "\033[96m" << centerText("+ " + to_string(qty) + " more " + it->getName() + " added") << "\033[92m" << "║" << "\n";
            cout << padLeft("║") << "\033[97m" << centerText("Total in cart: " + to_string(cart_it->second) + " items") << "\033[92m" << "║" << "\n";
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        } else {
            // Item not in cart, add new entry
            cart.push_back({*it, qty});
            
            // Success message for new item
            gotoxy(20, 32);
            cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << "\033[93m" << "                               ✅ ITEM ADDED TO CART!                             " << "\033[92m" << "║" << "\n";
            cout << padLeft("║") << "\033[96m" << centerText(to_string(qty) + " x " + it->getName() + " added successfully") << "\033[92m" << "║" << "\n";
            double itemTotal = qty * it->getPrice();
            cout << padLeft("║") << "\033[97m" << centerText("Item Total: $" + to_string((int)(itemTotal * 100) / 100.0)) << "\033[92m" << "║" << "\n";
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        }
    } else {
        // Error message for invalid quantity
        gotoxy(20, 32);
        cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << "                               ❌ INVALID QUANTITY!                     " << "\033[91m" << "          ║" << "\n";
        cout << padLeft("║") << "\033[96m" << centerText("Requested: " + to_string(qty) + " | Available: " + to_string(it->getQuantity())) << "\033[91m" << "║" << "\n";
        cout << padLeft("║") << "\033[97m" << centerText("Please enter a valid quantity") << "\033[91m" << "║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
    }

    // Footer prompt
    cout << "\n" << padLeft("\033[94m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << "\033[96m" << centerText("Press any key to go back to the menu...") << "\033[94m" << "║" << "\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
    _getch(); // Wait for any key press
}
// Function to view the contents of the global cart

void printStockReport() {
    system("cls");
    
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

    // ASCII Art Header
    cout << "\033[35m"; // Magenta
    cout << "                                              ____  ____  ____   ___   ____  ____     ____  ____  ____   ___   ____  ____ \n";
    cout << "                                             / ___)(_  _)(  _ \\ / __) (  _ \\(_  _)   (  _ \\(  __)(  _ \\ / __) (  _ \\(_  _)\n";
    cout << "                                             \\___ \\  )(   )(_) ( (__   )   /  )(      )   / ) _)  )___/( (__   )   /  )( \n";
    cout << "                                             (____/ (__) (____/ \\___) (_)\\_) (__)    (_)\\_)(____)(__)   \\___) (_)\\_) (__)\n";
    cout << "\033[0m"; // Reset

    // Current Date & Time
    time_t now = time(nullptr);
    char dateBuf[100];
    strftime(dateBuf, sizeof(dateBuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    SetColor(11); // Bright Cyan

    // Report Header with enhanced styling
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[93m" << centerText("📊 INVENTORY STOCK SUMMARY REPORT 📊") << "\033[96m" << "    ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[94m" << centerText("Generated on: " + string(dateBuf)) << "\033[96m" << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";

    // Stats Setup
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

    // Summary Statistics Section
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("📈 INVENTORY OVERVIEW") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    
    // Format summary data with proper spacing
    string totalItemsStr = "📦 Total Items: " + to_string(totalItems);
    string totalQuantityStr = "📊 Total Quantity: " + to_string(totalQuantity);
    string totalValueStr = "💰 Inventory Value: $" + to_string((int)(totalValue * 100) / 100.0);
    
    cout << padLeft("║         ") << "\033[92m" << totalItemsStr << "\033[96m" << string(tableWidth - 9 - totalItemsStr.length(), ' ') << "  ║\n";
    cout << padLeft("║         ") << "\033[92m" << totalQuantityStr << "\033[96m" << string(tableWidth - 9 - totalQuantityStr.length(), ' ') << "  ║\n";
    cout << padLeft("║         ") << "\033[92m" << totalValueStr << "\033[96m" << string(tableWidth - 9 - totalValueStr.length(), ' ') << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    
    // Product Statistics Section
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("🏆 PRODUCT HIGHLIGHTS") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";

    if (mostExpensive) {
        string expensiveStr = "💎 Most Expensive: " + mostExpensive->getName() + " ($" + to_string((int)(mostExpensive->getPrice() * 100) / 100.0) + ")";
        cout << padLeft("║         ") << "\033[93m" << expensiveStr << "\033[96m" << string(tableWidth - 9 - expensiveStr.length(), ' ') << "  ║\n";
    }

    if (leastExpensive) {
        string cheapStr = "💵 Least Expensive: " + leastExpensive->getName() + " ($" + to_string((int)(leastExpensive->getPrice() * 100) / 100.0) + ")";
        cout << padLeft("║         ") << "\033[93m" << cheapStr << "\033[96m" << string(tableWidth - 9 - cheapStr.length(), ' ') << "  ║\n";
    }

    if (mostStocked) {
        string stockedStr = "📈 Most Stocked: " + mostStocked->getName() + " (" + to_string(mostStocked->getQuantity()) + " items)";
        cout << padLeft("║         ") << "\033[93m" << stockedStr << "\033[96m" << string(tableWidth - 9 - stockedStr.length(), ' ') << "  ║\n";
    }

    if (leastStocked) {
        string lowStockStr = "📉 Least Stocked: " + leastStocked->getName() + " (" + to_string(leastStocked->getQuantity()) + " items)";
        cout << padLeft("║         ") << "\033[93m" << lowStockStr << "\033[96m" << string(tableWidth - 9 - lowStockStr.length(), ' ') << "  ║\n";
    }

    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";

    // Recent Transactions Section
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("🧾 RECENT TRANSACTIONS (Last 5)") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";

    if (receipts.empty()) {
        cout << padLeft("║") << "\033[91m" << centerText("No transactions recorded yet.") << "\033[96m" << "║\n";
    } else {
        int count = 0;
        for (auto it = receipts.rbegin(); it != receipts.rend() && count < 5; ++it, ++count) {
            time_t transactionTime = it->getTransactionTime();
            tm ptm{};
            
            string receiptInfo;
            if (localtime_s(&ptm, &transactionTime) == 0) {
                char timeBuf[50];
                strftime(timeBuf, sizeof(timeBuf), "%m/%d %H:%M", &ptm);
                receiptInfo = "🧾 ID:" + to_string(it->getReceiptId()) + " | " + it->getUsername() + " | $" + 
                             to_string((int)(it->getTotalPrice() * 100) / 100.0) + " | " + string(timeBuf);
            } else {
                receiptInfo = "🧾 ID:" + to_string(it->getReceiptId()) + " | " + it->getUsername() + " | $" + 
                             to_string((int)(it->getTotalPrice() * 100) / 100.0) + " | [Invalid Time]";
            }

            // Truncate if too long
            if (receiptInfo.length() > 72) {
                receiptInfo = receiptInfo.substr(0, 69) + "...";
            }
            
            cout << padLeft("║         ") << "\033[94m" << receiptInfo << "\033[96m" << string(tableWidth - 9 - receiptInfo.length(), ' ') << "  ║\n";

            // Show item details (limited to fit in box)
            for (const auto& item : it->getItems()) {
                string itemInfo = "    • " + item.first.getName() + " x" + to_string(item.second) + 
                                 " @ $" + to_string((int)(item.first.getPrice() * 100) / 100.0);
                if (itemInfo.length() > 72) {
                    itemInfo = itemInfo.substr(0, 69) + "...";
                }
                cout << padLeft("║         ") << "\033[97m" << itemInfo << "\033[96m" << string(tableWidth - 9 - itemInfo.length(), ' ') << "  ║\n";
            }
            cout << padLeft("║") << centerText("") << "║\n";
        }

        if (receipts.size() > 5) {
            string totalInfo = "(Total transactions: " + to_string(receipts.size()) + ")";
            cout << padLeft("║") << "\033[90m" << centerText(totalInfo) << "\033[96m" << " ║\n";
            cout << padLeft("║") << centerText("") << " ║\n";
        }
    }

    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m"; // Reset colors

    // Success footer
    cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << "\033[93m" << centerText("📋 REPORT GENERATED SUCCESSFULLY!") << "\033[92m" << "  ║" << "\n";
    cout << padLeft("║") << "\033[96m" <<  centerText("Press any key to go back to the menu...") << "\033[92m" << "║" << "\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
    _getch(); // Wait for user input
}
void viewCart() {
    system("cls");
    string username = (currentUser != nullptr) ? currentUser->getUsername() : " ";
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
    // Check if cart is empty
    if (cart.empty()) {
        cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << centerText("🛒 CART IS EMPTY!") << "\033[91m" << "  ║" << "\n";
        cout << padLeft("║") << "\033[96m" << centerText("Add some products to get started") << "\033[91m" << "║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        cout << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        return;
    }

    // Loading animation
    cout << "\n" << padLeft("\033[96m🔍 Loading your cart items");
    for(int i = 0; i < 3; i++) {
        cout << "▓";
        cout.flush();
        Sleep(200);
    }
    cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
    cout << "        ";          // Print spaces to erase
    cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start       

    double total = 0;

    // Cart items display with enhanced styling
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[93m" << centerText("📦 CART ITEMS") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    // Header row
    cout << padLeft("║") << "\033[94m" << "  PRODUCT NAME" << string(25, ' ') << "QUANTITY" << string(5, ' ') << "UNIT PRICE" << string(5, ' ') << "SUBTOTAL" << string(7, ' ') << "\033[96m║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";

    // Display each cart item
    cout << fixed << setprecision(2);
    for (auto &c : cart) {
        double itemTotal = c.first.getPrice() * c.second;
        string productName = c.first.getName();
        
        // Truncate long product names
        if (productName.length() > 30) {
            productName = productName.substr(0, 27) + "...";
        }
        
        cout << padLeft("║") << "\033[92m  " << productName << string(37- productName.length(), ' ') 
             << "\033[93m" << setw(3) << c.second << string(10, ' ')
             << "\033[95m$" << setw(8) << c.first.getPrice() << string(7, ' ')
             << "\033[97m$" << setw(8) << itemTotal << string(5, ' ') << "\033[96m║\n";
        
        total += itemTotal;
    }

    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    
    // Total section with enhanced styling
    cout << padLeft("║") << "\033[93m" << "  TOTAL AMOUNT: " << "\033[92m\033[1m$" << setw(10) << total << "\033[96m" << string(55, ' ') << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    // Action buttons section
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[93m" << centerText("🎯 QUICK ACTIONS") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[94m" << "[1] 🛍️  Continue Shopping" << "\033[96m" << string(48, ' ') << " ║\n";
    cout << padLeft("║             ") << "\033[92m" << "└─ Add more items to your cart" << "\033[96m" << string(38, ' ') << " ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[94m" << "[2] 💳 Proceed to Checkout" << "\033[96m" << string(46, ' ') << " ║\n";
    cout << padLeft("║             ") << "\033[92m" << "└─ Complete your purchase" << "\033[96m" << string(43, ' ') << " ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[94m" << "[3] 🗑️  Clear Cart" << "\033[96m" << string(56, ' ') << "║\n";
    cout << padLeft("║             ") << "\033[92m" << "└─ Remove all items from cart" << "\033[96m" << string(40, ' ') << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    cout << "\n" << padLeft("\033[95m\033[1m💭 Enter your choice (or 0 to go back) ▶ \033[0m");
    
    int choice;
    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        choice = 0;
    }

    switch (choice) {
        case 1:
            // Continue shopping logic here
            cout << "\n" << padLeft("\033[96m🛍️ Redirecting to shopping...\033[0m");
            Sleep(1000);
            // Call shopping function here
            return addItemToCart(); // Assuming this function allows adding items to the cart
        case 2:
            // Checkout logic here
            cout << "\n" << padLeft("\033[92m💳 Proceeding to checkout...\033[0m");
            Sleep(1000);
            checkoutCart(username); 
            break; // Exit to allow checkoutCart to be called externally
        case 3:
            // Clear cart logic
            cart.clear();
            cout << "\n" << padLeft("\033[93m🗑️ Cart cleared successfully!\033[0m");
            Sleep(1000);
            break;
        case 0:
            return staffDashboard();    
        default:
            cout << "\n" << padLeft("\033[91m❌ Invalid choice! Please try again.\033[0m");
            cout << "\n" << padLeft("\033[93m📋 Press any key to continue menu...\033[0m");
            cin.get();
            Sleep(1500);
            break;
    }

}

void checkoutCart(const string& username) {
    system("cls");
    
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


    // Enhanced checkout cart design with dynamic centering
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("🛒 Complete your purchase and finalize your order 💳") << "\033[96m" << "    ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    // Check if cart is empty
    if (cart.empty()) {
        cout << "\n" << padLeft("\033[91m╔═══════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << "                   🛒 CART EMPTY!                    " << "\033[91m" << "     ║" << "\n";
        cout << padLeft("║") << "\033[94m" << "           Your cart is empty. Nothing to checkout.      " << "\033[91m" << " ║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        cout << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        return;
    }

    // Display cart contents with enhanced styling
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[93m" << centerText("🛒 YOUR SHOPPING CART") << "\033[96m" << "  ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    // Table header
    printf("%s║ %s%-4s │ %-30s │ %-8s │ %-10s │ %-10s%s       ║%s\n",
           string(leftPadding, ' ').c_str(), "\033[94m\033[1m",
           "ID", "PRODUCT NAME", "QUANTITY", "UNIT PRICE", "TOTAL",
           "\033[96m", "\033[0m");
    
    // cout << padLeft("║") << "\033[94m" << string(4, '─') << "┼" << string(30, '─') << "┼" 
    //      << string(8, '─') << "┼" << string(10, '─') << "┼" << string(10, '─') << "\033[96m" << string(15, ' ') << " ║\n";
    cout << padLeft("\033[96m╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";

    double cartTotal = 0.0;
    for (const auto& cart_item : cart) {
        double itemTotal = cart_item.first.getPrice() * cart_item.second;
        cartTotal += itemTotal;
        
        printf("%s║ %s%-4d%s │ %s%-30s%s │ %s%-8d%s │ %s$%-9.2f%s │ %s$%-9.2f%s       ║%s\n",
               string(leftPadding, ' ').c_str(), 
               "\033[97m", cart_item.first.getId(), "\033[96m",
               "\033[94m", 
               cart_item.first.getName().length() > 30 ? 
               (cart_item.first.getName().substr(0, 27) + "...").c_str() : 
               cart_item.first.getName().c_str(), "\033[96m",
               "\033[92m", cart_item.second, "\033[96m",
               "\033[93m", cart_item.first.getPrice(), "\033[96m",
               "\033[91m", itemTotal, "\033[96m", "\033[0m");
    }

    cout << padLeft("\033[96m║") << centerText("") << "║\n";
    cout << padLeft("\033[96m╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    printf("%s\033[96m║%s                             %sGRAND TOTAL: %s$%-10.2f%s                             ║%s\n",
           string(leftPadding, ' ').c_str(), "\033[96m", "\033[93m\033[1m", 
           "\033[91m\033[1m", cartTotal, "\033[96m", "\033[0m");
    cout << padLeft("\033[96m╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    // Confirmation prompt with enhanced styling
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[93m" << centerText("💳 PURCHASE CONFIRMATION") << "\033[95m" << "  \033[96m║\n";
    cout << padLeft("\033[96m║") << centerText("") << "║\n";
    cout << padLeft("\033[96m╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("\033[96m║") << centerText("") << "║\n";
    cout << padLeft("\033[96m║         ") << "\033[94m" << "Confirm purchase of items in cart? " << "\033[95m" << string(30, ' ') << "        \033[96m║\n";
    cout << padLeft("\033[96m║") << centerText("") << "║\n";
    cout << padLeft("\033[96m║           ") << "\033[92m" << "[Y] ✅ Yes, proceed with checkout" << "\033[95m" << string(36, ' ') << "  \033[96m║\n";
    cout << padLeft("\033[96m║           ") << "\033[91m" << "[N] ❌ No, cancel checkout" << "\033[95m" << string(43, ' ') << "  \033[96m║\n";
    cout << padLeft("\033[96m║") << centerText("") << "║\n";
    cout << padLeft("\033[96m╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    cout << "\n" << padLeft("\033[95m\033[1m💭 Enter your choice (Y/N) ▶ \033[0m");
    
    char confirm;
    cin >> confirm;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    if (tolower(confirm) != 'y') {
        cout << "\n" << padLeft("\033[93m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[96m" << centerText("🚫 CHECKOUT CANCELLED") << "\033[93m" << "  ║" << "\n";
        cout << padLeft("║") << "\033[94m" << centerText("Your cart has been preserved safely") << "\033[93m" << "║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        cout << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
        cin.get();
        return;
    }

    // Processing animation
    cout << "\n" << padLeft("\033[96m💳 Processing your order");
    for(int i = 0; i < 4; i++) {
        cout << "▓";
        cout.flush();
        Sleep(300);
    }
    cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
    cout << "        ";          // Print spaces to erase
    cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start  
    int receiptId = ExcelUtil::getNextReceiptId(receipts);
    vector<pair<Stock, int>> purchasedItemsForReceipt;
    double totalCartPrice = 0.0;
    bool transactionSuccessful = true;
    vector<string> failedItems;

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
                failedItems.push_back(stock_it->getName() + " (Available: " + 
                                    to_string(stock_it->getQuantity()) + ")");
                transactionSuccessful = false;
            }
        } else {
            failedItems.push_back("Item ID " + to_string(id) + " (Not found)");
            transactionSuccessful = false;
        }
    }

    if (transactionSuccessful && !purchasedItemsForReceipt.empty()) {
        // Complete success
        Receipt newReceipt(receiptId, purchasedItemsForReceipt, username);
        receipts.push_back(newReceipt);

        ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
        ExcelUtil::writeTransactionsToFile("data/transactions.xlsx", receipts);

        cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << centerText("✅ CHECKOUT SUCCESSFUL!") << "\033[92m" << " ║" << "\n";
        cout << padLeft("║") << "\033[96m" << centerText("Your purchase has been completed") << "\033[92m" << "║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        
        // Receipt details
        cout << "\033[96m\033[1m";
        cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("🧾 RECEIPT DETAILS") << "\033[96m" << "  ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        
        printf("%s║ %sReceipt ID:%s %-10d                    %sCustomer:%s %-20s%s         ║%s\n",
               string(leftPadding, ' ').c_str(), "\033[94m", "\033[97m", newReceipt.getReceiptId(),
               "\033[94m", "\033[97m", username.c_str(), "\033[96m", "\033[0m");
        
        printf("%s\033[96m║%s %sTOTAL AMOUNT: %s$%-10.2f%s                                                        ║%s\n",
               string(leftPadding, ' ').c_str(), "\033[96m", "\033[93m\033[1m", 
               "\033[91m\033[1m", newReceipt.getTotalPrice(), "\033[96m", "\033[0m");
        
        cout << padLeft("\033[96m╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
        cout << "\033[0m";

        cart.clear();
        
    } else if (!purchasedItemsForReceipt.empty()) {
        // Partial success
        Receipt newReceipt(receiptId, purchasedItemsForReceipt, username);
        receipts.push_back(newReceipt);

        ExcelUtil::writeStockToFile("data/stock.xlsx", stocks);
        ExcelUtil::writeTransactionsToFile("data/transactions.xlsx", receipts);

        cout << "\n" << padLeft("\033[93m╔═══════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[96m" << "              ⚠️  PARTIAL CHECKOUT                   " << "\033[93m" << "     ║" << "\n";
        cout << padLeft("║") << "\033[94m" << "         Some items purchased, others failed            " << "\033[93m" << "  ║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";

        // Show failed items
        if (!failedItems.empty()) {
            cout << "\033[91m\033[1m";
            cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << centerText("") << "║\n";
            cout << padLeft("║") << "\033[93m" << centerText("❌ FAILED ITEMS") << "\033[91m" << "  ║\n";
            cout << padLeft("║") << centerText("") << "║\n";
            cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
            
            for (const auto& failedItem : failedItems) {
                printf("%s║ %s• %-70s%s     ║%s\n",
                       string(leftPadding, ' ').c_str(), "\033[94m",
                       failedItem.length() > 70 ? (failedItem.substr(0, 67) + "...").c_str() : failedItem.c_str(),
                       "\033[91m", "\033[0m");
            }
            
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
            cout << "\033[0m";
        }

        // Show successful receipt
        cout << "\033[96m\033[1m";
        cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("🧾 PARTIAL RECEIPT") << "\033[96m" << "  ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        
        printf("%s║ %sReceipt ID:%s %-10d                    %sCustomer:%s %-20s%s      ║%s\n",
               string(leftPadding, ' ').c_str(), "\033[94m", "\033[97m", newReceipt.getReceiptId(),
               "\033[94m", "\033[97m", username.c_str(), "\033[96m", "\033[0m");
        
        cout << padLeft("║") << centerText("") << "║\n";
        printf("%s║%s                           %sTOTAL PAID: %s$%-10.2f%s                          ║%s\n",
               string(leftPadding, ' ').c_str(), "\033[96m", "\033[93m\033[1m", 
               "\033[91m\033[1m", newReceipt.getTotalPrice(), "\033[96m", "\033[0m");
        
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
        cout << "\033[0m";

        cart.clear();
        
    } else {
        // Complete failure
        cout << "\n" << padLeft("\033[91m╔═══════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << "\033[93m" << "                ❌ CHECKOUT FAILED!                 " << "\033[91m" << "     ║" << "\n";
        cout << padLeft("║") << "\033[94m" << "          No items could be successfully purchased       " << "\033[91m" << "║" << "\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";

        // Show failed items
        if (!failedItems.empty()) {
            cout << "\033[91m\033[1m";
            cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << centerText("") << "║\n";
            cout << padLeft("║") << "\033[93m" << centerText("⚠️  ISSUES ENCOUNTERED") << "\033[91m" << "  ║\n";
            cout << padLeft("║") << centerText("") << "║\n";
            cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
            
            for (const auto& failedItem : failedItems) {
                printf("%s║ %s• %-70s%s     ║%s\n",
                       string(leftPadding, ' ').c_str(), "\033[94m",
                       failedItem.length() > 70 ? (failedItem.substr(0, 67) + "...").c_str() : failedItem.c_str(),
                       "\033[91m", "\033[0m");
            }
            
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
            cout << "\033[0m";
        }

        cart.clear();
    }

    cout << "\n" << padLeft("\033[93m📋 Press any key to go back to the menu...\033[0m") << "\n";
    cin.get();
}
void deleteUser() {

    system("cls");
    string username;
    
    // Dynamic centering setup
    const int tableWidth = 82;
    const int terminalWidth = 164;
    int leftPadding = (terminalWidth - (tableWidth + 4)) / 2;

    auto padLeft = [&](const string& text) {
        return string(leftPadding, ' ') + text;
    };
    
    auto centerText = [&](const string& text) {
        int spacesLeft = (tableWidth - text.size()) / 2;
        int spacesRight = tableWidth - text.size() - spacesLeft;
        return string(spacesLeft, ' ') + text + string(spacesRight, ' ');
    };

    string asciiArt[] = {
        " _____       _      _           _              _                 ",
        "|  __ \\     | |    | |         | |            | |                ",
        "| |  | | ___| | ___| |_ ___  __| | ___   ___  | | ___   ___  ___ ",
        "| |  | |/ _ \\ |/ _ \\ __/ _ \\/ _` |/ _ \\ / _ \\ | |/ _ \\ / _ \\/ __|",
        "| |__| |  __/ |  __/ ||  __/ (_| | (_) |  __/ | | (_) |  __/\\__ \\",
        "|_____/ \\___|_|\\___|\\__\\___|\\__,_|\\___/ \\___| |_|\\___/ \\___||___/"
    };

    cout << "\n";
    for (const string& line : asciiArt) {
        cout << padLeft(" " + centerText(line) + " ") << endl;
    }
    cout << "\n";


    SetColor(9);

    // Enhanced delete user design
    cout << "\033[96m\033[1m";
    cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[95m" << centerText("Remove Users from System 🗑️") << "\033[96m" << "      ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║") << "\033[91m" << centerText("⚠️  WARNING: This action cannot be undone!") << "\033[96m" << "     ║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("║         ") << "\033[92m" << "👤 Username:" << "\033[96m" << string(60, ' ') << " ║\n";
    cout << padLeft("║           ") << "\033[97m" << "└─ Enter the username to delete" << "\033[96m" << string(40, ' ') << "║\n";
    cout << padLeft("║") << centerText("") << "║\n";
    cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
    cout << "\033[0m";

    // Enhanced input prompt
    gotoxy(61, 17);
    cin >> username;

    // Search animation
    gotoxy(55,21);
    cout << "\n" << padLeft("\033[96m🔍 Searching for user");
    for(int i = 0; i < 3; i++) {
        cout << " ▓";
        cout.flush();
        Sleep(200);
    }
    cout << "\033[0m" << "\n";
    cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
    cout << "        ";          // Print spaces to erase
    cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start 
    // Find the user by username
    auto it = find_if(users.begin(), users.end(), [username](const User& u) {
        return u.getUsername() == username;
    });

    if (it != users.end()) {
        // Prevent deletion of admin account
        if (it->isAdmin()) {
            cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << "\033[93m" << centerText("⚠️  ADMIN ACCOUNT PROTECTED") << "\033[91m" << "║" << "\n";
            cout << padLeft("║") << "\033[96m" << centerText("Cannot delete administrator accounts") << "\033[91m" << "║" << "\n";
            cout << padLeft("║") << "\033[94m" << centerText("Please contact system administrator") << "\033[91m" << "║" << "\n";
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
            
            cout << padLeft("\033[93m📋 Press any key to continue...\033[0m") << "\n";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
            return;
        }

        // User found - show details and confirmation
// User found - show details and confirmation - Organized table
        cout << "\033[96m\033[1m";
        cout << "\n" << padLeft("╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[91m" << centerText("🗑️ CONFIRM USER DELETION") << "\033[96m" << "      ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("Are you sure you want to delete this user?") << "\033[96m" << "║\n";
        cout << padLeft("║") << "\033[94m" << centerText("This action is permanent and cannot be undone.") << "\033[96m" << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║         ") << "\033[92m" << "[Y] Yes, delete this user" << "\033[96m" << string(48, ' ') << "║\n";
        cout << padLeft("║         ") << "\033[91m" << "[N] No, keep this user" << "\033[96m" << string(51, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\n";
        cout << "\033[0m";

        cout << "\n" << padLeft("\033[95m\033[1m💭 Enter your choice (Y/N) ▶ \033[0m");
        char confirmation;
        cin >> confirmation;

        if (confirmation == 'Y' || confirmation == 'y') {
            // Processing deletion animation
            cout << "\n" << padLeft("\033[91m🗑️ Deleting user from system");
            for(int i = 0; i < 4; i++) {
                cout << " ▓";
                cout.flush();
                Sleep(400);
            }
            cout << "\033[0m" << "\n";
            cout << "\b\b\b\b\b\b\b\b";  // Go back 8 spaces (4 symbols × 2 bytes each)
            cout << "        ";          // Print spaces to erase
            cout << "\b\b\b\b\b\b\b\b";  // Move cursor back again to the start 
            

            users.erase(it);
            ExcelUtil::writeUsersToFile("data/users.xlsx", users);
            
        // User deleted successfully - Organized table
        cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("✅ USER DELETED SUCCESSFULLY!") << "\033[91m" << " ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[96m" << centerText("🗑️  Deletion Summary:") << "\033[91m" << "      ║\n";
        cout << padLeft("║                        ") << "\033[94m" << "• Username: " << "\033[97m" << username << "\033[91m" << string(45 - username.length(), ' ') << " ║\n";
        cout << padLeft("║                        ") << "\033[94m" << "• Status: " << "\033[97m" << "PERMANENTLY REMOVED" << "\033[91m" << string(29, ' ') << "║\n";
        cout << padLeft("║                        ") << "\033[94m" << "• Action: " << "\033[97m" << "DELETION COMPLETED" << "\033[91m" << string(30, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[95m" << centerText("💡 User account has been removed from system database") << "\033[91m" << "  ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        } else {
            // Cancellation message
            cout << "\n" << padLeft("\033[92m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
            cout << padLeft("║") << "\033[93m" << centerText("❌ DELETION CANCELLED") << "\033[92m" << " ║" << "\n";
            cout << padLeft("║") << "\033[96m" << centerText("User account has been kept in system") << "\033[92m" << "║" << "\n";
            cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        }

    } else {
// User not found - Organized table
        cout << "\n" << padLeft("\033[91m╔══════════════════════════════════════════════════════════════════════════════════╗") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[93m" << centerText("❌ USER NOT FOUND!") << "\033[91m" << " ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╠══════════════════════════════════════════════════════════════════════════════════╣") << "\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[96m" << centerText("🔍 Search Results:") << "\033[91m" << "  ║\n";
        cout << padLeft("║                        ") << "\033[94m" << "• Username: " << "\033[97m" << username << "\033[91m" << string(45 - username.length(), ' ') << " ║\n";
        cout << padLeft("║                        ") << "\033[94m" << "• Status: " << "\033[97m" << "NOT FOUND IN DATABASE" << "\033[91m" << string(27, ' ') << "║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("║") << "\033[95m" << centerText("💡 Please verify the username and try again") << "\033[91m" << "  ║\n";
        cout << padLeft("║") << centerText("") << "║\n";
        cout << padLeft("╚══════════════════════════════════════════════════════════════════════════════════╝") << "\033[0m" << "\n";
        }
    
    cout << padLeft("\033[93m📋 Press any key to continue...\033[0m") << "\n";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    _getch();
}