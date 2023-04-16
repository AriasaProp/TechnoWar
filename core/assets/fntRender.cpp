#include <iostream>
#include <fstream>
#include <vector>
#include <string>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

struct Character {
    int id;
    float x, y, width, height;
    float xoffset, yoffset, xadvance;
};

int try_on() {

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

    // Load texture atlas
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    // ... Load texture data ...

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Setup vertex data
    float vertices[] = {
        // Position      TexCoord
        0.0f, 0.0f,     0.0f, 0.0f,
        1.0f, 0.0f,     1.0f, 0.0f,
        1.0f, 1.0f,     1.0f, 1.0f,
        0.0f, 1.0f,     0.0f, 1.0f
    };

    GLuint indices[] = {
        0, 1, 2,
        0, 2, 3
    };

    // Setup shader program
    const char* vertexShaderSource = 
        "attribute vec4 aPosition;\n"
        "attribute vec2 aTexCoord;\n"
        "varying vec2 vTexCoord;\n"
        "void main() {\n"
        "  gl_Position = aPosition;\n"
        "  vTexCoord = aTexCoord;\n"
        "}\n";

    const char* fragmentShaderSource = 
        "precision mediump float;\n"
        "uniform sampler2D uTexture;\n"
        "varying vec2 vTexCoord;\n"
        "void main() {\n"
        "  gl_FragColor = texture2D(uTexture, vTexCoord);\n"
        "}\n";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1
