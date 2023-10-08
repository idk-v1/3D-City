// https://github.com/idk-v1/3D-City
// Currently just a basic 3D renderer with collisions and minor clipping errors

#include <fstream>

#include "City.h"


void getInput(sf::RenderWindow& pWin, sf::Vector3f& pVel, sf::Vector3f& pRot, sf::Vector2i& pMousePos, int pDelta);


void collide(City& pCity, sf::Vector3f& pPos, sf::Vector3f& pVel);


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
    sf::RectangleShape overlay;

    sf::Vector3f pos(-50.f, 100.f, -50.f), vel, rot(225.f, -45.f, 0.f);
    sf::Vector2i mousePos(win.getSize().x / 2, win.getSize().y / 2);

    sf::Vector3i size(21, 10, 21);
    City city(size);
    std::vector<Vec3Dist> reorder;
    std::vector<Block> templates;

    int delta, count, time = 0, dayLength = 60000;
    unsigned long long ticks = 0;
    bool focus = false, click = false, paused = false;

    win.setFramerateLimit(60);
    win.setMouseCursorVisible(false);
    sf::Mouse::setPosition(sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2), win);
    reorder.resize(size.x * size.y * size.z);
    vert.setPrimitiveType(sf::Quads);
    vert.resize(size.x * size.y * size.z * 4 * 3 * 2);
    loadBlockTemplates(templates);
    city.setTemplatePtr(&templates);
    city.generate(0);
    tex.loadFromFile("res/blocks.png");
    overlay.setFillColor(sf::Color(63, 63, 63, 127));

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
            case sf::Event::KeyPressed:
                if (focus)
                {
                    if (evt.key.code == sf::Keyboard::Escape)
                    {
                        paused = !paused;
                        if (paused)
                        {
                            win.setMouseCursorVisible(true);
                            sf::Mouse::setPosition(sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2), win);
                        }
                        else
                        {
                            win.setMouseCursorVisible(false);
                            sf::Mouse::setPosition(sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2), win);
                            mousePos = sf::Vector2i(win.getSize().x / 2, win.getSize().y / 2);
                        }
                    }
                }
                break;
            }

        delta = timer.restart().asMilliseconds();
        ticks += delta;
        time = ticks % dayLength - dayLength / 2;
        if (focus && !paused)
            getInput(win, vel, rot, mousePos, delta);
        collide(city, pos, vel);

        view.reset(sf::FloatRect(0, 0, win.getSize().x, win.getSize().y));
        win.setView(view);

        win.setTitle("3D City | FPS: " + (delta ? std::to_string(1000 / delta) : "Infinity"));

        win.clear(bg);

        city.calcVis(reorder, pos);

        count = 0;
        for (int x = 0; x < size.x; x++)
            for (int y = 0; y < size.y; y++)
                for (int z = 0; z < size.z; z++)
                    if (city.getBlock(x, y, z).isSolid)
                    {
                        genCube(vert, count * 4 * 3, city.getBlock(sf::Vector3i(reorder[count / 2].pos)), pos, rot, reorder[count / 2].pos, win.getSize());
                        for (int i = 0; i < 4 * 3; i++)
                        {
                            vert[(count + 1) * 4 * 3 + i] = vert[count * 4 * 3 + i];
                            vert[count * 4 * 3 + i].color = bg;
                        }
                        count += 2;
                    }

        win.draw(vert);

        if (paused)
        {
            overlay.setSize(sf::Vector2f(win.getSize()));
            win.draw(overlay);
        }

        win.display();

        for (int i = 0; i < vert.getVertexCount(); i++)
            vert[i] = sf::Vertex();
        for (int i = 0; i < reorder.size(); i++)
            reorder[i] = Vec3Dist();
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
    else if (pBlock.visXN && pPlayerPos.x < (pObjPos.x - 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 0 + i].color = sf::Color(0, 0, 0);
        if (/**/project(pVert[pIndex +  0], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  1], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  2], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  3], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 0 + i] = sf::Vertex();
    }
    if (pBlock.visYP && pPlayerPos.y > (pObjPos.y + 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 4 + i].color = sf::Color(192, 192, 192);
        if (/**/project(pVert[pIndex +  4], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  5], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  6], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  7], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 4 + i] = sf::Vertex();
    }
    else if (pBlock.visYN && pPlayerPos.y < (pObjPos.y - 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 4 + i].color = sf::Color(64, 64, 64);
        if (/**/project(pVert[pIndex +  4], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  5], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  6], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  7], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 4 + i] = sf::Vertex();
    }
    if (pBlock.visZP && pPlayerPos.z > (pObjPos.z + 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 8 + i].color = sf::Color(128, 128, 128);
        if (/**/project(pVert[pIndex +  8], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex +  9], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 10], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize) &&
                project(pVert[pIndex + 11], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z + 0.5f), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 8 + i] = sf::Vertex();
    }
    else if (pBlock.visZN && pPlayerPos.z < (pObjPos.z - 0.5f) * scale)
    {
        for (int i = 0; i < 4; i++)
            pVert[pIndex + 8 + i].color = sf::Color(128, 128, 128);
        if (/**/project(pVert[pIndex +  8], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex +  9], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y + 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 10], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x + 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize) &&
                project(pVert[pIndex + 11], pPlayerPos, pPlayerRot, sf::Vector3f(pObjPos.x - 0.5f, pObjPos.y - 0.5f, pObjPos.z - 0.5f), pWinSize));
        else
            for (int i = 0; i < 4; i++)
                pVert[pIndex + 8 + i] = sf::Vertex();
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

    pVert.position.x = pWinSize.x / 2 - (d.x * 1000.f) / -d.z;
    pVert.position.y = pWinSize.y / 2 - (d.y * 1000.f) / -d.z;
    pVert.color.a = std::fmax(0, 255 - std::fmin(255, 255.f * (std::sqrt(x * x + y * y + z * z) / fog)));

    return d.z < 0.f;
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


void collide(City& pCity, sf::Vector3f& pPos, sf::Vector3f& pVel)
{
    pVel.x *= 0.8f;
    pVel.y *= 0.8f;
    pVel.z *= 0.8f;
    pPos.x += pVel.x;
    pPos.y += pVel.y;
    pPos.z += pVel.z;

    if (pCity.getBlock(pPos.x / 10.f + 0.5f + (pVel.x > 0 ? 0.5f : -0.5f), pPos.y / 10.f + 0.5f, pPos.z / 10.f + 0.5f).isSolid)
    {
        pPos.x -= pVel.x;
        pVel.x = 0;
    }
    if (pCity.getBlock(pPos.x / 10.f + 0.5f, pPos.y / 10.f + 0.5f + (pVel.y > 0 ? 0.5f : -0.5f), pPos.z / 10.f + 0.5f).isSolid)
    {
        pPos.y -= pVel.y;
        pVel.y = 0;
    }
    if (pCity.getBlock(pPos.x / 10.f + 0.5f, pPos.y / 10.f + 0.5f, pPos.z / 10.f + 0.5f + (pVel.z > 0 ? 0.5f : -0.5f)).isSolid)
    {
        pPos.z -= pVel.z;
        pVel.z = 0;
    }
}
