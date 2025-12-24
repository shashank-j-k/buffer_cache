# Buffer Cache Implementation (C)

## Overview
This project is a Unix-inspired **buffer cache simulation** implemented in C.
It demonstrates how operating systems manage disk blocks in memory using
**hash queues** for fast lookup and a **free list** for buffer allocation.

The implementation focuses on low-level concepts such as pointer manipulation,
dynamic memory management, and linked list operations.

---

## Concepts Used
- Pointers and dynamic memory allocation
- Linked lists
- Hash queues
- Free list management
- Hash collision handling
- OS-level buffer cache concepts

---

## Design Overview
- Buffers are indexed using a **hash function** and stored in **hash queues**
  for efficient lookup.
- A **free list** maintains available buffers.
- When a buffer is requested:
  - The corresponding hash queue is searched.
  - If present, the buffer is reused.
  - If not present, a buffer is taken from the free list and inserted into the
    appropriate hash queue.
- Careful pointer updates maintain consistency between hash queues and the free list.

This design closely follows classical Unix buffer cache principles.

---
