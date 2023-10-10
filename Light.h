#ifndef LIGHT_H
#define LIGHT_H

#include <SFML/System/Vector2.hpp>
#include <string>

class Light
{
    public:
        Light();

        Light(sf::Vector2i pPos, sf::Vector2i pSize);

        Light(std::string pStr);

        sf::Vector2i pos, size;
        bool state = false;
};

#endif
