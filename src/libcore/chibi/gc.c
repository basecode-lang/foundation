/*  gc.c -- simple mark&sweep garbage collector               */
/*  Copyright (c) 2009-2015 Alex Shinn.  All rights reserved. */
/*  BSD-style license: http://synthcode.com/license.txt       */

#if ! SEXP_USE_BOEHM && ! SEXP_USE_MALLOC

#include "chibi/sexp.h"

#if SEXP_USE_TIME_GC
#include <sys/resource.h>
#endif

#if SEXP_USE_MMAP_GC
#include <sys/mman.h>
#endif

#define SEXP_BANNER(x) ("**************** GC "x"\n")

#define SEXP_MINIMUM_OBJECT_SIZE (sexp_heap_align(1))

#if SEXP_USE_GLOBAL_HEAP
sexp_heap sexp_global_heap;
#endif

#if SEXP_USE_CONSERVATIVE_GC
static sexp* stack_base;
#endif

#if SEXP_USE_DEBUG_GC
#define sexp_debug_printf(fmt, ...) fprintf(stderr, SEXP_BANNER(fmt),__VA_ARGS__)
#else
#define sexp_debug_printf(fmt, ...)
#endif

static sexp_heap sexp_heap_last (sexp_heap h) {
  while (h->next) h = h->next;
  return h;
}

#if !SEXP_USE_FIXED_CHUNK_SIZE_HEAPS
static size_t sexp_heap_total_size (sexp_heap h) {
  size_t total_size = 0;
  for (; h; h=h->next)
    total_size += h->size;
  return total_size;
}
#endif

#if ! SEXP_USE_GLOBAL_HEAP
#if SEXP_USE_DEBUG_GC
void sexp_debug_heap_stats (sexp_heap heap) {
  sexp_free_list ls;
  size_t available = 0;
  for (ls=heap->free_list; ls; ls=ls->next)
    available += ls->size;
#if SEXP_USE_FIXED_CHUNK_SIZE_HEAPS
  sexp_debug_printf("free heap: %p (chunk size: %lu): %ld / %ld used (%.2f%%)", heap, heap->chunk_size, heap->size - available, heap->size, 100*(heap->size - available) / (float)heap->size);
#else
  sexp_debug_printf("free heap: %p: %ld / %ld used (%.2f%%)", heap, heap->size - available, heap->size, 100*(heap->size - available) / (float)heap->size);
#endif
  if (heap->next)
    sexp_debug_heap_stats(heap->next);
}
#endif

#if SEXP_USE_TRACK_ALLOC_TIMES
void sexp_debug_alloc_times(sexp ctx) {
  double mean = (double) sexp_context_alloc_usecs(ctx) / sexp_context_alloc_count(ctx);
  double var = (double) sexp_context_alloc_usecs_sq(ctx) / sexp_context_alloc_count(ctx) - mean*mean;
  fprintf(stderr, SEXP_BANNER("alloc: mean: %0.3lfμs var: %0.3lfμs (%ld times)"), mean, var, sexp_context_alloc_count(ctx));
}
#endif

#if SEXP_USE_TRACK_ALLOC_SIZES
void sexp_debug_alloc_sizes(sexp ctx) {
  int i;
  fprintf(stderr, "alloc size histogram: {");
  for (i=0; i<SEXP_ALLOC_HISTOGRAM_BUCKETS; ++i) {
    if ((i+1)*sexp_heap_align(1)<100 || sexp_context_alloc_histogram(ctx)[i]>0)
      fprintf(stderr, "  %ld:%ld", (i+1)*sexp_heap_align(1), sexp_context_alloc_histogram(ctx)[i]);
  }
  fprintf(stderr, "}\n");
}
#endif

void sexp_free_heap (void* alloc, sexp_heap heap) {
#if SEXP_USE_MMAP_GC
  munmap(heap, sexp_heap_pad_size(heap->size));
#else
  sexp_free(alloc, heap);
#endif
}
#endif

