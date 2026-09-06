# TPIN MALLOC

> tpin is short for terrapin which is an username I use sometimes for games

### About

I've wanted to write my own custom memory allocator for a while so here it is

This malloc does not intend to replace any of the existing mallocs, it is purely a learning project. Although I intend to keep it simple for now I don't rule out adding support for multiple sized bins, fastbins or even compatibility for POSIX threads ~~not sure about the last one though...~~

This should be obvious for anyone but this malloc is heavily inspired on other mallocs, specially dlmalloc (see [references](#references))

**Objectives**:
- Return usable data chunks of arbitrary sizes to the user
- Let the user free a chunk of data
- Be able to reuse free'd chunks for new allocations

### References
- *Computer Systems: A Programmer's Perspective* - For boundary tags and other useful low level data manipulation
- *The Linux Programming Interface* - For syscalls
- [*Glibc wiki's malloc internals*](https://sourceware.org/glibc/wiki/MallocInternals)
- [dlmalloc](https://gee.cs.oswego.edu/dl/html/malloc.html)

