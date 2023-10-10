#include "Light.h"

Light::Light()
{

}


Light::Light(sf::Vector2i pPos, sf::Vector2i pSize)
{
    pos = pPos;
    size = pSize;
}


Light::Light(std::string pStr)
{
    int comma = 0;
    pos.x = std::stoi(pStr.substr(comma, pStr.find(',', comma)));
    comma = pStr.find(',', comma) + 1;
    pos.y = std::stoi(pStr.substr(comma, pStr.find(',', comma)));
    comma = pStr.find(',', comma) + 1;
    size.x = std::stoi(pStr.substr(comma, pStr.find(',', comma)));
    comma = pStr.find(',', comma) + 1;
    size.y = std::stoi(pStr.substr(comma));
}
