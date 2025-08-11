#ifndef EXCEL_UTIL_HPP
#define EXCEL_UTIL_HPP
#define _CRT_SECURE_NO_WARNINGS

#include <vector>
#include <string>
#include <xlnt/xlnt.hpp>
#include "User.hpp"
#include "Stock.hpp"
#include "Receipt.hpp" // Include Receipt.hpp

// Utility class for reading and writing data to Excel files
class ExcelUtil {
public:
    // User file functions
    static std::vector<User> readUsersFromFile(const std::string& filename);
    static void writeUsersToFile(const std::string& filename, const std::vector<User>& users);
    
    // Stock file functions
    static std::vector<Stock> readStockFromFile(const std::string& filename);
    static void writeStockToFile(const std::string& filename, const std::vector<Stock>& stocks);
    static int getNextStockId(const std::vector<Stock>& stocks);

    // Transaction file functions (New)
    static std::vector<Receipt> readTransactionsFromFile(const std::string& filename);
    static void writeTransactionsToFile(const std::string& filename, const std::vector<Receipt>& receipts);
    static int getNextReceiptId(const std::vector<Receipt>& receipts); // New: Get next receipt ID

private:
    // Helper functions for file creation
    static void createUsersFile(const std::string& filename);
    static void createStockFile(const std::string& filename);
    static void createTransactionsFile(const std::string& filename); // New: Helper to create transactions file

};

#endif // EXCEL_UTIL_HPP
