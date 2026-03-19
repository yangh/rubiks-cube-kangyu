#include "formula.h"
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <iostream>

FormulaManager::FormulaManager()
    : selectedItemIndex_(-1) {
    // Determine formula directory path
    formulaDir_ = "formula";
}

void FormulaManager::loadFormulas() {
    files_.clear();
    selectedFile_ = "";
    selectedItemIndex_ = -1;

    // Check if formula directory exists
    if (!std::filesystem::exists(formulaDir_)) {
        std::cerr << "Formula directory '" << formulaDir_ << "' not found." << std::endl;
        return;
    }

    // Iterate through all .txt files in formula directory
    for (const auto& entry : std::filesystem::directory_iterator(formulaDir_)) {
        if (entry.is_regular_file() && entry.path().extension() == ".txt") {
            FormulaFile file;
            if (parseFormulaFile(entry.path().string(), file)) {
                files_[file.filename] = file;
            }
        }
    }

    // Auto-select first file and first item if available
    if (!files_.empty()) {
        selectedFile_ = files_.begin()->first;
        if (!files_[selectedFile_].items.empty()) {
            selectedItemIndex_ = 0;
        }
    }
}

std::vector<std::string> FormulaManager::getFileNames() const {
    std::vector<std::string> names;
    for (const auto& pair : files_) {
        names.push_back(pair.first);
    }
    std::sort(names.begin(), names.end());
    return names;
}

const std::vector<FormulaItem>* FormulaManager::getFileItems(const std::string& filename) const {
    auto it = files_.find(filename);
    if (it != files_.end()) {
        return &it->second.items;
    }
    return nullptr;
}

const std::vector<FormulaItem>* FormulaManager::getSelectedFileItems() const {
    return getFileItems(selectedFile_);
}

const FormulaItem* FormulaManager::getSelectedItem() const {
    const std::vector<FormulaItem>* items = getSelectedFileItems();
    if (items != nullptr && selectedItemIndex_ >= 0 &&
        static_cast<size_t>(selectedItemIndex_) < items->size()) {
        return &(*items)[selectedItemIndex_];
    }
    return nullptr;
}

void FormulaManager::setSelectedFile(const std::string& filename) {
    if (files_.find(filename) != files_.end()) {
        selectedFile_ = filename;
        // Select first item in the file
        selectedItemIndex_ = (!files_[filename].items.empty()) ? 0 : -1;
    }
}

void FormulaManager::setSelectedItem(size_t index) {
    const std::vector<FormulaItem>* items = getSelectedFileItems();
    if (items != nullptr && index < items->size()) {
        selectedItemIndex_ = static_cast<int>(index);
    }
}

void FormulaManager::refresh() {
    loadFormulas();
}

bool FormulaManager::parseFormulaFile(const std::string& filePath, FormulaFile& outFile) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open formula file: " << filePath << std::endl;
        return false;
    }

    // Extract filename without extension
    std::filesystem::path path(filePath);
    outFile.filename = path.stem().string();
    outFile.items.clear();

    std::string line;
    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        // Skip empty lines and comment lines
        if (line.empty() || line[0] == '#') {
            continue;
        }

        FormulaItem item;

        // Check if line has format "Name: moves" or just "moves"
        size_t colonPos = line.find(':');
        if (colonPos != std::string::npos && colonPos > 0) {
            // Format: "Name: moves"
            item.name = line.substr(0, colonPos);
            // Trim name
            item.name.erase(0, item.name.find_first_not_of(" \t"));
            item.name.erase(item.name.find_last_not_of(" \t") + 1);

            std::string movesStr = line.substr(colonPos + 1);

            // Check for loop syntax: "* number" at the end
            size_t starPos = movesStr.rfind('*');
            if (starPos != std::string::npos) {
                // Extract the loop count
                std::string loopStr = movesStr.substr(starPos + 1);
                // Trim whitespace
                loopStr.erase(0, loopStr.find_first_not_of(" \t"));
                loopStr.erase(loopStr.find_last_not_of(" \t") + 1);
                // Remove loop syntax from moves string
                movesStr = movesStr.substr(0, starPos);
                // Parse loop count
                try {
                    item.loopCount = std::stoi(loopStr);
                } catch (...) {
                    item.loopCount = 0;
                }
            }

            item.moves = parseMoveSequence(movesStr);
        } else {
            // Format: just "moves", use "Formula N" as name
            static int formulaCounter = 1;
            item.name = "Formula " + std::to_string(formulaCounter++);

            // Check for loop syntax: "* number" at the end
            size_t starPos = line.rfind('*');
            if (starPos != std::string::npos) {
                // Extract the loop count
                std::string loopStr = line.substr(starPos + 1);
                // Trim whitespace
                loopStr.erase(0, loopStr.find_first_not_of(" \t"));
                loopStr.erase(loopStr.find_last_not_of(" \t") + 1);
                // Remove loop syntax from moves string
                std::string movesStr = line.substr(0, starPos);
                // Parse loop count
                try {
                    item.loopCount = std::stoi(loopStr);
                } catch (...) {
                    item.loopCount = 0;
                }
                item.moves = parseMoveSequence(movesStr);
            } else {
                item.moves = parseMoveSequence(line);
            }
        }

        // Only add if there are valid moves
        if (!item.moves.empty()) {
            outFile.items.push_back(item);
        }
    }

    file.close();
    return !outFile.items.empty();
}
