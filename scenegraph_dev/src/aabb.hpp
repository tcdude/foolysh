/**
 * Simple 2D Axis Aligned Bounding Box implementation.
 */

#ifndef AABB_HPP
#define AABB_HPP


namespace scenegraph {
    enum Quadrant {
        TL,
        TR,
        BL,
        BR
    };
    class AABB {
    public:
        AABB();
        AABB(double _x, double _y, double _hw, double _hh);

        bool inside(const AABB& aabb);
        bool inside(double _x, double _y);
        bool overlap(const AABB& aabb);
        AABB split(Quadrant _q);
        AABB split(double _x, double _y, Quadrant _q);
        Quadrant find_quadrant(double _x, double _y);

        bool operator==(const AABB& rhs) {
            return (x == rhs.x && y == rhs.y && hw == rhs.hw && hh == rhs.hh);
        }

        double x, y, hw, hh;
    };
}  // namespace scenegraph
#endif