#if SEXP_USE_LIMITED_MALLOC
static sexp_sint_t allocated_bytes=0, max_allocated_bytes=-1;
void* sexp_malloc(size_t size) {
  char* max_alloc;
  void* res;
  if (max_allocated_bytes < 0) {
    max_alloc = getenv("CHIBI_MAX_ALLOC");
    max_allocated_bytes = max_alloc ? atoi(max_alloc) : 8192000; /* 8MB */
  }
  if (max_allocated_bytes > 0 && allocated_bytes + size > max_allocated_bytes)
    return NULL;
  if (!(res = malloc(size))) return NULL;
  allocated_bytes += size;
  return res;
}
/* TODO: subtract freed memory from max_allocated_bytes */
void sexp_free(void* ptr) {
  free(ptr);
}
#endif

void sexp_preserve_object(sexp ctx, sexp x) {
  sexp_global(ctx, SEXP_G_PRESERVATIVES) = sexp_cons(ctx, x, sexp_global(ctx, SEXP_G_PRESERVATIVES));
}

void sexp_release_object(sexp ctx, sexp x) {
  sexp ls1, ls2;
  for (ls1=NULL, ls2=sexp_global(ctx, SEXP_G_PRESERVATIVES); sexp_pairp(ls2);
       ls1=ls2, ls2=sexp_cdr(ls2))
    if (sexp_car(ls2) == x) {
      if (ls1) sexp_cdr(ls1) = sexp_cdr(ls2);
      else sexp_global(ctx, SEXP_G_PRESERVATIVES) = sexp_cdr(ls2);
      break;
    }
}

SEXP_API sexp_uint_t sexp_allocated_bytes (sexp ctx, sexp x) {
  sexp_uint_t res;
  sexp t;
  if (!sexp_pointerp(x) || (sexp_pointer_tag(x) >= sexp_context_num_types(ctx)))
    return sexp_heap_align(1);
  t = sexp_object_type(ctx, x);
  res = sexp_type_size_of_object(t, x) + SEXP_GC_PAD;
#if SEXP_USE_DEBUG_GC
  if (res == 0) {
    fprintf(stderr, SEXP_BANNER("%p zero-size object: %p (type tag: %d)"), ctx, x, sexp_pointer_tag(x));
    return 1;
  }
#endif
  return res;
}

#if SEXP_USE_SAFE_GC_MARK

#if SEXP_USE_DEBUG_GC > 2
int sexp_valid_heap_position(sexp ctx, sexp_heap h, sexp x) {
  sexp p = sexp_heap_first_block(h), end = sexp_heap_end(h);
  sexp_free_list q = h->free_list, r;
  while (p < end) {
    for (r=q->next; r && ((char*)r<(char*)p); q=r, r=r->next)
      ;
    if ((char*)r == (char*)p) {
      p = (sexp) (((char*)p) + r->size);
      continue;
    }
    if (p == x) {
      return 1;
    } else if (p > x) {
      fprintf(stderr, SEXP_BANNER("bad heap position: %p free: %p-%p : %p-%p"),
              x, q, ((char*)q)+q->size, r, ((char*)r)+r->size);
      return 0;
    }
    p = (sexp) (((char*)p)+sexp_heap_align(sexp_allocated_bytes(ctx, p)));
  }
  fprintf(stderr, SEXP_BANNER("bad heap position: %p heap: %p-%p"), x, h, end);
  return 0;
}
#else
#define sexp_valid_heap_position(ctx, h, x) 1
#endif

int sexp_in_heap_p(sexp ctx, sexp x) {
  sexp_heap h;
  if ((sexp_uint_t)x & (sexp_heap_align(1)-1)) {
    fprintf(stderr, SEXP_BANNER("invalid heap alignment: %p"), x);
    return 0;
  }
  for (h=sexp_context_heap(ctx); h; h=h->next)
    if (((sexp)h < x) && (x < (sexp)(h->data + h->size)))
      return sexp_valid_heap_position(ctx, h, x);
  fprintf(stderr, SEXP_BANNER("invalid object outside heap: %p"), x);
  return 0;
}
#endif

