#include "interval.hpp"

#include "node.hpp"
#include "vector2.hpp"

#include <algorithm>
#include <stdexcept>


/**
 *
 */
animation::Interval::
Interval(const scenegraph::Node& node, const double d, BlendType bt)
    : _duration(d), _blend_type(bt), _node_ptr(new scenegraph::Node(node)) {
    _type_flags = 0;
    _delta = _t = 0.0;
}

/**
 *
 */
animation::Interval::Interval(const Interval& other)
    : _duration(other._duration), _blend_type(other._blend_type),
      _node_ptr(new scenegraph::Node(*other._node_ptr)) {
    if (other._other_ptr) {
        _other_ptr.reset(new scenegraph::Node(*other._other_ptr));
    }
    _type_flags = other._type_flags;
    _delta = other._delta;
    _t = other._t;
}

/**
 * Add position component to Interval.
 */
void animation::Interval::
add_position(const tools::Vector2& start, const tools::Vector2& end) {
    _pos_ptr.reset(new Position());
    _pos_ptr->start = start;
    _pos_ptr->end = end;
    _type_flags = _type_flags | 1;
}

/**
 * Add only end position component to Interval, start = current position.
 */
void animation::Interval::
add_position(const tools::Vector2& start, const tools::Vector2& end) {
    _pos_ptr.reset(new Position());
    _pos_ptr->start = _node_ptr->get_pos();
    _pos_ptr->end = end;
    _type_flags = _type_flags | 1;
}

/**
 * Add origin Node to use in position Interval.
 */
void animation::Interval::
add_node_origin(const scenegraph::Node& other) {
    _other_ptr.reset(new scenegraph::Node(other));
}

/**
 * Add scale component to Interval as scalar.
 */
void animation::Interval::
add_scale(const double start, const double end) {
    if (_type_flags & 4 > 0 || _type_flags & 8 > 0) {
        throw std::logic_error("SCALE_X or SCALE_Y already set for Interval");
    }
    _scale_ptr.reset(new Scalar());
    _scale_ptr->start = start;
    _scale_ptr->end = end;
    _type_flags = _type_flags | 2;
}

/**
 * Add scale component to Interval as scalar for X-Axis only.
 */
void animation::Interval::
add_scale_x(const double start, const double end) {
    if (_type_flags & 2 > 0) {
        throw std::logic_error("SCALE already set for Interval");
    }
    _sx_ptr.reset(new Scalar());
    _sx_ptr->start = start;
    _sx_ptr->end = end;
    _type_flags = _type_flags | 4;
}

/**
 * Add scale component to Interval as scalar for Y-Axis only.
 */
void animation::Interval::
add_scale_y(const double start, const double end) {
    if (_type_flags & 2 > 0) {
        throw std::logic_error("SCALE already set for Interval");
    }
    _sy_ptr.reset(new Scalar());
    _sy_ptr->start = start;
    _sy_ptr->end = end;
    _type_flags = _type_flags | 8;
}

/**
 * Add scale component to Interval as two scalars, representing X/Y-Axis.
 */
void animation::Interval::
add_scale(const double sx, const double sy, const double ex, const double ey) {
    if (_type_flags & 2 > 0) {
        throw std::logic_error("SCALE already set for Interval");
    }
    _sx_ptr.reset(new Scalar());
    _sy_ptr.reset(new Scalar());
    _sx_ptr->start = sx;
    _sy_ptr->start = sy;
    _sx_ptr->end = ex;
    _sy_ptr->end = ey;
    _type_flags = _type_flags | 4;
    _type_flags = _type_flags | 8;
}

/**
 * Add rotation component to Interval in degrees.
 */
void animation::Interval::
add_rotation(const double start, const double end) {
    _rot_ptr.reset(new Scalar());
    _rot_ptr->start = start;
    _rot_ptr->end = end;
    _type_flags = _type_flags | 16;
}

/**
 * Add rotation center component to Interval.
 */
void animation::Interval::
add_rotation_center(const tools::Vector2& start, const tools::Vector2& end) {
    _rot_center_ptr.reset(new Position());
    _rot_center_ptr->start = start;
    _rot_center_ptr->end = end;
    _type_flags = _type_flags | 32;
}

/**
 * Return the remaining time of the Interval and update all components of the
 * Interval on the Node.
 */
