#include "../include/ExcelUtil.hpp"
#include <iostream>
#include <filesystem> // For creating directories
#include <numeric>

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
