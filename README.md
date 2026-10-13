# channel.hpp

An implementation of an MPMC (broadcast) channel that I wrote for [catwire](https://github.com/nktauserum/catwire). It went unused after an architectural redesign, but I didn't want to just delete the source code.

It is nearly lock-free; the only blocking feature is a futex on the consumer side to wait for new messages.
