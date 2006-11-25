#ifndef POSITION_H
#define POSITION_H

namespace LucED {

class Position
{
public:
    Position()
        : x(0), y(0), w(0), h(0) {}

    Position(int x, int y, int width, int height)
        : x(x), y(y), w(width), h(height) {}

    bool operator==(const Position& p) const {
        return x == p.x && y ==p.y && w == p.w && h == p.h;
    }

    bool operator!=(const Position& p) const {
        return !operator==(p);
    }

    int x, y;
    int w, h;
};
    
} // namespace LucED

#endif // POSITION_H
