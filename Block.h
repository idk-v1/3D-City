#ifndef BLOCK_H
#define BLOCK_H

#include <vector>
#include <SFML/Graphics.hpp>

#include "Light.h"


class Block
{
    public:
        Block();

        void setAttrib(Block pTemplate);

        void toggle(int pIndex);

        bool isSolid : 1;
        bool visXP : 1;
        bool visYP : 1;
        bool visZP : 1;
        bool visXN : 1;
        bool visYN : 1;
        bool visZN : 1;

        sf::Uint16 type;

        std::string name;

        std::vector<Light> lights;
};

#endif
