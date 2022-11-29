#include <cctype>
#include <vector>

#include "system/assert.h"
#include "system/file.h"
#include "system/string_helpers.h"
#include "system/yaml_parser.h"

namespace Gamma {
  /**
   * Gm_ParsePrimitiveValue
   * ----------------------
   *
   * @todo description
   */
  static void* Gm_ParsePrimitiveValue(const std::string& str) {
    // Check for boolean literals
    if (str == "true") {
      return new bool(true);
    } else if (str == "false") {
      return new bool(false);
    }

    // Check for numbers
    if (std::isdigit(str[0])) {
      // @todo use std::stof if the string contains a '.'
      return new int(std::stoi(str));
    }

    // Fall back to string
    // @todo strip leading/trailing quotes
    return new std::string(str);
  }

  /**
   * Gm_ParseYamlFile
   * ----------------
   */
  YamlObject& Gm_ParseYamlFile(const char* path) {
    std::vector<YamlObject*> objectStack;
    std::string fileContents = Gm_LoadFileContents(path);
    auto lines = Gm_SplitString(fileContents, "\n");
    auto* root = new YamlObject();

    objectStack.push_back(root);

    for (u32 i = 0; i < lines.size(); i++) {
      auto trimmedLine = Gm_TrimString(lines[i]);

      if (trimmedLine[0] == '}') {
        // End of object
        objectStack.pop_back();
      } else if (trimmedLine.find(":") != std::string::npos) {
        // Property declaration
        YamlProperty property;

        if (trimmedLine.back() == '{') {
          // Nested object property
          property.object = new YamlObject();
        } else if (trimmedLine.back() == '[') {
          // Specialization for array leaf properties
          auto array = new YamlArray<void*>();
          auto incomingLine = Gm_TrimString(lines[++i]);

          // Iterate over and parse new lines into values
          // until we reach the end of the array
          while (incomingLine[0] != ']') {
            if (incomingLine.back() == ',') {
              incomingLine = incomingLine.substr(0, incomingLine.size() - 1);
            }

            // @todo strip leading/trailing quotes

            array->push_back(Gm_ParsePrimitiveValue(incomingLine));

            incomingLine = Gm_TrimString(lines[++i]);
          }

          property.value = array;
        } else {
          // Other leaf properties (strings, numbers, or booleans)
          u32 vStart = trimmedLine.find(":") + 1;
          u32 vLength = trimmedLine.find(",") - vStart;
          auto value = Gm_TrimString(trimmedLine.substr(vStart, vLength));

          property.value = Gm_ParsePrimitiveValue(value);
        }

        // Assign the property to the current object
        auto propertyName = trimmedLine.substr(0, trimmedLine.find(":"));
        auto& currentObject = *objectStack.back();

        currentObject[propertyName] = property;

        if (property.object != nullptr) {
          // Push nested objects onto the stack so they can
          // represent the current object on the next cycle
          objectStack.push_back(property.object);
        }
      }
    }

    // Remove the root object from the stack
    objectStack.pop_back();

    assert(objectStack.size() == 0, "Malformed YAML file");

    return *root;
  }

  /**
   * Gm_HasYamlProperty
   * ------------------
   */
  bool Gm_HasYamlProperty(const YamlObject& object, const std::string& property) {
    return object.find(property) != object.end();
  }

  /**
   * Gm_FreeYamlObject
   * --------------------
   */
  void Gm_FreeYamlObject(YamlObject* object) {
    for (auto& [ key, property ] : *object) {
      if (property.object != nullptr) {
        Gm_FreeYamlObject(property.object);
      } else if (property.value != nullptr) {
        delete property.value;
      }
    }

    object->clear();

    delete object;
  }
}