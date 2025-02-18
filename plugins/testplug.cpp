#include "testplug.hpp"
#include <iostream>
#include <filesystem>
std::string LigmaTest::pluginUUID() const{
    return "7734463d-9fea-4d46-a8cc-e8265cbe46b2";
}
std::string LigmaTest::gameName() const{ return "TestPlug"; }
int LigmaTest::gameID() const {
  return 413150;
  // Stardew Valley ID in Steam
}
std::string LigmaTest::executeCommand() const{ return "program"; }
std::string LigmaTest::modPath() const{return "";}
void LigmaTest::initializeModState(std::filesystem::path modsDirPath) const{

  return;
}
