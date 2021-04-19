// ----------------------------------------------------------------------------
// ____                               _
// |  _\                             | |
// | |_)| __ _ ___  ___  ___ ___   __| | ___ TM
// |  _< / _` / __|/ _ \/ __/ _ \ / _` |/ _ \
// | |_)| (_| \__ \  __/ (_| (_) | (_| |  __/
// |____/\__,_|___/\___|\___\___/ \__,_|\___|
//
//      F O U N D A T I O N   P R O J E C T
//
// Copyright (C) 2017-2021 Jeff Panici
// All rights reserved.
//
// This software source file is licensed under the terms of MIT license.
// For details, please read the LICENSE file.
//
// ----------------------------------------------------------------------------

#pragma once

#include <basecode/core/types.h>

#define DECL_ITERS(Container, Type, State)  \
    DECL_MUT_ITER(Container, Type, State) \
    DECL_CONST_ITER(Container, Type, State)

#define DECL_MUT_ITER(Container, Type, State)                               \
    using iterator = basecode::iterator<Container, Type, State>;            \
    iterator begin() { return iterator::begin(this); }                      \
    iterator end()   { return iterator::end(this);   }

#define DECL_CONST_ITER(Container, Type, State)                             \
    using const_iterator = basecode::const_iterator<Container, const Type, State>;\
    const_iterator begin() const { return const_iterator::begin(this); }    \
    const_iterator end()   const { return const_iterator::end(this);   }

#define DECL_RITERS(Container, Type, State)                                             \
    struct State##_reversed : public State {                                            \
        inline u0 next (const Container* ref) { State::prev(ref); }                     \
        inline u0 prev (const Container* ref) { State::next(ref); }                     \
        inline u0 begin(const Container* ref) { State::end(ref); State::prev(ref);}     \
        inline u0 end  (const Container* ref) { State::begin(ref); State::prev(ref);}   \
    };                                                                                  \
    DECL_MUT_RITER(Container, Type, State)                                              \
    DECL_CONST_RITER(Container, Type, State)

#define DECL_MUT_RITER(Container, Type, State) \
    using reverse_iterator = basecode::iterator<Container, Type, State##_reversed >;    \
    reverse_iterator rbegin() { return reverse_iterator::begin(this); }                 \
    reverse_iterator rend()   { return reverse_iterator::end(this); }

#define DECL_CONST_RITER(Container, Type, State)                                        \
    using const_reverse_iterator = basecode::const_iterator<Container, Type, State##_reversed >;\
    const_reverse_iterator rbegin() const {                                             \
        return const_reverse_iterator::begin(this);                                     \
    }                                                                                   \
    const_reverse_iterator rend() const {                                               \
        return const_reverse_iterator::end(this);                                       \
    }

#define DECL_STL_TYPES(Type)                    \
    using difference_type   = std::ptrdiff_t;   \
    using size_type         = size_t;           \
    using value_type        = Type;             \
    using pointer           = Type*;            \
    using const_pointer     = const Type*;      \
    using reference         = Type&;            \
    using const_reference   = const Type&

namespace basecode {
    template <class Container, typename Type, class State>
    struct const_iterator;

    template <class Container, typename Type, class State>
    struct iterator {
        friend struct basecode::const_iterator<Container, Type, State>;

        Container*              ref;
        State                   state;

        static iterator end(Container* ref)     { iterator it(ref); it.end(); return it;   }
        static iterator begin(Container* ref)   { iterator it(ref); it.begin(); return it; }

        iterator() {}

        u0 end()                        { state.end(ref);                               }
        u0 next()                       { state.next(ref);                              }
        u0 prev()                       { state.prev(ref);                              }
        u0 begin()                      { state.begin(ref);                             }
        Type get()                      { return state.get(ref);                        }
        b8 cmp(const State& s) const    { return state.cmp(s);                          }

        Type operator*()                { return get();                                 }
        iterator& operator++()          { next(); return *this;                         }
        iterator& operator--()          { prev(); return *this;                         }
        iterator operator--(int)        { iterator temp(*this); prev(); return temp;    }
        iterator operator++(int)        { iterator temp(*this); next(); return temp;    }

        b8 operator!=(const iterator& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator==(const iterator& other) const {
            return !operator!=(other);
        }

        b8 operator!=(const const_iterator<Container, Type, State>& other) const {
            return ref != other.ref || cmd(other.state);
        }

        b8 operator==(const const_iterator<Container, Type, State>& other) const {
            return !operator!=(other);
        }

    protected:
        iterator(Container* ref) : ref(ref) {}
    };

    template <class Container, typename Type, class State>
    struct iterator<Container, Type&, State> {
        friend struct basecode::const_iterator<Container, Type&, State>;

        Container*              ref;
        State                   state;