#if SEXP_USE_DEBUG_GC > 1
int sexp_valid_object_type_p (sexp ctx, sexp x) {
  if (sexp_pointer_tag(x)<=0 || sexp_pointer_tag(x)>sexp_context_num_types(ctx)){
    fprintf(stderr, SEXP_BANNER("%p mark: bad object at %p: tag: %d"),
            ctx, x, sexp_pointer_tag(x));
    return 0;
  }
  return 1;
}
#else
#define sexp_valid_object_type_p(ctx, x) 1
#endif

#if SEXP_USE_HEADER_MAGIC
int sexp_valid_header_magic_p (sexp ctx, sexp x) {
  if (sexp_pointer_magic(x)  != SEXP_POINTER_MAGIC
      && sexp_pointer_tag(x) != SEXP_TYPE && sexp_pointer_tag(x) != SEXP_OPCODE
      && sexp_pointer_tag(x) != SEXP_CORE && sexp_pointer_tag(x) != SEXP_STACK) {
    fprintf(stderr, SEXP_BANNER("%p mark: bad magic at %p: %x"),
            ctx, x, sexp_pointer_magic(x));
    return 0;
  }
  return 1;
}
#else
#define sexp_valid_header_magic_p(ctx, x) 1
#endif

#if SEXP_DEBUG_GC > 1 || SEXP_USE_SAFE_GC_MARK || SEXP_USE_HEADER_MAGIC
int sexp_valid_object_p (sexp ctx, sexp x) {
  return sexp_in_heap_p(ctx, x) && sexp_valid_object_type_p(ctx, x)
    && sexp_valid_header_magic_p(ctx, x);
}
#define sexp_gc_pass_ctx(x) x,
#else
#define sexp_gc_pass_ctx(x)
#endif

static void sexp_mark_stack_push (sexp ctx, sexp *start, sexp *end) {
  struct sexp_mark_stack_ptr_t *stack = sexp_context_mark_stack(ctx);
  struct sexp_mark_stack_ptr_t **ptr = &sexp_context_mark_stack_ptr(ctx);
  struct sexp_mark_stack_ptr_t *old = *ptr;

  if (old == NULL) {
    *ptr = stack;
  } else if (old >= stack && old + 1 < stack + SEXP_MARK_STACK_COUNT) {
    (*ptr)++;
  } else {
    *ptr = sexp_malloc(sexp_context_alloc(ctx), sizeof(**ptr));
  }

  (*ptr)->start = start;
  (*ptr)->end = end;
  (*ptr)->prev = old;
}

static void sexp_mark_stack_pop (sexp ctx) {
  struct sexp_mark_stack_ptr_t *stack = sexp_context_mark_stack(ctx);
  struct sexp_mark_stack_ptr_t *old = sexp_context_mark_stack_ptr(ctx);

  sexp_context_mark_stack_ptr(ctx) = old->prev;
  if (!(old >= stack && old < stack + SEXP_MARK_STACK_COUNT)) {
    sexp_free(sexp_context_alloc(ctx), old);
  }
}

static void sexp_mark_one (sexp ctx, sexp* types, sexp x) {
  sexp_sint_t len;
  sexp t, *p, *q;
  struct sexp_gc_var_t *saves;
 loop:
  if (!x || !sexp_pointerp(x) || !sexp_valid_object_p(ctx, x) || sexp_markedp(x))
    return;
  sexp_markedp(x) = 1;
  if (sexp_contextp(x)) {
    for (saves=sexp_context_saves(x); saves; saves=saves->next)
      if (saves->var) sexp_mark_one(ctx, types, *(saves->var));
  }
  t = types[sexp_pointer_tag(x)];
  len = sexp_type_num_slots_of_object(t, x) - 1;
  if (len >= 0) {
    p = (sexp*) (((char*)x) + sexp_type_field_base(t));
    q = p + len;
    while (p < q && (*q && sexp_pointerp(*q) ? sexp_markedp(*q) : 1))
      q--;                      /* skip trailing immediates */
    while (p < q && *q == q[-1])
      q--;                      /* skip trailing duplicates */
    if (p < q) {
      sexp_mark_stack_push(ctx, p, q);
    }
    x = *q;
    goto loop;
  }
}

