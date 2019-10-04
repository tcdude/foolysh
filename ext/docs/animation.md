# The Animation System in foolysh

This document outlines the Animation System in foolysh. It allows for different
types of operations to animate a Node in the Scenegraph each frame. It features
four distinct but related classes in the API.

> **Warning:**
>
> This is a design document, where I hash out ideas to come up with a reasonable
> API design for the matter at hand. This is neither part of the documentation
> nor will it be the final API specification!


## Content

1. [Desired Functionality](#desired-functionality)
    1. [easy](#easy)
    1. [automated](#automated)
    1. [parametric](#parametric)
1. [Structure](#structure)
    1. [AnimationManager](#animationmanager)
    1. [Sequence](#sequence)
    1. [Animation](#animation)
    1. [Interval](#interval)
1. [Out of scope and potential extensions](#out-of-scope-and-potential-extensions)

## Desired Functionality

The Animation System should allow for [easy](#easy), [automated](#automated) and
[parametric](#parametric) manipulation of Nodes in the Scenegraph. The major
goal is to create an API that is short and self explanatory for the user when
creating or revisiting animation in their projects.


### easy

For mundane animations, the API should provide convenience functions, that
allow to accomplish an animation in one line

**Pseudo-Code Example:**

```python
# Create an Interval that moves "my_node" to "destination_node" in 2 seconds:
ival1 = pos_interval(node=my_node, duration=2, stop=destination_node)

# Add rotation around its current rotation center for 720° at 180° per
# second to "ival", created above:
ival1 += rot_animation(node=my_node, speed=180, stop=720)

# Create a second interval, using a combined interval with blending:
ival2 = pos_scale_interval(
  node=my_node,
  duration=5,
  stop=(Vector2(20, 10), 120),
  blend_flags=Blend.IN|Blend.OUT
)

# Combine multiple intervals to a Sequence:
seq = Sequence(
  ival1,
  ival2
)

# Animate the sequence, starting with the next frame update:
seq.play()
```


### automated

The user doesn't have to do any housekeeping on the Animation System. The
Animation System cleans up after itself and takes care of potential conflicts:

- e.g. when the user generated code requests a new animation, that the Animation
  System automatically either replaces or extends any existing animations on
  a Node, depending on the conflict.

To keep the user in the loop, proper logging takes place, where warnings and
information will be logged, when conflicts are automatically resolved. This
should make it easier for the user when debugging code that uses the Animation
System.


### parametric

The Animation System should provide fine grained control over every aspect
involving an Animation. Next to the convenience functions mentioned earlier,
the API must provide seamless access to the underlying classes, that are also
being used by those convenience functions. It should also allow for easy
replication of animations, if the user chooses for example to apply the same
animation on multiple Nodes simultaneously.


## Structure

![see /images/animation_structure.png][animation-structure]


### AnimationManager

Responsible to execute single steps of [Sequence](#sequence) /
[Interval](#interval) instances. Animations are registered and controlled
through the AnimationManager. AnimationManager is allowed to manipulate Node
instances directly.

Provides convenience methods for creating Animations.


### Sequence

Represents a sequence of [Animation](#animation) or [Interval](#interval)
instances in a user defined order. Exposes an interface for the
[AnimationManager](#animationmanager) to execute animation steps and request
necessary Node manipulations.


### Animation

Represents a parametric Node manipulation with the possibility of specifying:

  * **Speed:** e.g. movement speed for position
  * **Blend Type:** None, ease_in, ease_out, ease_int_out
  * *[Start-]* / **End-State:** e.g. start and end position (can also be a node)
  * **Relative to Node:** whether the states supplied should be applied relative
    to another Node

Animations get transformed automatically into an [Interval](#interval), which
is the base component of the entire Animation System.


### Interval

Represents a parametric Node manipulation with the possibility of specifying:

  * **Duration:** e.g. movement speed for position
  * **Blend Type:** None, ease_in, ease_out, ease_int_out
  * *[Start-]* / **End-State:** e.g. start and end position (can also be a node)
  * **Relative to Node:** whether the states supplied should be applied relative
    to another Node

Intervals represent the base component of the Animation System out of which
complex animations can be constructed.


## Out of scope and potential extensions

- **Curves / Splines:** My math skills are not there yet, but could be added
  in a future release.

  *To future-me:* Read up on
  [quadratic equations](https://en.wikipedia.org/wiki/Quadratic_equation) and
  [B-spline](https://en.wikipedia.org/wiki/B-spline)
- **Acc-/Deceleration for Animation:** Out of scope, but probably something that
  cannot be easily added with the current structure. Also probably not needed,
  since the lerp blending used in ease_in/out already does a great job and any
  significantly more complex type of Animation/Node manipulation will likely be
  handled outside the Animation System.

<!-- References -->
[animation-structure]: animation_structure.png "Structure of the animation System"