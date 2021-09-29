#ifndef LIBCSV_H__
#define LIBCSV_H__
#include <stdlib.h>
#include <stdio.h>

/*张子辰's modification begin*/
#if defined(_MSC_VER) && defined(CSV_EXPORT)
#  define CSV_API_TARGET __declspec(dllexport)
#else
#  define CSV_API_TARGET
#endif
/*张子辰's modification end*/

#ifdef __cplusplus
extern "C" {
#endif

#define CSV_MAJOR 3
#define CSV_MINOR 0
#define CSV_RELEASE 3

/* Error Codes */
#define CSV_SUCCESS 0
#define CSV_EPARSE 1   /* Parse error in strict mode */
#define CSV_ENOMEM 2   /* Out of memory while increasing buffer size */
#define CSV_ETOOBIG 3  /* Buffer larger than SIZE_MAX needed */
#define CSV_EINVALID 4 /* Invalid code,should never be received from csv_error*/


/* parser options */
#define CSV_STRICT 1    /* enable strict mode */
#define CSV_REPALL_NL 2 /* report all unquoted carriage returns and linefeeds */
#define CSV_STRICT_FINI 4 /* causes csv_fini to return CSV_EPARSE if last
                             field is quoted and doesn't containg ending 
                             quote */
#define CSV_APPEND_NULL 8 /* Ensure that all fields are null-terminated */
#define CSV_EMPTY_IS_NULL 16 /* Pass null pointer to cb1 function when
                                empty, unquoted fields are encountered */


/* Character values */
#define CSV_TAB    0x09
#define CSV_SPACE  0x20
#define CSV_CR     0x0d
#define CSV_LF     0x0a
#define CSV_COMMA  0x2c
#define CSV_QUOTE  0x22

struct csv_parser {
  int pstate;         /* Parser state */
  int quoted;         /* Is the current field a quoted field? */
  size_t spaces;      /* Number of continious spaces after quote or in a non-quoted field */
  unsigned char * entry_buf;   /* Entry buffer */
  size_t entry_pos;   /* Current position in entry_buf (and current size of entry) */
  size_t entry_size;  /* Size of entry buffer */
  int status;         /* Operation status */
  unsigned char options;
  unsigned char quote_char;
  unsigned char delim_char;
  int (*is_space)(unsigned char);
  int (*is_term)(unsigned char);
  size_t blk_size;
  void *(*malloc_func)(size_t);
  void *(*realloc_func)(void *, size_t);
  void (*free_func)(void *);
};

/* Function Prototypes */
/*CSV_API_TARGET is added by 张子辰*/
CSV_API_TARGET int csv_init(struct csv_parser *p, unsigned char options);
CSV_API_TARGET int csv_fini(struct csv_parser *p, void (*cb1)(void *, size_t, void *), void (*cb2)(int, void *), void *data);
CSV_API_TARGET void csv_free(struct csv_parser *p);
CSV_API_TARGET int csv_error(struct csv_parser *p);
CSV_API_TARGET char * csv_strerror(int error);
CSV_API_TARGET size_t csv_parse(struct csv_parser *p, const void *s, size_t len, void (*cb1)(void *, size_t, void *), void (*cb2)(int, void *), void *data);
CSV_API_TARGET size_t csv_write(void *dest, size_t dest_size, const void *src, size_t src_size);
CSV_API_TARGET int csv_fwrite(FILE *fp, const void *src, size_t src_size);
CSV_API_TARGET size_t csv_write2(void *dest, size_t dest_size, const void *src, size_t src_size, unsigned char quote);
CSV_API_TARGET int csv_fwrite2(FILE *fp, const void *src, size_t src_size, unsigned char quote);
CSV_API_TARGET int csv_get_opts(struct csv_parser *p);
CSV_API_TARGET int csv_set_opts(struct csv_parser *p, unsigned char options);
CSV_API_TARGET void csv_set_delim(struct csv_parser *p, unsigned char c);
CSV_API_TARGET void csv_set_quote(struct csv_parser *p, unsigned char c);
CSV_API_TARGET unsigned char csv_get_delim(struct csv_parser *p);
CSV_API_TARGET unsigned char csv_get_quote(struct csv_parser *p);
CSV_API_TARGET void csv_set_space_func(struct csv_parser *p, int (*f)(unsigned char));
CSV_API_TARGET void csv_set_term_func(struct csv_parser *p, int (*f)(unsigned char));
CSV_API_TARGET void csv_set_realloc_func(struct csv_parser *p, void *(*)(void *, size_t));
CSV_API_TARGET void csv_set_free_func(struct csv_parser *p, void (*)(void *));
CSV_API_TARGET void csv_set_blk_size(struct csv_parser *p, size_t);
CSV_API_TARGET size_t csv_get_buffer_size(struct csv_parser *p);

#ifdef __cplusplus
}
#endif

/*张子辰's modification begin*/
#undef CSV_API_TARGET
/*张子辰's modification end*/

#endif
