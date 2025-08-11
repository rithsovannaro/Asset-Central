#include "../include/ExcelUtil.hpp"
#include <iostream>
#include <filesystem> // For creating directories
#include <numeric>
#include <ctime>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

// Helper function to create the data directory if it doesn't exist
void ensureDirectoryExists(const std::string& path) {
    if (!fs::exists(path)) {
        try {
            fs::create_directories(path);
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error creating directory: " << e.what() << std::endl;
            exit(1); 
        }
    }
}

// Helper to create a user Excel file with headers
void ExcelUtil::createUsersFile(const std::string& filename) {
    ensureDirectoryExists("data");
    xlnt::workbook wb;
    xlnt::worksheet ws = wb.active_sheet();
    ws.cell("A1").value("Username");
    ws.cell("B1").value("Password");
    ws.cell("C1").value("IsAdmin");
    wb.save(filename);
}

// Reads user data from the users.xlsx file
std::vector<User> ExcelUtil::readUsersFromFile(const std::string& filename) {
    std::vector<User> users;
    ensureDirectoryExists("data");
    if (!fs::exists(filename)) {
        createUsersFile(filename);
        return users;
    }

    try {
        xlnt::workbook wb;
        wb.load(filename);
        xlnt::worksheet ws = wb.active_sheet();
        for (auto row : ws.rows(false)) {
            if (row[0].to_string() == "Username") continue;
            
            std::string username = row[0].to_string();
            std::string password = row[1].to_string();
            bool isAdmin = row[2].to_string() == "true";
            users.emplace_back(username, password, isAdmin);
        }
    } catch (const xlnt::exception& e) {
        std::cerr << "Error reading users file: " << e.what() << std::endl;
    }
    return users;
}

// Writes user data to the users.xlsx file
void ExcelUtil::writeUsersToFile(const std::string& filename, const std::vector<User>& users) {
    ensureDirectoryExists("data");
    xlnt::workbook wb;
    xlnt::worksheet ws = wb.active_sheet();
    ws.cell("A1").value("Username");
    ws.cell("B1").value("Password");
    ws.cell("C1").value("IsAdmin");
    int row_num = 2;
    for (const auto& user : users) {
        ws.cell("A" + std::to_string(row_num)).value(user.getUsername());
        ws.cell("B" + std::to_string(row_num)).value(user.getPassword());
        ws.cell("C" + std::to_string(row_num)).value(user.isAdmin() ? "true" : "false");
        row_num++;
    }
    wb.save(filename);
}

// Helper to create a stock Excel file with headers
void ExcelUtil::createStockFile(const std::string& filename) {
    ensureDirectoryExists("data");
    xlnt::workbook wb;
    xlnt::worksheet ws = wb.active_sheet();
    ws.cell("A1").value("ID");
    ws.cell("B1").value("Name");
    ws.cell("C1").value("Quantity");
    ws.cell("D1").value("Price");
    wb.save(filename);
}

// Reads stock data from the stock.xlsx file
std::vector<Stock> ExcelUtil::readStockFromFile(const std::string& filename) {
    std::vector<Stock> stocks;
    ensureDirectoryExists("data");
    if (!fs::exists(filename)) {
        createStockFile(filename);
        return stocks;
    }

    try {
        xlnt::workbook wb;
        wb.load(filename);
        xlnt::worksheet ws = wb.active_sheet();
        for (auto row : ws.rows(false)) {
            if (row[0].to_string() == "ID") continue;

            try {
                int id = std::stoi(row[0].to_string());
                std::string name = row[1].to_string();
                int quantity = std::stoi(row[2].to_string());
                double price = std::stod(row[3].to_string());
                stocks.emplace_back(id, name, quantity, price);
            } catch (const std::exception& e) {
                std::cerr << "Skipping malformed row in stock file: " << e.what() << std::endl;
            }
        }
    } catch (const xlnt::exception& e) {
        std::cerr << "Error reading stock file: " << e.what() << std::endl;
    }
    return stocks;
}

// Writes stock data to the stock.xlsx file
void ExcelUtil::writeStockToFile(const std::string& filename, const std::vector<Stock>& stocks) {
    ensureDirectoryExists("data");
    xlnt::workbook wb;
    xlnt::worksheet ws = wb.active_sheet();
    ws.cell("A1").value("ID");
    ws.cell("B1").value("Name");
    ws.cell("C1").value("Quantity");
    ws.cell("D1").value("Price");
    int row_num = 2;
    for (const auto& stock : stocks) {
        ws.cell("A" + std::to_string(row_num)).value(stock.getId());
        ws.cell("B" + std::to_string(row_num)).value(stock.getName());
        ws.cell("C" + std::to_string(row_num)).value(stock.getQuantity());
        ws.cell("D" + std::to_string(row_num)).value(stock.getPrice());
        row_num++;
    }
    wb.save(filename);
}

