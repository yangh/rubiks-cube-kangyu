#include <iostream>
#include <getopt.h>

#include "app.h"

// Global flag to enable/disable cube dump (used by renderer and animator)
bool g_enableDump = false;

void showHelp(const char* programName) {
    std::cout << "Rubik's Cube Simulator\n\n";
    std::cout << "Usage: " << programName << " [OPTIONS]\n\n";
    std::cout << "Options:\n";
    std::cout << "  -d, --dump    Enable cube state dump to console\n";
    std::cout << "  -h, --help    Show this help message\n\n";
    std::cout << "Keyboard Shortcuts:\n";
    std::cout << "  U/D/L/R/F/B/M/E/S - Execute corresponding move (clockwise)\n";
    std::cout << "  Shift+Key           - Execute prime move (counter-clockwise)\n";
    std::cout << "  X/Y/Z               - Axis rotation (clockwise)\n";
    std::cout << "  Shift+X/Y/Z         - Axis rotation (counter-clockwise)\n";
    std::cout << "  Space               - Reset view to default angles\n";
    std::cout << "  ESC                 - Reset cube to solved state\n";
    std::cout << "  Ctrl+Z              - Undo last move\n";
    std::cout << "  Ctrl+R              - Redo last undone move\n";
    std::cout << "  Ctrl+Q              - Quit application\n";
    std::cout << "  F11                 - Toggle fullscreen mode\n";
    std::cout << "  Example: 'U' = U move, 'Shift+U' = U' move\n";
}

int main(int argc, char* argv[]) {
    // Parse command line arguments using getopt_long
    static struct option long_options[] = {
        {"dump", no_argument, 0, 'd'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;
    bool enableDump = false;

    while ((opt = getopt_long(argc, argv, "dh", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'd':
                enableDump = true;
                break;
            case 'h':
                showHelp(argv[0]);
                return 0;
            default:
                std::cerr << "Unknown option\n\n";
                showHelp(argv[0]);
                return 1;
        }
    }

    Application app;
    app.setEnableDump(enableDump);

    return app.run();
}
