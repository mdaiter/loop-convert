/**
 * Class orchestrator
 */

#include <vector>
#include <string>
#include <fstream>

class Orchestrator {
  public:
    static Orchestrator* getInstance();
    void pushFds(const std::string& fds);
    void pushTypedef(const std::string& typedefStr);
    void pushApiMapping(const std::string& apiMappingStr);
    void setOutputFile(std::ofstream* const output);
    bool writeFile() const;
  private:
    static Orchestrator* instance;
    Orchestrator();
    // Fully Qualified Names (converted)
    std::vector<std::string> fdsVector;
    // Opaque Pointer Names
    std::vector<std::string> typedefVector;
    //
    std::vector<std::string> apiMappingVector;
    // Output file
    std::ofstream* outputFile = nullptr;
};
