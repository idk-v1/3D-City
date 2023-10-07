#include <fstream>

#include "City.h"
#include "Vec3Dist.h"


void getInput(sf::RenderWindow& pWin, sf::Vector3f& pVel, sf::Vector3f& pRot, sf::Vector2i& pMousePos, int pDelta);


void genCube(sf::VertexArray& pVert, int pIndex, Block pBlock, sf::Vector3f pPlayerPos, sf::Vector3f pPlayerRot, sf::Vector3f pObjPos, sf::Vector2u pWinSize);


bool project(sf::Vertex& pVert, sf::Vector3f pPlayerPos, sf::Vector3f pPlayerRot, sf::Vector3f pObjPos, sf::Vector2u pWinPos);


void loadBlockTemplates(std::vector<Block>& pBlocks);


int main()
{
    sf::RenderWindow win(sf::VideoMode(800, 600), "");
    sf::View view;
    sf::Clock timer;
    sf::VertexArray vert;
    sf::Texture tex;
    sf::Color bg(255, 127, 255);

    sf::Vector3f pos(-50.f, 100.f, -50.f), vel, rot(225.f, -45.f, 0.f);
    sf::Vector2i mousePos;

    sf::Vector3i size(21, 10, 21);
    City city(size);
    std::vector<Vec3Dist> reorder;
    std::vector<Block> templates;

    int delta, count;
    bool focus = false;

    win.setFramerateLimit(60);
    reorder.resize(size.x * size.y * size.z);
    vert.setPrimitiveType(sf::Quads);
    vert.resize(size.x * size.y * size.z * 4 * 6 * 2);
    loadBlockTemplates(templates);
    city.setTemplatePtr(&templates);
    city.generate(0);
    tex.loadFromFile("res/blocks.png");

    while (win.isOpen())
    {
        sf::Event evt;
        while (win.pollEvent(evt))
            switch (evt.type)
            {
            case sf::Event::Closed:
                win.close();
                break;
            case sf::Event::GainedFocus:
                focus = true;
                break;
            case sf::Event::LostFocus:
                focus = false;
                break;
            case sf::Event::MouseButtonPressed:
                if (focus && evt.mouseButton.button == sf::Mouse::Left && win.getViewport(view).contains(sf::Mouse::getPosition(win)))
                {
                    win.setMouseCursorVisible(false);
                    sf::Mouse::setPosition(sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2), win);
                    mousePos = sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2);
                }
                break;
            case sf::Event::MouseButtonReleased:
                if (focus && evt.mouseButton.button == sf::Mouse::Left)
                {
                    win.setMouseCursorVisible(true);
                    sf::Mouse::setPosition(sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2), win);
                }
                break;
            }

        delta = timer.restart().asMilliseconds();
        if (focus)
            getInput(win, vel, rot, mousePos, delta);
        vel.x *= 0.8f;
        vel.y *= 0.8f;
        vel.z *= 0.8f;
        pos.x += vel.x;
        pos.y += vel.y;
        pos.z += vel.z;

        view.reset(sf::FloatRect(0, 0, win.getSize().x, win.getSize().y));
        win.setView(view);

        win.setTitle("Final Earth 3D | FPS: " + (delta ? std::to_string(1000 / delta) : "Inf"));

        win.clear(bg);

        city.calcVis();

        count = 0;
        for (int x = 0; x < size.x; x++)
            for (int y = 0; y < size.y; y++)
                for (int z = 0; z < size.z; z++)
                    if (city.getBlock(x, y, z).isSolid)
                        reorder[count++] = Vec3Dist(pos, sf::Vector3f(x, y, z));

        std::sort(reorder.begin(), reorder.begin() + count, [](Vec3Dist a, Vec3Dist b)
        {
            return a.dist > b.dist;
        });

        count = 0;
        for (int x = 0; x < size.x; x++)
            for (int y = 0; y < size.y; y++)
                for (int z = 0; z < size.z; z++)
                    if (city.getBlock(x, y, z).isSolid)
                    {
                        genCube(vert, count * 4 * 6, city.getBlock(sf::Vector3i(reorder[count / 2].pos)), pos, rot, reorder[count / 2].pos, win.getSize());
                        for (int i = 0; i < 4 * 6; i++)
                        {
                            vert[(count + 1) * 4 * 6 + i] = vert[count * 4 * 6 + i];
                            vert[count * 4 * 6 + i].color = bg;
                        }
                        count += 2;
                    }

        win.draw(vert);

        win.display();

        for (int i = 0; i < count * 4 * 6; i++)
            vert[i] = sf::Vertex();
        for (int i = 0; i < count / 2; i++)
        {
            reorder[i].dist = 0;
            reorder[i].pos = sf::Vector3f();
        }
    }
}