// Function to find the next available stock ID
int ExcelUtil::getNextStockId(const std::vector<Stock>& stocks) {
    if (stocks.empty()) {
        return 1;
    }
    
    int maxId = 0;
    for (const auto& stock : stocks) {
        if (stock.getId() > maxId) {
            maxId = stock.getId();
        }
    }
    return maxId + 1;
}
void ExcelUtil::createTransactionsFile(const std::string& filename) {
    ensureDirectoryExists("data");
    xlnt::workbook wb;
    xlnt::worksheet ws = wb.active_sheet();
    ws.cell("A1").value("ReceiptID");
    ws.cell("B1").value("Username");
    ws.cell("C1").value("ItemID");
    ws.cell("D1").value("ItemName");
    ws.cell("E1").value("Quantity");
    ws.cell("F1").value("PricePerUnit");
    ws.cell("G1").value("TotalPrice");
    ws.cell("H1").value("TransactionTime");
    wb.save(filename);
}

std::vector<Receipt> ExcelUtil::readTransactionsFromFile(const std::string& filename) {
    std::vector<Receipt> receipts;
    ensureDirectoryExists("data");
    if (!fs::exists(filename)) {
        createTransactionsFile(filename);
        return receipts;
    }

    try {
        xlnt::workbook wb;
        wb.load(filename);
        xlnt::worksheet ws = wb.active_sheet();
        for (auto row : ws.rows(false)) {
            if (row[0].to_string() == "ReceiptID") continue;

            try {
                int receiptId = std::stoi(row[0].to_string());
                std::string username = row[1].to_string();
                int itemId = std::stoi(row[2].to_string());
                std::string itemName = row[3].to_string();
                int quantity = std::stoi(row[4].to_string());
                double pricePerUnit = std::stod(row[5].to_string());

                Stock purchasedItem(itemId, itemName, 0, pricePerUnit);
                std::vector<std::pair<Stock, int>> items = {{purchasedItem, quantity}};
                receipts.emplace_back(receiptId, items, username);
            } catch (const std::exception& e) {
                std::cerr << "Skipping malformed row in transactions file: " << e.what() << std::endl;
            }
        }
    } catch (const xlnt::exception& e) {
        std::cerr << "Error reading transactions file: " << e.what() << std::endl;
    }
    return receipts;
}

void ExcelUtil::writeTransactionsToFile(const std::string& filename, const std::vector<Receipt>& receipts) {
    ensureDirectoryExists("data");
    xlnt::workbook wb;
    xlnt::worksheet ws = wb.active_sheet();
    ws.cell("A1").value("ReceiptID");
    ws.cell("B1").value("Username");
    ws.cell("C1").value("ItemID");
    ws.cell("D1").value("ItemName");
    ws.cell("E1").value("Quantity");
    ws.cell("F1").value("PricePerUnit");
    ws.cell("G1").value("TotalPrice");
    ws.cell("H1").value("TransactionTime");

    int row_num = 2;
    for (const auto& receipt : receipts) {
        for (const auto& item_pair : receipt.getItems()) {
            const Stock& stock_item = item_pair.first;
            int quantity = item_pair.second;

            ws.cell("A" + std::to_string(row_num)).value(receipt.getReceiptId());
            ws.cell("B" + std::to_string(row_num)).value(receipt.getUsername());
            ws.cell("C" + std::to_string(row_num)).value(stock_item.getId());
            ws.cell("D" + std::to_string(row_num)).value(stock_item.getName());
            ws.cell("E" + std::to_string(row_num)).value(quantity);
            ws.cell("F" + std::to_string(row_num)).value(stock_item.getPrice());
            ws.cell("G" + std::to_string(row_num)).value(stock_item.getPrice() * quantity);

            // ✅ Safe localtime_s usage
            std::time_t time = receipt.getTransactionTime();
            std::tm ptm{};
            if (localtime_s(&ptm, &time) == 0) {
                std::ostringstream oss;
                oss << std::put_time(&ptm, "%Y-%m-%d %H:%M:%S");
                ws.cell("H" + std::to_string(row_num)).value(oss.str());
            } else {
                ws.cell("H" + std::to_string(row_num)).value("Invalid Time");
            }

            row_num++;
        }
    }
    wb.save(filename);
}

int ExcelUtil::getNextReceiptId(const std::vector<Receipt>& receipts) {
    if (receipts.empty()) return 1;
    int maxId = 0;
    for (const auto& receipt : receipts) {
        if (receipt.getReceiptId() > maxId) {
            maxId = receipt.getReceiptId();
        }
    }
    return maxId + 1;
}
