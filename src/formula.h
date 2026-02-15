#ifndef FORMULA_H
#define FORMULA_H

#include <string>
#include <vector>
#include <map>
#include "cube.h"

// Represents a single formula item with its name and moves
struct FormulaItem {
    std::string name;
    std::vector<Move> moves;
};

// Represents a formula file containing multiple formula items
struct FormulaFile {
    std::string filename;  // File name without extension
    std::vector<FormulaItem> items;
};

class FormulaManager {
public:
    FormulaManager();

    // Load all formula files from the formula directory
    void loadFormulas();

    // Get list of available formula file names
    std::vector<std::string> getFileNames() const;

    // Get items in a specific file
    const std::vector<FormulaItem>* getFileItems(const std::string& filename) const;

    // Get the currently selected file's items
    const std::vector<FormulaItem>* getSelectedFileItems() const;

    // Get the currently selected formula item
    const FormulaItem* getSelectedItem() const;

    // Set the currently selected file
    void setSelectedFile(const std::string& filename);

    // Set the currently selected formula item (by index)
    void setSelectedItem(size_t index);

    // Get the currently selected file name
    const std::string& getSelectedFileName() const { return selectedFile_; }

    // Get the currently selected item index
    int getSelectedItemIndex() const { return selectedItemIndex_; }

    // Refresh formula list (reload from directory)
    void refresh();

private:
    // Parse a formula file
    bool parseFormulaFile(const std::string& filePath, FormulaFile& outFile);

    // Map of filename to file data
    std::map<std::string, FormulaFile> files_;

    // Currently selected file name
    std::string selectedFile_;

    // Currently selected item index in the file (-1 if none)
    int selectedItemIndex_;

    // Formula directory path
    std::string formulaDir_;
};

#endif // FORMULA_H