void genCube(sf::VertexArray& pVert, int pIndex, Block pBlock, sf::Vector3f pPlayerPos, sf::Vector3f pPlayerRot, sf::Vector3f pObjPos, sf::Vector2u pWinSize)
{
    float scale = 10.f;
    if (pBlock.visXP && pPlayerPos.x > (pObjPos.x + 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 0 + i].color = sf::Color(255, 255, 255);
        if (/**/project(pVert[pIndex + 0], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5), pWinSize) &&
                project(pVert[pIndex + 1], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5), pWinSize) &&
                project(pVert[pIndex + 2], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5), pWinSize) &&
                project(pVert[pIndex + 3], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 0 + i] = sf::Vertex();
    }
    if (pBlock.visXN && pPlayerPos.x < (pObjPos.x - 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 4 + i].color = sf::Color(0, 0, 0);
        if (/**/project(pVert[pIndex +  4], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  5], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  6], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  7], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 4 + i] = sf::Vertex();
    }
    if (pBlock.visYP && pPlayerPos.y > (pObjPos.y + 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 8 + i].color = sf::Color(192, 192, 192);
        if (/**/project(pVert[pIndex +  8], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  9], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 10], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 11], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 8 + i] = sf::Vertex();
    }
    if (pBlock.visYN && pPlayerPos.y < (pObjPos.y - 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 12 + i].color = sf::Color(64, 64, 64);
        if (/**/project(pVert[pIndex + 12], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 13], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 14], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 15], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 12 + i] = sf::Vertex();
    }
    if (pBlock.visZP && pPlayerPos.z > (pObjPos.z + 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 16 + i].color = sf::Color(128, 128, 128);
        if (/**/project(pVert[pIndex + 16], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 17], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 18], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 19], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 16 + i] = sf::Vertex();
    }
    if (pBlock.visZN && pPlayerPos.z < (pObjPos.z - 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 20 + i].color = sf::Color(128, 128, 128);
        if (/**/project(pVert[pIndex + 20], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 21], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 22], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 23], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 20 + i] = sf::Vertex();
    }
}


// https://en.wikipedia.org/wiki/3D_projection
bool project(sf::Vertex& pVert, sf::Vector3f pPlayerPos, sf::Vector3f pPlayerRot, sf::Vector3f pObjPos, sf::Vector2u pWinSize)
{
    float scale = 10.f, fog = 500.f,
          x = pObjPos.x * scale - pPlayerPos.x,
          y = pObjPos.y * scale - pPlayerPos.y,
          z = pObjPos.z * scale - pPlayerPos.z;
    sf::Vector3f d,
    c(std::cos(pPlayerRot.y * 3.14f / 180.f), std::cos(pPlayerRot.x * 3.14f / 180.f), std::cos(pPlayerRot.z * 3.14f / 180.f)),
    s(std::sin(pPlayerRot.y * 3.14f / 180.f), std::sin(pPlayerRot.x * 3.14f / 180.f), std::sin(pPlayerRot.z * 3.14f / 180.f));

    d.x = c.y * (s.z * y + c.z * x) - s.y * z;
    d.y = s.x * (c.y * z + s.y * (s.z * y + c.z * x)) + c.x * (c.z * y - s.z * x);
    d.z = c.x * (c.y * z + s.y * (s.z * y + c.z * x)) - s.x * (c.z * y - s.z * x);

    pVert.position.x = pWinSize.x / 2 - (d.x * 1000.f) / std::sqrt(x * x + y * y + z * z);
    pVert.position.y = pWinSize.y / 2 - (d.y * 1000.f) / std::sqrt(x * x + y * y + z * z);
    pVert.color.a = std::fmax(0, 255 - std::fmin(255, 255.f * (std::sqrt(x * x + y * y + z * z) / fog)));

    return d.z < 0;
}


