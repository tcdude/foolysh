#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include "list_t.hpp"

namespace animation {
    class Interval;
    class Sequence;

    class AnimationManager {
    public:
        static tools::ExtFreeList<Interval*> intervals;
        static tools::ExtFreeList<Sequence*> sequences;
    };

    tools::ExtFreeList<Interval*> AnimationManager::intervals;
    tools::ExtFreeList<Sequence*> AnimationManager::sequences;

}  // namespace animation

#endif