static void sexp_mark_one_start (sexp ctx, sexp* types, sexp x) {
  struct sexp_mark_stack_ptr_t **ptr = &sexp_context_mark_stack_ptr(ctx);
  sexp *p, *q;
  sexp_mark_one(ctx, types, x);

  while (*ptr) {
    p = (*ptr)->start;
    q = (*ptr)->end;
    sexp_mark_stack_pop(ctx);
    while (p < q) {
      sexp_mark_one(ctx, types, *p++);
    }
  }
}

void sexp_mark (sexp ctx, sexp x) {
  sexp_mark_one_start(ctx, sexp_vector_data(sexp_global(ctx, SEXP_G_TYPES)), x);
}

#if SEXP_USE_CONSERVATIVE_GC

int stack_references_pointer_p (sexp ctx, sexp x) {
  sexp *p;
  for (p=(&x)+1; p<stack_base; p++)
    if (*p == x)
      return 1;
  return 0;
}

#if SEXP_USE_TRACK_ALLOC_BACKTRACE
void sexp_print_gc_trace(sexp ctx, sexp p) {
  int i;
  char **debug_text = backtrace_symbols(p->backtrace, SEXP_BACKTRACE_SIZE);
  for (i=0; i < SEXP_BACKTRACE_SIZE; i++)
    fprintf(stderr, SEXP_BANNER("    : %s"), debug_text[i]);
  sexp_free(debug_text);
}
#else
#define sexp_print_gc_trace(ctx, p)
#endif

void sexp_conservative_mark (sexp ctx) {
  sexp_heap h = sexp_context_heap(ctx);
  sexp p, end;
  sexp_free_list q, r;
  for ( ; h; h=h->next) {   /* just scan the whole heap */
    p = sexp_heap_first_block(h);
    q = h->free_list;
    end = sexp_heap_end(h);
    while (p < end) {
      for (r=q->next; r && ((char*)r<(char*)p); q=r, r=r->next)
        ;
      if ((char*)r == (char*)p) {
        p = (sexp) (((char*)p) + r->size);
        continue;
      }
      if (!sexp_markedp(p) && stack_references_pointer_p(ctx, p)) {
#ifdef SEXP_USE_CONSERVATIVE_GC_PRESERVE_TAG
        if (sexp_pointer_tag(p) == SEXP_USE_CONSERVATIVE_GC_PRESERVE_TAG)
#endif
        if (1) {
#if SEXP_USE_DEBUG_GC > 3
          if (p && sexp_pointerp(p)) {
            fprintf(stderr, SEXP_BANNER("MISS: %p [%d]: %s"), p,
                    sexp_pointer_tag(p), sexp_pointer_source(p));
            sexp_print_gc_trace(ctx, p);
            fflush(stderr);
          }
#endif
          sexp_mark(ctx, p);
        }
      }
      p = (sexp) (((char*)p)+sexp_heap_align(sexp_allocated_bytes(ctx, p)));
    }
  }
}

#else
#define sexp_conservative_mark(ctx)
#endif

