#ifndef LIB_PINOCCIO_CALLBACK_H
#define LIB_PINOCCIO_CALLBACK_H

namespace pinoccio {

/**
 * These classes provide easy and efficient lists of callbacks. To use
 * them, declare a callback list, using the return type and any
 * arguments to all callbacks as template parameters. For example, to
 * store list of void func(bool, int) pointers:
 *
 * CallbackList<void, bool, int> fooCallbacks;
 *
 * Then, to add a callback to the list, first turn it into a Callback
 * object using the create_callback function. The callback should be
 * stored in a global or static variable, so it remains valid for as
 * long as it is in the CallbackList.
 *
 * The resulting object can then be passed to CallbackList::append() (or
 * prepend). For example, you could have:
 *
 * void doFoo(bool, int);
 *
 * void setup() {
 *   static auto callback = create_callback(doFoo);
 *   fooCallbacks.append(callback);
 *
 * Note the use of the auto keyword to prevent having to type the full
 * Callback<void, bool, int> type.
 *
 * Calling the callbacks in the list is simple:
 *
 * fooCallbacks.callAll(true, 10);
 */

/**
 * An object wrapping a function, for easy insertion in a linked List
 * (CallbackList). In particular, this combines the function pointer
 * with the next pointer. By allocating a Callback object statically, it
 * can be added to a list without needing any dynamic allocation.
 */
template<typename Ret, typename... Args>
struct Callback {
  // The function type this callback wraps
  typedef Ret (f_t)(Args...);

  f_t * const f;
  Callback *next;

  // Do not add constructors, that prevents the compiler from completely
  // initializing the object at compiletime.
};

/**
 * Helper function to efficiently create a Callback object. Over a
 * constructor, this serves two purposes:
 *  - Because it is a function, it can autodeduce template arguments.
 *  - Because it uses an explicit initializer, the compiler can
 *    completely resolve it at compile time. In particular, when storing
 *    the result in a static variable inside a function, the compiler
 *    can initialize it at compile time, instead of delaying that to the
 *    first function call.
 */
template<typename Ret, typename... Args>
constexpr Callback<Ret, Args...> build_callback(Ret (*f)(Args...)) {
  return {f, NULL};
}

/**
 * A list of callbacks.
 */
template<typename Ret, typename... Args>
class CallbackList {
  public:
    typedef Callback<Ret, Args...> C_t;

    /**
     * Call all callbacks in the list with the given arguments.
     */
    void callAll(Args... args) {
      C_t *c = _first;
      while(c) {
        c->f(args...);
        c = c->next;
      }
    }

    /**
     * Prepend a callback to this list.
     *
     * A reference to the callback passed is stored, so it is up to the
     * caller to ensure it stays valid.
     */
    void prepend(C_t &c) {
      prepend(&c);
    }

    void prepend(C_t *c) {
      c->next = _first;
      _first = c;
    }

    /**
     * Append a callback to this list.
     *
     * A reference to the callback passed is stored, so it is up to the
     * caller to ensure it stays valid.
     */
    void append(C_t &c) {
      append(&c);
    }

    void append(C_t *c) {
      C_t **i = &_first;
      while(*i) {
        i = &((*i)->next);
      }
      c->next = NULL;
      *i = c;
    }

    /**
     * Return the first callback in the list. Loop over the rest using
     * the Callback's next pointer.
     */
    const C_t* first() { return _first; }

  private:
    C_t *_first = NULL;
};

} // namespace pinoccio

#endif // LIB_PINOCCIO_CALLBACK_H