void loadBlockTemplates(std::vector<Block>& pBlocks)
{
    std::ifstream file("res/blocks.type");
    int count;
    bool solid;
    if (file.is_open())
    {
        file >> count;
        pBlocks.resize(count);
        for (int i = 0; i < count; i++)
        {
            file >> solid;
            pBlocks[i].isSolid = solid;
        }
    }
}


void getInput(sf::RenderWindow& pWin, sf::Vector3f& pVel, sf::Vector3f& pRot, sf::Vector2i& pMousePos, int pDelta)
{
    sf::Vector2f sensitivity(1.f, 1.f);
    float speed = 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
    {
        pVel.x += pDelta * speed / 100.f * std::sin((pRot.x + 180.f) * 3.14f / 180.f) * std::sin((pRot.y +  90.f) * 3.14f / 180.f);
        pVel.z += pDelta * speed / 100.f * std::sin((pRot.x + 270.f) * 3.14f / 180.f) * std::sin((pRot.y +  90.f) * 3.14f / 180.f);
        pVel.y += pDelta * speed / 100.f * std::sin((pRot.y +   0.f) * 3.14f / 180.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
    {
        pVel.x += pDelta * speed / 100.f * std::sin((pRot.x +   0.f) * 3.14f / 180.f) * std::sin((pRot.y +  90.f) * 3.14f / 180.f);
        pVel.z += pDelta * speed / 100.f * std::sin((pRot.x +  90.f) * 3.14f / 180.f) * std::sin((pRot.y +  90.f) * 3.14f / 180.f);
        pVel.y += pDelta * speed / 100.f * std::sin((pRot.y + 180.f) * 3.14f / 180.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
    {
        pVel.x += pDelta * speed / 100.f * std::sin((pRot.x + 270.f) * 3.14f / 180.f);
        pVel.z += pDelta * speed / 100.f * std::sin((pRot.x +   0.f) * 3.14f / 180.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
    {
        pVel.x += pDelta * speed / 100.f * std::sin((pRot.x +  90.f) * 3.14f / 180.f);
        pVel.z += pDelta * speed / 100.f * std::sin((pRot.x + 180.f) * 3.14f / 180.f);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
        pVel.y += pDelta * speed / 100.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift))
        pVel.y -= pDelta * speed / 100.f;

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
    {
        pRot.x -= (pMousePos.x - sf::Mouse::getPosition(pWin).x) * pDelta / 50.f * sensitivity.x;
        pRot.y += (pMousePos.y - sf::Mouse::getPosition(pWin).y) * pDelta / 50.f * sensitivity.y;
        if (pRot.y > 90.f)
            pRot.y = 90.f;
        if (pRot.y < -90.f)
            pRot.y = -90.f;
        if (pRot.x >= 360)
            pRot.x -= 360;
        if (pRot.x < 0)
            pRot.x += 360;


        // mouse leaves left of window
        if (sf::Mouse::getPosition(pWin).x < 0)
            sf::Mouse::setPosition(sf::Vector2i(pWin.getSize().x + sf::Mouse::getPosition(pWin).x, sf::Mouse::getPosition(pWin).y), pWin);
        // mouse leaves right of window
        if (sf::Mouse::getPosition(pWin).x >= pWin.getSize().x)
            sf::Mouse::setPosition(sf::Vector2i(sf::Mouse::getPosition(pWin).x - pWin.getSize().x, sf::Mouse::getPosition(pWin).y), pWin);
        // mouse leaves top of window
        if (sf::Mouse::getPosition(pWin).y < 0)
            sf::Mouse::setPosition(sf::Vector2i(sf::Mouse::getPosition(pWin).x, pWin.getSize().y + sf::Mouse::getPosition(pWin).y), pWin);
        // mouse leaves bottom of window
        if (sf::Mouse::getPosition(pWin).y >= pWin.getSize().y)
            sf::Mouse::setPosition(sf::Vector2i(sf::Mouse::getPosition(pWin).x, sf::Mouse::getPosition(pWin).y - pWin.getSize().y), pWin);

        pMousePos = sf::Mouse::getPosition(pWin);
    }
}