#if SEXP_USE_WEAK_REFERENCES
int sexp_reset_weak_references(sexp ctx) {
  int i, len, broke, all_reset_p;
  sexp_heap h;
  sexp p, t, end, *v;
  sexp_free_list q, r;
  if (sexp_not(sexp_global(ctx, SEXP_G_WEAK_OBJECTS_PRESENT)))
    return 0;
  broke = 0;
  /* just scan the whole heap */
  for (h = sexp_context_heap(ctx) ; h; h=h->next) {
    p = sexp_heap_first_block(h);
    q = h->free_list;
    end = sexp_heap_end(h);
    while (p < end) {
      /* find the preceding and succeeding free list pointers */
      for (r=q->next; r && ((char*)r<(char*)p); q=r, r=r->next)
        ;
      if ((char*)r == (char*)p) { /* this is a free block, skip it */
        p = (sexp) (((char*)p) + r->size);
        continue;
      }
      if (sexp_valid_object_p(ctx, p) && sexp_markedp(p)) {
        t = sexp_object_type(ctx, p);
        if (sexp_type_weak_base(t) > 0) {
          all_reset_p = 1;
          v = (sexp*) ((char*)p + sexp_type_weak_base(t));
          len = sexp_type_num_weak_slots_of_object(t, p);
          for (i=0; i<len; i++) {
            if (v[i] && sexp_pointerp(v[i]) && ! sexp_markedp(v[i])) {
              v[i] = SEXP_FALSE;
              sexp_brokenp(p) = 1;
            } else {
              all_reset_p = 0;
            }
          }
          if (all_reset_p) {      /* ephemerons */
            broke++;
            len += sexp_type_weak_len_extra(t);
            for ( ; i<len; i++) v[i] = SEXP_FALSE;
          }
        }
      }
      p = (sexp) (((char*)p)+sexp_heap_align(sexp_allocated_bytes(ctx, p)));
    }
  }
  sexp_debug_printf("%p (broke %d weak references)", ctx, broke);
  return broke;
}
#else
#define sexp_reset_weak_references(ctx) 0
#endif

#if SEXP_USE_FINALIZERS
sexp sexp_finalize (sexp ctx) {
  size_t size;
  sexp p, t, end;
  sexp_free_list q, r;
  sexp_proc2 finalizer;
  sexp_sint_t finalize_count = 0;
  sexp_heap h = sexp_context_heap(ctx);
#if SEXP_USE_DL
  sexp_sint_t free_dls = 0, pass = 0;
 loop:
#endif
  /* scan over the whole heap */
  for ( ; h; h=h->next) {
    p = sexp_heap_first_block(h);
    q = h->free_list;
    end = sexp_heap_end(h);
    while (p < end) {
      /* find the preceding and succeeding free list pointers */
      for (r=q->next; r && ((char*)r<(char*)p); q=r, r=r->next)
        ;
      if ((char*)r == (char*)p) { /* this is a free block, skip it */
        p = (sexp) (((char*)p) + r->size);
        continue;
      }
      size = sexp_heap_align(sexp_allocated_bytes(ctx, p));
      if (size == 0) {
        return SEXP_FALSE;
      }
      if (!sexp_markedp(p)) {
        t = sexp_object_type(ctx, p);
        finalizer = sexp_type_finalize(t);
        if (finalizer) {
          finalize_count++;
#if SEXP_USE_DL
          if (sexp_type_tag(t) == SEXP_DL && pass <= 0)
            free_dls = 1;
          else
#endif
            finalizer(ctx, NULL, 1, p);
        }
      }
      p = (sexp) (((char*)p)+size);
    }
  }
#if SEXP_USE_DL
  if (free_dls && pass++ <= 0) goto loop;
#endif
  return sexp_make_fixnum(finalize_count);
}
#endif

