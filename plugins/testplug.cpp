#include "testplug.hpp"

#include <QString>
#include <vector>

QString LigmaTest::pluginUUID() const {
    return "7734463d-9fea-4d46-a8cc-e8265cbe46b2";
}
QString LigmaTest::gameName() const {
    return "TestPlug";
}
int LigmaTest::gameID() const {
    return 413150;
    // Stardew Valley ID in Steam
}
QString LigmaTest::executableName() const {
    return "program";
}
bool LigmaTest::usesProton() const {
    return false;
}
std::vector<QString> LigmaTest::environmentVariables() const {
    return {};
}
std::vector<QString> LigmaTest::modPaths() const {
    return {};
}
void LigmaTest::initializeModState(std::filesystem::path modsDirPath) const {
    return;
}
QString LigmaTest::initialMessage() const {
    return "This is a test plugin!";
}
