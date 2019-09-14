#include <iostream>
#include <fstream>
// Declares clang::SyntaxOnlyAction.
#include "clang/Frontend/FrontendActions.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
// Declares llvm::cl::extrahelp.
#include "llvm/Support/CommandLine.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

#include "AddApiMapping.h"
#include "Orchestrator.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;
using namespace llvm;

auto classMatcher = cxxRecordDecl(isClass()).bind("id");

clang::ast_matchers::internal::BindableMatcher<clang::Decl> mk_call_expr_matcher(std::string const & ns_name)
{
  return namespaceDecl(hasName(ns_name));
}

class ForwardDeclarer : public MatchFinder::MatchCallback {
public:
  const std::string qualifiedToForwardDecl(const std::string& className) const {
    // Here, we need to add our typedefs and forward decls
    const auto qualified = className;
    size_t total = 0;

    auto start = 0U;
    auto end = qualified.find(delim);

    std::string retr_str = "";
    while (end != std::string::npos)
    {
      retr_str += "namespace " + qualified.substr(start, end-start) + " { ";
      start = end + delim_length;
      end = qualified.find(delim, start);
      ++total;
    }
    retr_str += "class " + qualified.substr(start, end) + ";";
    do { retr_str += " } ";} while (--total);
    retr_str += "\n";
    return retr_str;
  }

  // Convert qualified classes to opaque names in C.
  const std::string qualifiedToOpaqueCPtr(const std::string& className) const {
    const auto qualified = className;
    auto start = 0U;
    auto end = qualified.find(delim);
    
    std::string retr_str = "";
    while (end != std::string::npos)
    { 
      retr_str += toupper(qualified[start]);
      retr_str += qualified.substr(start+1, end-start-1);
      start = end + delim_length; 
      end = qualified.find(delim, start);
    }
    retr_str += toupper(qualified[start]);
    retr_str += qualified.substr(start+1, end-start-1);
    retr_str += endDelimiter;
    return retr_str;
  }

  const std::string constructTypedefMapping(const std::string& qualified, const std::string& opaquePtr) const {
    return "typedef " + qualified + "* " + opaquePtr + ";" + "\n";
  }
  const std::string constructApiMapping(const std::string& qualified, const std::string& opaquePtr) const {
    return "ADD_API_MAPPING(" + opaquePtr + ", " + qualified + ")" + "\n";
  }

  virtual void run(const MatchFinder::MatchResult &result) {
    CXXRecordDecl const * const call =
      result.Nodes.getNodeAs<CXXRecordDecl>("id");
    // TODO(mdaiter): find a better way to screen for default constructors vs classes
    if (call && ++count_for_method_screen & 1) {
      const auto forwardDeclStr = qualifiedToForwardDecl(call->getQualifiedNameAsString());
      Orchestrator::getInstance()->pushFds(forwardDeclStr);
      const auto opaquePtr = qualifiedToOpaqueCPtr(call->getQualifiedNameAsString());
      Orchestrator::getInstance()->pushTypedef(constructTypedefMapping(call->getQualifiedNameAsString(), opaquePtr));
      Orchestrator::getInstance()->pushApiMapping(constructApiMapping(call->getQualifiedNameAsString(), opaquePtr));
    }
    return;
  }
private:
  uint8_t count_for_method_screen = 0;
  static constexpr auto delim = "::";
  static constexpr auto delim_length = 2;
  static constexpr auto endDelimiter = "Ptr";
};

// Apply a custom category to all command-line options so that they are the
// only ones displayed.
static llvm::cl::OptionCategory MyToolCategory("my-tool options");

// CommonOptionsParser declares HelpMessage with a description of the common
// command-line options related to the compilation database and input files.
// It's nice to have this help message in all tools.
static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// A help message for this specific tool can be added afterwards.
static cl::extrahelp MoreHelp("\nMore help text...\n");

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv, MyToolCategory);
  ClangTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());

  ForwardDeclarer forwardDeclarer;
  MatchFinder Finder;
  Finder.addMatcher(classMatcher, &forwardDeclarer);

  std::ofstream outputFile;
  outputFile.open("conversions.h");

  Orchestrator::getInstance()->setOutputFile(&outputFile);

  Tool.run(newFrontendActionFactory(&Finder).get());

  Orchestrator::getInstance()->writeFile();
  outputFile.close();
  return 0;
}