sexp sexp_sweep (sexp ctx, size_t *sum_freed_ptr) {
  size_t freed, max_freed=0, sum_freed=0, size;
  sexp_heap h = sexp_context_heap(ctx);
  sexp p, end;
  sexp_free_list q, r, s;
  /* scan over the whole heap */
  for ( ; h; h=h->next) {
    p = sexp_heap_first_block(h);
    q = h->free_list;
    end = sexp_heap_end(h);
    while (p < end) {
      /* find the preceding and succeeding free list pointers */
      for (r=q->next; r && ((char*)r<(char*)p); q=r, r=r->next)
        ;
      if ((char*)r == (char*)p) { /* this is a free block, skip it */
        p = (sexp) (((char*)p) + r->size);
        continue;
      }
      size = sexp_heap_align(sexp_allocated_bytes(ctx, p));
#if SEXP_USE_DEBUG_GC > 1
      if (!sexp_valid_object_p(ctx, p))
        fprintf(stderr, SEXP_BANNER("%p sweep: invalid object at %p"), ctx, p);
      if ((char*)q + q->size > (char*)p)
        fprintf(stderr, SEXP_BANNER("%p sweep: bad size at %p < %p + %lu"),
                ctx, p, q, q->size);
      if (r && ((char*)p)+size > (char*)r)
        fprintf(stderr, SEXP_BANNER("%p sweep: bad size at %p + %lu > %p"),
                ctx, p, size, r);
#endif
      if (!sexp_markedp(p)) {
        /* free p */
        sum_freed += size;
        if (((((char*)q) + q->size) == (char*)p) && (q != h->free_list)) {
          /* merge q with p */
          if (r && r->size && ((((char*)p)+size) == (char*)r)) {
            /* ... and with r */
            q->next = r->next;
            freed = q->size + size + r->size;
            p = (sexp) (((char*)p) + size + r->size);
          } else {
            freed = q->size + size;
            p = (sexp) (((char*)p)+size);
          }
          q->size = freed;
        } else {
          s = (sexp_free_list)p;
          if (r && r->size && ((((char*)p)+size) == (char*)r)) {
            /* merge p with r */
            s->size = size + r->size;
            s->next = r->next;
            q->next = s;
            freed = size + r->size;
          } else {
            s->size = size;
            s->next = r;
            q->next = s;
            freed = size;
          }
          p = (sexp) (((char*)p)+freed);
        }
        if (freed > max_freed)
          max_freed = freed;
      } else {
        sexp_markedp(p) = 0;
        p = (sexp) (((char*)p)+size);
      }
    }
  }
  if (sum_freed_ptr) *sum_freed_ptr = sum_freed;
  return sexp_make_fixnum(max_freed);
}

#if SEXP_USE_GLOBAL_SYMBOLS
void sexp_mark_global_symbols(sexp ctx) {
  int i;
  for (i=0; i<SEXP_SYMBOL_TABLE_SIZE; i++)
    sexp_mark(ctx, sexp_symbol_table[i]);
}
#else
#define sexp_mark_global_symbols(ctx)
#endif

sexp sexp_gc (sexp ctx, size_t *sum_freed) {
  sexp res, finalized SEXP_NO_WARN_UNUSED;
#if SEXP_USE_TIME_GC
  sexp_uint_t gc_usecs;
  struct rusage start, end;
  getrusage(RUSAGE_SELF, &start);
  sexp_debug_printf("%p (heap: %p size: %lu)", ctx, sexp_context_heap(ctx),
                    sexp_heap_total_size(sexp_context_heap(ctx)));
#endif
  sexp_mark_global_symbols(ctx);
  sexp_mark(ctx, ctx);
  sexp_conservative_mark(ctx);
  sexp_reset_weak_references(ctx);
  finalized = sexp_finalize(ctx);
  res = sexp_sweep(ctx, sum_freed);
  ++sexp_context_gc_count(ctx);
#if SEXP_USE_TIME_GC
  getrusage(RUSAGE_SELF, &end);
  gc_usecs = (end.ru_utime.tv_sec - start.ru_utime.tv_sec) * 1000000 +
    end.ru_utime.tv_usec - start.ru_utime.tv_usec;
  sexp_context_gc_usecs(ctx) += gc_usecs;
  sexp_debug_printf("%p (freed: %lu max_freed: %lu finalized: %lu time: %luus)",
                    ctx, (sum_freed ? *sum_freed : 0), sexp_unbox_fixnum(res),
                    sexp_unbox_fixnum(finalized), gc_usecs);
#endif
  return res;
}

