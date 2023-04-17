#include <iostream>
#include <fstream>
#include <vector>
#include <string>

struct Character {
    int id;
    float x, y, width, height;
    float xoffset, yoffset, xadvance;
};

void try_on() {
    // Load FNT file
    std::ifstream file("font.fnt");
    if (!file.is_open()) {
        std::cout << "Failed to open font file." << std::endl;
        return -1;
    }

    // Parse FNT file
    std::vector<Character> characters;
    std::string line;
    while (std::getline(file, line)) {
        if (line.substr(0, 4) == "char") {
            Character character;
            sscanf(line.c_str(), "char %*s id=%d x=%f y=%f width=%f height=%f xoffset=%f yoffset=%f xadvance=%f", 
                   &character.id, &character.x, &character.y, &character.width, &character.height, 
                   &character.xoffset, &character.yoffset, &character.xadvance);
            characters.push_back(character);
        }
    }
}
