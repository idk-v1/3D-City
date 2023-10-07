#ifndef BLOCK_H
#define BLOCK_H

#include <SFML/Graphics.hpp>


class Block
{
    public:
        Block();

        void setAttrib(Block pTemplate);

        bool isSolid : 1 = 0;
        bool visXP : 1;
        bool visYP : 1;
        bool visZP : 1;
        bool visXN : 1;
        bool visYN : 1;
        bool visZN : 1;

        sf::Uint16 type = 0;
};

#endif