sexp_heap sexp_make_heap (void* alloc, size_t size, size_t max_size, size_t chunk_size) {
  sexp_free_list free, next;
  sexp_heap h;
#if SEXP_USE_MMAP_GC
  h =  mmap(NULL, sexp_heap_pad_size(size), PROT_READ|PROT_WRITE,
            MAP_ANON|MAP_PRIVATE, -1, 0);
  if (h == MAP_FAILED) return NULL;
#else
  h =  sexp_malloc(alloc, sexp_heap_pad_size(size));
  if (! h) return NULL;
#endif
  h->size = size;
  h->max_size = max_size;
  h->chunk_size = chunk_size;
  h->data = (char*) sexp_heap_align(sizeof(h->data)+(sexp_uint_t)&(h->data));
  free = h->free_list = (sexp_free_list) h->data;
  h->next = NULL;
  next = (sexp_free_list) (((char*)free)+sexp_heap_align(sexp_free_chunk_size));
  free->size = 0; /* actually sexp_heap_align(sexp_free_chunk_size) */
  free->next = next;
  next->size = size - sexp_heap_align(sexp_free_chunk_size);
  next->next = NULL;
#if SEXP_USE_DEBUG_GC
  fprintf(stderr, SEXP_BANNER("heap: %p-%p data: %p-%p"),
          h, ((char*)h)+sexp_heap_pad_size(size), h->data, h->data + size);
  fprintf(stderr, SEXP_BANNER("first: %p end: %p"),
          sexp_heap_first_block(h), sexp_heap_end(h));
  fprintf(stderr, SEXP_BANNER("free1: %p-%p free2: %p-%p"),
          free, ((char*)free)+free->size, next, ((char*)next)+next->size);
#endif
  return h;
}

int sexp_grow_heap (sexp ctx, size_t size, size_t chunk_size) {
  size_t cur_size, new_size;
  sexp_heap tmp, h = sexp_heap_last(sexp_context_heap(ctx));
#if SEXP_USE_FIXED_CHUNK_SIZE_HEAPS
  for (tmp=sexp_context_heap(ctx); tmp; tmp=tmp->next)
    if (tmp->chunk_size == size) {
      while (tmp->next && tmp->next->chunk_size == size)
        tmp = tmp->next;
      h = tmp;
      chunk_size = size;
      break;
    }
#endif
  cur_size = h->size;
  new_size = (size_t) ceil(SEXP_GROW_HEAP_FACTOR * (double) (sexp_heap_align(((cur_size > size) ? cur_size : size))));
  tmp = sexp_make_heap(sexp_context_alloc(ctx), new_size, h->max_size, chunk_size);
  if (tmp) {
    tmp->next = h->next;
    h->next = tmp;
  }
  return (h->next != NULL);
}

void* sexp_try_alloc (sexp ctx, size_t size) {
  sexp_free_list ls1, ls2, ls3;
  sexp_heap h;
#if SEXP_USE_FIXED_CHUNK_SIZE_HEAPS
  int found_fixed = 0;
#endif
  for (h=sexp_context_heap(ctx); h; h=h->next) {
#if SEXP_USE_FIXED_CHUNK_SIZE_HEAPS
    if (h->chunk_size) {
      if (h->chunk_size != size)
        continue;
      found_fixed = 1;
    } else if (found_fixed) {   /* don't use a non-fixed heap */
      return NULL;
    }
#endif
    for (ls1=h->free_list, ls2=ls1->next; ls2; ls1=ls2, ls2=ls2->next) {
      if (ls2->size >= size) {
#if SEXP_USE_DEBUG_GC > 1
        ls3 = (sexp_free_list) sexp_heap_end(h);
        if (ls2 >= ls3)
          fprintf(stderr, "alloced %lu bytes past end of heap: %p (%lu) >= %p"
                  " next: %p (%lu)\n", size, ls2, ls2->size, ls3, ls2->next,
                  (ls2->next ? ls2->next->size : 0));
#endif
        if (ls2->size >= (size + SEXP_MINIMUM_OBJECT_SIZE)) {
          ls3 = (sexp_free_list) (((char*)ls2)+size); /* the tail after ls2 */
          ls3->size = ls2->size - size;
          ls3->next = ls2->next;
          ls1->next = ls3;
        } else {                  /* take the whole chunk */
          ls1->next = ls2->next;
        }
        memset((void*)ls2, 0, size);
        return ls2;
      }
    }
  }
  return NULL;
}

