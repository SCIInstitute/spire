Spire
=====

[![Build Status](https://travis-ci.org/SCIInstitute/spire.png)](https://travis-ci.org/SCIInstitute/spire)

Built as the primary renderer for SCIRun5, Spire is evolving into a thin
interface to desktop OpenGL 2.0+ and OpenGL ES 2.0+.

| Warning                                                                            |
| ---------------------------------------------------------------------------------- |
| Spire is pre-alpha software and its interface will change between now and `1.0.0`. |

Usage
-----

### Interface

```c++
#include <spire/Interface.h>
```

See
[documentation](http://sciinstitute.github.io/spire/class_c_p_m___s_p_i_r_e___n_s_1_1_interface.html).

### Context

```c++
#include <spire/Context.h>
```

The context class is always included via `Interface.h`. Derive and implement
the interface from this abstract class in order to provide Spire with a usable
context.

| Note          |
| ------------- |
| This abstract context class will be removed in future versions of Spire. |

See
[documentation](http://sciinstitute.github.io/spire/class_c_p_m___s_p_i_r_e___n_s_1_1_context.html).

Examples
--------

A number of CPM modules use spire. Below are a list of some source examples.
You may also refer to the spire unit tests.

