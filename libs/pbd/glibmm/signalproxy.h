#ifndef ARDOUR_GLIBMM_SIGNALPROXY_H
#define ARDOUR_GLIBMM_SIGNALPROXY_H

#include_next <glibmm/signalproxy.h>

namespace Glib
{

template <typename R = void>
using SignalProxy0 = SignalProxy<R()>;

template <typename R, typename A1>
using SignalProxy1 = SignalProxy<R(A1)>;

template <typename R, typename A1, typename A2>
using SignalProxy2 = SignalProxy<R(A1, A2)>;

template <typename R, typename A1, typename A2, typename A3>
using SignalProxy3 = SignalProxy<R(A1, A2, A3)>;

template <typename R, typename A1, typename A2, typename A3, typename A4>
using SignalProxy4 = SignalProxy<R(A1, A2, A3, A4)>;

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5>
using SignalProxy5 = SignalProxy<R(A1, A2, A3, A4, A5)>;

template <typename R, typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
using SignalProxy6 = SignalProxy<R(A1, A2, A3, A4, A5, A6)>;

} // namespace Glib

#endif