double animation::Interval::
execute_step(const double dt) {
    _delta += dt;
    const double ret = _duration - _delta;
    if (ret < 0) {
        _delta = _duration;
    }

    if (_duration == 0.0) {
        _t = 1.0;
    }
    else {
        _t = _delta / _duration;
        _t = std::min(std::max(_t, 0.0), 1.0);

        switch (_blend_type) {
            case EASE_IN: {
                double t2 = _t * _t;
                _t = ((3.0 * t2) - (t2 * _t)) * 0.5;
                break;
            }
            case EASE_OUT: {
                double t2 = _t * _t;
                _t = ((3.0 * _t) - (t2 * _t)) * 0.5;
                break;
            }
            case EASE_IN_OUT: {
                double t2 = _t * _t;
                _t = (3.0 * t2) - (2.0 * _t * t2);
                break;
            }
        }
    }
    if (_type_flags & 1 > 0) {
        tools::Vector2 pos = get_position();
        _node_ptr->set_pos(pos);
    }

    if (_type_flags & 2 > 0) {

        _node_ptr->set_scale(get_scale());
    }
    else {
        scenegraph::Scale s = _node_ptr->get_scale();
        if (_type_flags & 4 > 0) {
            s.sx = get_scale_x();
        }
        if (_type_flags & 8 > 0) {
            s.sy = get_scale_y();
        }
        _node_ptr->set_scale(s);
    }

    if (_type_flags & 16 > 0) {
        _node_ptr->set_angle(get_rotation());
    }

    if (_type_flags & 32 > 0) {
        tools::Vector2 c = get_rotation_center();
        _node_ptr->set_rotation_center(c);
    }

    return ret;
}

/**
 * Reset Interval.
 */
void animation::Interval::
reset() {
    _delta = _t = 0.0;
}

/**
 * Return current position.
 */
tools::Vector2 animation::Interval::
get_position() {
    if (_type_flags & 1 == 0) {
        throw std::logic_error("Not a Position Interval");
    }
    if (!_pos_ptr) {
        throw std::runtime_error("_pos_ptr not allocated");
    }

    tools::Vector2 rel_offset;
    if (_other_ptr) {
        rel_offset = _node_ptr->get_relative_pos() - _other_ptr->get_relative_pos();
    }

    if (_t == 0.0) {
        return _pos_ptr->start + rel_offset;
    }
    else if (_t == 1.0) {
        return _pos_ptr->end + rel_offset;
    }
    tools::Vector2 delta = (_pos_ptr->end - _pos_ptr->start) * _t;
    return _pos_ptr->start + delta + rel_offset;
}

/**
 * Return current scale.
 */
double animation::Interval::
get_scale() const {
    if (_type_flags & 2 == 0) {
        throw std::logic_error("Not a Scale Interval");
    }
    if (!_scale_ptr) {
        throw std::runtime_error("_scale_ptr not allocated");
    }

    if (_t == 0.0) {
        return _scale_ptr->start;
    }
    else if (_t == 1.0) {
        return _scale_ptr->end;
    }
    const double delta = (_scale_ptr->end - _scale_ptr->start) * _t;
    return _scale_ptr->start + delta;
}

/**
 * Return current X-Axis scale.
 */
double animation::Interval::
get_scale_x() const {
    if (_type_flags & 4 == 0) {
        throw std::logic_error("Not a Scale X Interval");
    }
    if (!_sx_ptr) {
        throw std::runtime_error("_sx_ptr not allocated");
    }

    if (_t == 0.0) {
        return _sx_ptr->start;
    }
    else if (_t == 1.0) {
        return _sx_ptr->end;
    }
    const double delta = (_sx_ptr->end - _sx_ptr->start) * _t;
    return _sx_ptr->start + delta;
}

/**
 * Return current Y-Axis scale.
 */
double animation::Interval::
get_scale_y() const {
    if (_type_flags & 8 == 0) {
        throw std::logic_error("Not a Scale Y Interval");
    }
    if (!_sy_ptr) {
        throw std::runtime_error("_sy_ptr not allocated");
    }

    if (_t == 0.0) {
        return _sy_ptr->start;
    }
    else if (_t == 1.0) {
        return _sy_ptr->end;
    }
    const double delta = (_sy_ptr->end - _sy_ptr->start) * _t;
    return _sy_ptr->start + delta;
}

/**
 * Return current rotation.
 */
double animation::Interval::
get_rotation() const {
    if (_type_flags & 16 == 0) {
        throw std::logic_error("Not a Rotation Interval");
    }
    if (!_rot_ptr) {
        throw std::runtime_error("_rot_ptr not allocated");
    }

    if (_t == 0.0) {
        return _rot_ptr->start;
    }
    else if (_t == 1.0) {
        return _rot_ptr->end;
    }
    const double delta = (_rot_ptr->end - _rot_ptr->start) * _t;
    return _rot_ptr->start + delta;
}

/**
 * Return current rotation center.
 */
tools::Vector2 animation::Interval::
get_rotation_center() const {
    if (_type_flags & 32 == 0) {
        throw std::logic_error("Not a Rotation Center Interval");
    }
    if (!_rot_center_ptr) {
        throw std::runtime_error("_rot_center_ptr not allocated");
    }

    if (_t == 0.0) {
        return _rot_center_ptr->start;
    }
    else if (_t == 1.0) {
        return _rot_center_ptr->end;
    }
    tools::Vector2 delta = (_rot_center_ptr->end - _rot_center_ptr->start) * _t;
    return _rot_center_ptr->start + delta;
}
