#include "Block.h"

Block::Block()
{

}


void Block::setAttrib(Block pTemplate)
{
    *this = pTemplate;
}


void Block::toggle(int pIndex)
{
    lights[pIndex].state = !lights[pIndex].state;
}