#if SEXP_USE_FIXED_CHUNK_SIZE_HEAPS
int sexp_find_fixed_chunk_heap_usage(sexp ctx, size_t size, size_t* sum_freed, size_t* total_size) {
  sexp_heap h;
  sexp_free_list ls;
  size_t avail=0, total=0;
  for (h=sexp_context_heap(ctx); h; h=h->next) {
    if (h->chunk_size == size || !h->chunk_size) {
      for (; h && (h->chunk_size == size || !h->chunk_size); h=h->next) {
        total += h->size;
        for (ls=h->free_list; ls; ls=ls->next)
          avail += ls->size;
      }
      *sum_freed = avail;
      *total_size = total;
      return h && h->chunk_size > 0;
    }
  }
  return 0;
}
#endif

void* sexp_alloc (sexp ctx, size_t size) {
  void *res;
  size_t max_freed, sum_freed, total_size=0;
  sexp_heap h = sexp_context_heap(ctx);
#if SEXP_USE_TRACK_ALLOC_SIZES
  size_t size_bucket;
#endif
#if SEXP_USE_TRACK_ALLOC_TIMES
  sexp_uint_t alloc_time;
  struct timeval start, end;
  gettimeofday(&start, NULL);
#endif
  size = sexp_heap_align(size) + SEXP_GC_PAD;
#if SEXP_USE_TRACK_ALLOC_SIZES
  size_bucket = (size - SEXP_GC_PAD) / sexp_heap_align(1) - 1;
  ++sexp_context_alloc_histogram(ctx)[size_bucket >= SEXP_ALLOC_HISTOGRAM_BUCKETS ? SEXP_ALLOC_HISTOGRAM_BUCKETS-1 : size_bucket];
#endif
  res = sexp_try_alloc(ctx, size);
  if (! res) {
    max_freed = sexp_unbox_fixnum(sexp_gc(ctx, &sum_freed));
#if SEXP_USE_FIXED_CHUNK_SIZE_HEAPS
    sexp_find_fixed_chunk_heap_usage(ctx, size, &sum_freed, &total_size);
#else
    total_size = sexp_heap_total_size(sexp_context_heap(ctx));
#endif
    if (((max_freed < size)
         || ((total_size > sum_freed)
             && (total_size - sum_freed) > (total_size*SEXP_GROW_HEAP_RATIO)))
        && ((!h->max_size) || (total_size < h->max_size)))
      sexp_grow_heap(ctx, size, 0);
    res = sexp_try_alloc(ctx, size);
    if (! res) {
      res = sexp_global(ctx, SEXP_G_OOM_ERROR);
      sexp_debug_printf("ran out of memory allocating %lu bytes => %p", size, res);
    }
  }
#if SEXP_USE_TRACK_ALLOC_TIMES
  gettimeofday(&end, NULL);
  alloc_time = 1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec);
  sexp_context_alloc_count(ctx) += 1;
  sexp_context_alloc_usecs(ctx) += alloc_time;
  sexp_context_alloc_usecs_sq(ctx) += alloc_time*alloc_time;
#endif
  return res;
}


void sexp_gc_init (void) {
#if SEXP_USE_GLOBAL_HEAP || SEXP_USE_CONSERVATIVE_GC
  sexp_uint_t size = sexp_heap_align(SEXP_INITIAL_HEAP_SIZE);
#endif
#if SEXP_USE_GLOBAL_HEAP
  sexp_global_heap = sexp_make_heap(size, SEXP_MAXIMUM_HEAP_SIZE, 0);
#endif
#if SEXP_USE_CONSERVATIVE_GC
  /* the +32 is a hack, but this is just for debugging anyway */
  stack_base = ((sexp*)&size) + 32;
#endif
}

#endif  /* ! SEXP_USE_BOEHM && ! SEXP_USE_MALLOC */
