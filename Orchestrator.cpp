#include "AddApiMapping.h"
#include "Orchestrator.h"

Orchestrator* Orchestrator::instance = 0;
Orchestrator* Orchestrator::getInstance() {
  if (!instance) {
    instance = new Orchestrator();
  }
  return instance;
}

Orchestrator::Orchestrator() {
}

void Orchestrator::pushFds(const std::string& fds) {
  fdsVector.push_back(fds);
}
void Orchestrator::pushTypedef(const std::string& typedefStr) {
  typedefVector.push_back(typedefStr);
}
void Orchestrator::pushApiMapping(const std::string& apiMappingStr) {
  apiMappingVector.push_back(apiMappingStr);
}

bool Orchestrator::writeFile() const {
  if (outputFile != nullptr) {
    // Need to append this to file inherently
    for (const auto stmt : constexprStmts) {
      *outputFile << stmt << "\n";
    }

    for (const auto fd : fdsVector) {
      *outputFile << fd << "\n";
    }
    for (const auto typedefStr : typedefVector) {
      *outputFile << typedefStr << "\n";
    }
    for (const auto apiMappingStr : apiMappingVector) {
      *outputFile << apiMappingStr << "\n";
    }
    return 1;
  }
  return 0;
}

void Orchestrator::setOutputFile(std::ofstream* const output) {
  outputFile = output;
}