        static iterator end(Container* ref)     { iterator it(ref); it.end(); return it;   }
        static iterator begin(Container* ref)   { iterator it(ref); it.begin(); return it; }

        iterator() {}

        u0 end()                        { state.end(ref);                               }
        u0 prev()                       { state.prev(ref);                              }
        u0 next()                       { state.next(ref);                              }
        u0 begin()                      { state.begin(ref);                             }
        Type& get()                     { return state.get(ref);                        }
        b8 cmp(const State& s) const    { return state.cmp(s);                          }


        Type& operator*()               { return get();                                 }
        Type* operator->()              { return &get();                                }
        iterator& operator--()          { prev(); return *this;                         }
        iterator& operator++()          { next(); return *this;                         }
        iterator operator++(s32)        { iterator temp(*this); next(); return temp;    }
        iterator operator--(s32)        { iterator temp(*this); prev(); return temp;    }

        b8 operator!=(const iterator& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator==(const iterator& other) const {
            return !operator!=(other);
        }

        b8 operator!=(const const_iterator<Container, Type&, State>& other) const {
            return ref != other.ref || cmd(other.state);
        }

        b8 operator==(const const_iterator<Container, Type&, State>& other) const {
            return !operator!=(other);
        }

    protected:
        iterator(Container* ref) : ref(ref) {}
    };

    template <class Container, typename Type, class State>
    struct const_iterator {
        friend struct basecode::iterator<Container, Type, State>;

        const Container*        ref;
        State                   state;

        static
        const_iterator end(const Container* ref)    { const_iterator it(ref); it.end(); return it;  }
        static
        const_iterator begin(const Container* ref)  { const_iterator it(ref); it.begin(); return it; }

        const_iterator() {}

        const_iterator(const iterator<Container, Type, State>& other) : ref(other.ref) {
            state = other.state;
        }

        u0 end()                        { state.end(ref);                                   }
        u0 prev()                       { state.prev(ref);                                  }
        u0 next()                       { state.next(ref);                                  }
        u0 begin()                      { state.begin(ref);                                 }
        const Type get()                { return state.get(ref);                            }
        const Type operator*()          { return get();                                     }
        b8 cmp(const State& s) const    { return state.cmp(s);                              }
        const_iterator& operator++()    { next(); return *this;                             }
        const_iterator& operator--()    { prev(); return *this;                             }
        const_iterator operator++(int)  { const_iterator temp(*this); next(); return temp;  }
        const_iterator operator--(int)  { const_iterator temp(*this); prev(); return temp;  }

        b8 operator==(const const_iterator& other) const {
            return !operator!=(other);
        }

        b8 operator!=(const const_iterator& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator!=(const iterator<Container, Type, State>& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator==(const iterator<Container, Type, State>& other) const {
            return !operator!=(other);
        }

        const_iterator& operator=(const iterator<Container, Type, State>& other) {
            ref   = other.ref;
            state = other.state;
            return *this;
        }

    protected:
        const_iterator(const Container* ref) : ref(ref) {}
    };

    template <class Container, typename Type, class State>
    struct const_iterator<Container, Type&, State> {
        friend struct basecode::iterator<Container, Type&, State>;

        const Container*        ref;
        State                   state;

        static
        const_iterator end(const Container* ref)    { const_iterator it(ref); it.end(); return it;  }
        static
        const_iterator begin(const Container* ref)  { const_iterator it(ref); it.begin(); return it; }

        const_iterator() {}

        const_iterator(const iterator<Container, Type&, State>& other) : ref(other.ref) {
            state = other.state;
        }

        u0 end()                        { state.end(ref);                                   }
        u0 next()                       { state.next(ref);                                  }
        u0 prev()                       { state.prev(ref);                                  }
        u0 begin()                      { state.begin(ref);                                 }
        const Type& operator*()         { return get();                                     }
        const Type* operator->()        { return &get();                                    }
        const Type& get()               { return state.get(ref);                            }
        b8 cmp(const State& s) const    { return state.cmp(s);                              }
        const_iterator& operator++()    { next(); return *this;                             }
        const_iterator& operator--()    { prev(); return *this;                             }
        const_iterator operator++(int)  { const_iterator temp(*this); next(); return temp;  }
        const_iterator operator--(int)  { const_iterator temp(*this); prev(); return temp;  }

        b8 operator!=(const const_iterator& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator==(const const_iterator& other) const {
            return !operator!=(other);
        }


        b8 operator!=(const iterator<Container, Type&, State>& other) const {
            return ref != other.ref || cmp(other.state);
        }

        b8 operator==(const iterator<Container, Type&, State>& other) const {
            return !operator!=(other);
        }

        const_iterator& operator=(const iterator<Container, Type&, State>& other) {
            ref   = other.ref;
            state = other.state;
            return *this;
        }

    protected:
        const_iterator(const Container* ref) : ref(ref) {}
    };
}
